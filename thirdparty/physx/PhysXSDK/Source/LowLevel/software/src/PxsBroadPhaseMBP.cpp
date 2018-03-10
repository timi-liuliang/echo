/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PxsBroadPhaseMBP.h"
#include "PxsBroadPhaseMBPCommon.h"
#include "GuContainer.h"
#include "GuRevisitedRadixBuffered.h"
#include "PxsAABBManager.h"
#include "CmUtils.h"
#include "CmEventProfiler.h"

//#define CHECK_NB_OVERLAPS
//#define USE_SINGLE_THREADED_REFERENCE_CODE
#define USE_FULLY_INSIDE_FLAG

using namespace physx;
using namespace Gu;

	static PX_FORCE_INLINE void storeDwords(PxU32* dest, PxU32 nb, PxU32 value)
	{
		while(nb--)
			*dest++ = value;
	}

#define MBP_ALLOC(x)		PX_ALLOC(x, PX_DEBUG_EXP("MBP"))
#define MBP_ALLOC_TMP(x)	PX_ALLOC_TEMP(x, PX_DEBUG_EXP("MBP_TMP"))
#define MBP_FREE(x)			if(x)	PX_FREE_AND_RESET(x)
#define DELETESINGLE(x)		if (x) { delete x;		x = NULL; }
#define DELETEARRAY(x)		if (x) { delete []x;	x = NULL; }

#define	INVALID_ID	0xffffffff

	typedef	MBP_Index*	MBP_Mapping;

	PX_FORCE_INLINE PxU32 encodeFloat_(PxU32 ir)
	{
		if(ir & 0x80000000)
			return ~ir;
		else
			return ir | 0x80000000;
/*
		const PxU32 c = ~ir;
		const PxU32 d = ir | 0x80000000;
		return (((PxI32(ir & 0x80000000)>>31) & (c^d)) ^ d);
*/
	}

/*	PX_FORCE_INLINE PxU32 encodeFloat(const float val)
	{
		// We may need to check on -0 and 0
		// But it should make no practical difference.
		PxU32 ir = IR(val);

		if(ir & 0x80000000) //negative?
			ir = ~ir;//reverse sequence of negative numbers
		else
			ir |= 0x80000000; // flip sign

		return ir;
	}*/

	struct IAABB : public IAABB_Data, public Ps::UserAllocated
	{
		PX_FORCE_INLINE bool isInside(const IAABB& box) const
		{
/*			if(box.GetMin(0)>GetMin(0))	return false;
			if(box.GetMin(1)>GetMin(1))	return false;
			if(box.GetMin(2)>GetMin(2))	return false;
			if(box.GetMax(0)<GetMax(0))	return false;
			if(box.GetMax(1)<GetMax(1))	return false;
			if(box.GetMax(2)<GetMax(2))	return false;*/

			if(box.mMinX>mMinX)	return false;
			if(box.mMinY>mMinY)	return false;
			if(box.mMinZ>mMinZ)	return false;
			if(box.mMaxX<mMaxX)	return false;
			if(box.mMaxY<mMaxY)	return false;
			if(box.mMaxZ<mMaxZ)	return false;
			return true;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersect(const IAABB& a)	const
		{
			if(mMaxX < a.mMinX || a.mMaxX < mMinX
			|| mMaxY < a.mMinY || a.mMaxY < mMinY
			|| mMaxZ < a.mMinZ || a.mMaxZ < mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersectNoTouch(const IAABB& a)	const
		{
			if(mMaxX <= a.mMinX || a.mMaxX <= mMinX
			|| mMaxY <= a.mMinY || a.mMaxY <= mMinY
			|| mMaxZ <= a.mMinZ || a.mMaxZ <= mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

/*		PX_FORCE_INLINE	void	initFrom(const PxBounds3& box)
		{
			mMinX = encodeFloat(box.minimum.x);
			mMinY = encodeFloat(box.minimum.y);
			mMinZ = encodeFloat(box.minimum.z);
			mMaxX = encodeFloat(box.maximum.x);
			mMaxY = encodeFloat(box.maximum.y);
			mMaxZ = encodeFloat(box.maximum.z);
		}*/

		PX_FORCE_INLINE	void	initFrom2(const PxBounds3& box)
		{
/*			mMinX = encodeFloat(box.GetMin(0))>>1;
			mMinY = encodeFloat(box.GetMin(1))>>1;
			mMinZ = encodeFloat(box.GetMin(2))>>1;
			mMaxX = encodeFloat(box.GetMax(0))>>1;
			mMaxY = encodeFloat(box.GetMax(1))>>1;
			mMaxZ = encodeFloat(box.GetMax(2))>>1;*/
			const PxU32* PX_RESTRICT binary = (const PxU32*)&box.minimum.x;
			mMinX = encodeFloat_(binary[0])>>1;
			mMinY = encodeFloat_(binary[1])>>1;
			mMinZ = encodeFloat_(binary[2])>>1;
			mMaxX = encodeFloat_(binary[3])>>1;
			mMaxY = encodeFloat_(binary[4])>>1;
			mMaxZ = encodeFloat_(binary[5])>>1;
		}

		PX_FORCE_INLINE	void	decode(PxBounds3& box)	const
		{
			PxU32* PX_RESTRICT binary = (PxU32*)&box.minimum.x;
			binary[0] = PxsBpDecodeFloat(mMinX<<1);
			binary[1] = PxsBpDecodeFloat(mMinY<<1);
			binary[2] = PxsBpDecodeFloat(mMinZ<<1);
			binary[3] = PxsBpDecodeFloat(mMaxX<<1);
			binary[4] = PxsBpDecodeFloat(mMaxY<<1);
			binary[5] = PxsBpDecodeFloat(mMaxZ<<1);
		}

		PX_FORCE_INLINE PxU32	getMin(PxU32 i)	const	{	return (&mMinX)[i];	}
		PX_FORCE_INLINE PxU32	getMax(PxU32 i)	const	{	return (&mMaxX)[i];	}
	};

#ifdef MBP_SIMD_OVERLAP
	struct SIMD_AABB : public SIMD_AABB_Data, public Ps::UserAllocated
	{
		PX_FORCE_INLINE	void	initFrom(const PxBounds3& box)
		{
			const PxU32* PX_RESTRICT binary = (const PxU32*)&box.minimum.x;
			mMinX = encodeFloat_(binary[0]);
			mMinY = encodeFloat_(binary[1]);
			mMinZ = encodeFloat_(binary[2]);
			mMaxX = encodeFloat_(binary[3]);
			mMaxY = encodeFloat_(binary[4]);
			mMaxZ = encodeFloat_(binary[5]);
		}

		PX_FORCE_INLINE	void	initFrom2(const PxBounds3& box)
		{
			const PxU32* PX_RESTRICT binary = (const PxU32*)&box.minimum.x;
/*			mMinX = encodeFloat_(binary[0]);//>>1;
			mMinY = encodeFloat_(binary[1]);//>>1;
			mMinZ = encodeFloat_(binary[2]);//>>1;
			mMaxX = encodeFloat_(binary[3]);//>>1;
			mMaxY = encodeFloat_(binary[4]);//>>1;
			mMaxZ = encodeFloat_(binary[5]);//>>1;
*/
			mMinX = encodeFloat_(binary[0])>>1;
			mMinY = encodeFloat_(binary[1])>>1;
			mMinZ = encodeFloat_(binary[2])>>1;
			mMaxX = encodeFloat_(binary[3])>>1;
			mMaxY = encodeFloat_(binary[4])>>1;
			mMaxZ = encodeFloat_(binary[5])>>1;

/*
			mMinX = PxI32(box.mMin.x);
			mMinY = PxI32(box.mMin.y);
			mMinZ = PxI32(box.mMin.z);
			mMaxX = PxI32(box.mMax.x);
			mMaxY = PxI32(box.mMax.y);
			mMaxZ = PxI32(box.mMax.z);*/
		}

		PX_FORCE_INLINE	void	decode(PxBounds3& box)	const
		{
			PxU32* PX_RESTRICT binary = (PxU32*)&box.minimum.x;
			binary[0] = PxsBpDecodeFloat(mMinX<<1);
			binary[1] = PxsBpDecodeFloat(mMinY<<1);
			binary[2] = PxsBpDecodeFloat(mMinZ<<1);
			binary[3] = PxsBpDecodeFloat(mMaxX<<1);
			binary[4] = PxsBpDecodeFloat(mMaxY<<1);
			binary[5] = PxsBpDecodeFloat(mMaxZ<<1);
		}

		PX_FORCE_INLINE bool isInside(const SIMD_AABB& box) const
		{
/*			if(box.GetMin(0)>GetMin(0))	return false;
			if(box.GetMin(1)>GetMin(1))	return false;
			if(box.GetMin(2)>GetMin(2))	return false;
			if(box.GetMax(0)<GetMax(0))	return false;
			if(box.GetMax(1)<GetMax(1))	return false;
			if(box.GetMax(2)<GetMax(2))	return false;*/

			if(box.mMinX>mMinX)	return false;
			if(box.mMinY>mMinY)	return false;
			if(box.mMinZ>mMinZ)	return false;
			if(box.mMaxX<mMaxX)	return false;
			if(box.mMaxY<mMaxY)	return false;
			if(box.mMaxZ<mMaxZ)	return false;
			return true;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersect(const SIMD_AABB& a)	const
		{
			if(mMaxX < a.mMinX || a.mMaxX < mMinX
			|| mMaxY < a.mMinY || a.mMaxY < mMinY
			|| mMaxZ < a.mMinZ || a.mMaxZ < mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersectNoTouch(const SIMD_AABB& a)	const
		{
			if(mMaxX <= a.mMinX || a.mMaxX <= mMinX
			|| mMaxY <= a.mMinY || a.mMaxY <= mMinY
			|| mMaxZ <= a.mMinZ || a.mMaxZ <= mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}
	};
#endif

struct BoxPrunerHandle : public Ps::UserAllocated
{
	PxU16	mHandle;			// Handle from box pruner
	PxU16	mInternalBPHandle;	// Index of box pruner data within mBoxPruners
};

enum MBPFlags
{
	MBP_FLIP_FLOP		= (1<<1),
#ifdef USE_FULLY_INSIDE_FLAG
//	MBP_FULLY_INSIDE	= (1<<2),
#endif
	MBP_REMOVED			= (1<<2)	// ### added for TA24714, not needed otherwise
};

// We have one of those for each of the "200K" objects so we should optimize this size as much as possible
struct MBP_Object : public Ps::UserAllocated
{
	PxcBpHandle	mUserID;		// ### added for PhysX integration
	PxU16		mNbHandles;		// Number of BoxPruners the object is part of
	PxU16		mFlags;			// MBPFlags ### only 1 bit used in the end

	PX_FORCE_INLINE	bool	getFlipFlop()	const	{ return (mFlags & MBP_FLIP_FLOP)==0;	}

	union
	{
		BoxPrunerHandle	mHandle;
		PxU32			mHandlesIndex;
	};
};

// This one is used in each BoxPruner
struct MBPEntry : public MBPEntry_Data, public Ps::UserAllocated
{
	PX_FORCE_INLINE	MBPEntry()
	{
		mMBPHandle = INVALID_ID;
	}
};

///////////////////////////////////////////////////////////////////////////////

//#define BIT_ARRAY_STACK	512

	/*static PX_FORCE_INLINE PxU32 bitsToBytes(PxU32 nbBits)
	{
		return (nbBits>>3) + ((nbBits&7) ? 1 : 0);
	}*/

	static PX_FORCE_INLINE PxU32 bitsToDwords(PxU32 nbBits)
	{
		return (nbBits>>5) + ((nbBits&31) ? 1 : 0);
	}

	// Use that one instead of an array of bools. Takes less ram, nearly as fast [no bounds checkings and so on].
	class BitArray
	{
		public:
										BitArray();
										BitArray(PxU32 nbBits);
										~BitArray();

						bool			init(PxU32 nbBits);
						void			empty();
						void			resize(PxU32 nbBits);

		PX_FORCE_INLINE	void			setBitChecked(PxU32 bitNumber)
										{
											const PxU32 index = bitNumber>>5;
											if(index>=mSize)
												resize(bitNumber);
											mBits[index] |= 1<<(bitNumber&31);
										}

		PX_FORCE_INLINE	void			clearBitChecked(PxU32 bitNumber)
										{
											const PxU32 index = bitNumber>>5;
											if(index>=mSize)
												resize(bitNumber);
											mBits[index] &= ~(1<<(bitNumber&31));
										}

		// Data management
		PX_FORCE_INLINE	void			setBit(PxU32 bitNumber)					{ mBits[bitNumber>>5] |= 1<<(bitNumber&31);				}
		PX_FORCE_INLINE	void			clearBit(PxU32 bitNumber)				{ mBits[bitNumber>>5] &= ~(1<<(bitNumber&31));			}
		PX_FORCE_INLINE	void			toggleBit(PxU32 bitNumber)				{ mBits[bitNumber>>5] ^= 1<<(bitNumber&31);				}

		PX_FORCE_INLINE	void			clearAll()								{ PxMemZero(mBits, mSize*4);							}
		PX_FORCE_INLINE	void			setAll()								{ PxMemSet(mBits, 0xff, mSize*4);						}

		// Data access
		PX_FORCE_INLINE	Ps::IntBool		isSet(PxU32 bitNumber)			const	{ return Ps::IntBool(mBits[bitNumber>>5] & (1<<(bitNumber&31)));		}
		PX_FORCE_INLINE	Ps::IntBool		isSetChecked(PxU32 bitNumber)	const
										{
											const PxU32 index = bitNumber>>5;
											if(index>=mSize)
												return 0;
											return Ps::IntBool(mBits[index] & (1<<(bitNumber&31)));
										}

		PX_FORCE_INLINE	const PxU32*	getBits()						const	{ return mBits;											}
		PX_FORCE_INLINE	PxU32			getSize()						const	{ return mSize;											}

		protected:
						PxU32*			mBits;		//!< Array of bits
						PxU32			mSize;		//!< Size of the array in dwords
#ifdef BIT_ARRAY_STACK
						PxU32			mStack[BIT_ARRAY_STACK];
#endif
	};

///////////////////////////////////////////////////////////////////////////////

BitArray::BitArray() : mBits(NULL), mSize(0)
{
}

BitArray::BitArray(PxU32 nbBits) : mBits(NULL), mSize(0)
{
	init(nbBits);
}

BitArray::~BitArray()
{
	empty();
}

void BitArray::empty()
{
#ifdef BIT_ARRAY_STACK
	if(mBits!=mStack)
#endif
		MBP_FREE(mBits);
	mBits = NULL;
	mSize = 0;
}

bool BitArray::init(PxU32 nbBits)
{
	mSize = bitsToDwords(nbBits);
	// Get ram for n bits
#ifdef BIT_ARRAY_STACK
	if(mBits!=mStack)
#endif
		MBP_FREE(mBits);
#ifdef BIT_ARRAY_STACK
	if(mSize>BIT_ARRAY_STACK)
#endif
		mBits = (PxU32*)MBP_ALLOC(sizeof(PxU32)*mSize);
#ifdef BIT_ARRAY_STACK
	else
		mBits = mStack;
#endif

	// Set all bits to 0
	clearAll();
	return true;
}

void BitArray::resize(PxU32 nbBits)
{
	const PxU32 newSize = bitsToDwords(nbBits+128);
	PxU32* newBits = NULL;
#ifdef BIT_ARRAY_STACK
	if(newSize>BIT_ARRAY_STACK)
#endif
	{
		// Old buffer was stack or allocated, new buffer is allocated
		newBits = (PxU32*)MBP_ALLOC(sizeof(PxU32)*newSize);
		if(mSize)
			PxMemCopy(newBits, mBits, sizeof(PxU32)*mSize);
	}
#ifdef BIT_ARRAY_STACK
	else
	{
		newBits = mStack;
		if(mSize>BIT_ARRAY_STACK)
		{
			// Old buffer was allocated, new buffer is stack => copy to stack, shrink
			CopyMemory(newBits, mBits, sizeof(PxU32)*BIT_ARRAY_STACK);
		}
		else
		{
			// Old buffer was stack, new buffer is stack => keep working on the same stack buffer, nothing to do
		}
	}
#endif
	const PxU32 remain = newSize - mSize;
	if(remain)
		PxMemZero(newBits + mSize, remain*sizeof(PxU32));

#ifdef BIT_ARRAY_STACK
	if(mBits!=mStack)
#endif
		MBP_FREE(mBits);
	mBits = newBits;
	mSize = newSize;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef MBP_SIMD_OVERLAP
	typedef	SIMD_AABB	MBP_AABB;
#else
	typedef	IAABB		MBP_AABB;
#endif

	struct MBP_Pair : public Ps::UserAllocated
	{
		PxU32		id0;
		PxU32		id1;
		// TODO: optimize memory here
		bool		isNew;
		bool		isUpdated;
	};

	struct MBPEntry;
	struct BoxPrunerHandle;
	struct MBP_Object;

	class MBP_PairManager : public Ps::UserAllocated
	{
		public:
											MBP_PairManager();
											~MBP_PairManager();

						void				purge();
						void				shrinkMemory();

						MBP_Pair*			addPair				(PxU32 id0, PxU32 id1, const PxcBpHandle* PX_RESTRICT groups = NULL, const MBP_Object* objects = NULL);
						bool				removePair			(PxU32 id0, PxU32 id1);
//						bool				removePairs			(const BitArray& array);
						bool				removeMarkedPairs	(const MBP_Object* objects, PxsBroadPhaseMBP* mbp, const BitArray& updated, const BitArray& removed);
						const MBP_Pair*		findPair			(PxU32 id0, PxU32 id1)	const;
		PX_FORCE_INLINE	PxU32				getPairIndex		(const MBP_Pair* pair)		const
											{
												return ((PxU32)((size_t(pair) - size_t(mActivePairs)))/sizeof(MBP_Pair));
											}

						PxU32				mHashSize;
						PxU32				mMask;
						PxU32				mNbActivePairs;
						PxU32*				mHashTable;
						PxU32*				mNext;
						MBP_Pair*			mActivePairs;
						PxU32				mReservedMemory;

		PX_FORCE_INLINE	MBP_Pair*			findPair(PxU32 id0, PxU32 id1, PxU32 hashValue) const;
						void				removePair(PxU32 id0, PxU32 id1, PxU32 hashValue, PxU32 pairIndex);
						void				reallocPairs();
						void				reserveMemory(PxU32 memSize);
	};

	///////////////////////////////////////////////////////////////////////////

	#define STACK_BUFFER_SIZE	256
	struct MBPOS_TmpBuffers
	{
					MBPOS_TmpBuffers();
					~MBPOS_TmpBuffers();

		void		allocateSleeping(PxU32 nbSleeping, PxU32 nbSentinels);
		void		allocateUpdated(PxU32 nbUpdated, PxU32 nbSentinels);

		// PT: wtf, why doesn't the 128 version compile?
//		MBP_AABB	PX_ALIGN(128, mSleepingDynamicBoxes_Stack[STACK_BUFFER_SIZE]);
//		MBP_AABB	PX_ALIGN(128, mUpdatedDynamicBoxes_Stack[STACK_BUFFER_SIZE]);
		MBP_AABB	PX_ALIGN(16, mSleepingDynamicBoxes_Stack[STACK_BUFFER_SIZE]);
		MBP_AABB	PX_ALIGN(16, mUpdatedDynamicBoxes_Stack[STACK_BUFFER_SIZE]);
		MBP_Index	mInToOut_Dynamic_Sleeping_Stack[STACK_BUFFER_SIZE];

		PxU32		mNbSleeping;
		PxU32		mNbUpdated;
		MBP_Index*	mInToOut_Dynamic_Sleeping;
		MBP_AABB*	mSleepingDynamicBoxes;
		MBP_AABB*	mUpdatedDynamicBoxes;
	};

	struct BIP_Input
	{
		BIP_Input() :
			mObjects		(NULL),
			mNbUpdatedBoxes	(0),
			mNbStaticBoxes	(0),
			mDynamicBoxes	(NULL),
			mStaticBoxes	(NULL),
			mInToOut_Static	(NULL),
			mInToOut_Dynamic(NULL),
			mNeeded			(false)
		{
		}

		const MBPEntry*		mObjects;
		PxU32				mNbUpdatedBoxes;
		PxU32				mNbStaticBoxes;
		const MBP_AABB*		mDynamicBoxes;
		const MBP_AABB*		mStaticBoxes;
		const MBP_Index*	mInToOut_Static;
		const MBP_Index*	mInToOut_Dynamic;
		bool				mNeeded;
	};

	struct BoxPruning_Input
	{
		BoxPruning_Input() :
			mObjects					(NULL),
			mUpdatedDynamicBoxes		(NULL),
			mSleepingDynamicBoxes		(NULL),
			mInToOut_Dynamic			(NULL),
			mInToOut_Dynamic_Sleeping	(NULL),
			mNbUpdated					(0),
			mNbNonUpdated				(0),
			mNeeded						(false)
		{
		}

		const MBPEntry*		mObjects;
		const MBP_AABB*		mUpdatedDynamicBoxes;
		const MBP_AABB*		mSleepingDynamicBoxes;
		const MBP_Index*	mInToOut_Dynamic;
		const MBP_Index*	mInToOut_Dynamic_Sleeping;
		PxU32				mNbUpdated;
		PxU32				mNbNonUpdated;
		bool				mNeeded;

		BIP_Input			mBIPInput;
	};

	class BoxPruner : public Ps::UserAllocated
	{
		PX_NOCOPY(BoxPruner)

		public:
							BoxPruner();
							~BoxPruner();

		void				updateObject(const MBP_AABB& bounds, MBP_Index handle);
		MBP_Index			addObject(const MBP_AABB& bounds, PxU32 mbpHandle, bool isStatic);
		void				removeObject(MBP_Index handle);
		PxU32				retrieveBounds(MBP_AABB& bounds, MBP_Index handle)	const;
		void				setBounds(MBP_Index handle, const MBP_AABB& bounds);
#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
		void				prepareOverlaps();
		void				findOverlaps(MBP_PairManager& pairManager, MBPOS_TmpBuffers& buffers);
#endif
		void				prepareOverlapsMT();
		void				findOverlapsMT(MBP_PairManager& pairManager, const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* PX_RESTRICT mbpObjects);

//		private:
		BoxPruning_Input	PX_ALIGN(16, mInput);
		PxU32				mNbObjects;
		PxU32				mMaxNbObjects;
		PxU32				mFirstFree;
		MBPEntry*			mObjects;			// All objects, indexed by user handle
		PxU32				mMaxNbStaticBoxes;
		PxU32				mNbStaticBoxes;
		PxU32				mMaxNbDynamicBoxes;
		PxU32				mNbDynamicBoxes;
		MBP_AABB*			mStaticBoxes;
		MBP_AABB*			mDynamicBoxes;
		MBP_Mapping			mInToOut_Static;	// Maps static boxes to mObjects
		MBP_Mapping			mInToOut_Dynamic;	// Maps dynamic boxes to mObjects
		PxU32*				mPosList;
		PxU32				mNbUpdatedBoxes;
		PxU32				mPrevNbUpdatedBoxes;
		BitArray			mStaticBits;
		RadixSortBuffered	mRS;
		bool				mNeedsSorting;
		bool				mNeedsSortingSleeping;
				
		MBPOS_TmpBuffers	mTmpBuffers;

		void				optimizeMemory();
		void				resizeObjects();
		void				staticSort();
		void				preparePruning(MBPOS_TmpBuffers& buffers);
		void				prepareBIPPruning(const MBPOS_TmpBuffers& buffers);
	};

	///////////////////////////////////////////////////////////////////////////

	#define MAX_NB_MBP	256
//	#define MAX_NB_MBP	16

	class MBP : public Ps::UserAllocated
	{
		public:
											MBP(PxsAABBManager* manager);
		virtual								~MBP();

						bool				preallocate(PxU32 nbBoxPruners, PxU32 nbObjects);
						void				reset();
						void				freeBuffers();

						PxU32				addBoxPruner(const PxBroadPhaseRegion& region, bool populateRegion);
						bool				removeBoxPruner(PxU32 handle);
						const BoxPruner*	getBoxPruner(PxU32 i)					const;
		PX_FORCE_INLINE	PxU32				getNbBoxPruners()						const	{ return mNbBoxPruners;	}
						PxU32				addObject(const MBP_AABB& box, PxcBpHandle userID, bool isStatic=false);
						bool				removeObject(PxU32 handle);
						bool				updateObject(PxU32 handle, const MBP_AABB& box);
						bool				updateObject(PxU32 handle, BoxPruner* removedPruner);
#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
						void				prepareOverlaps();
						void				findOverlaps();
#endif
						void				prepareOverlapsMT();
						void				findOverlapsMT(const PxcBpHandle* PX_RESTRICT groups);
						PxU32				finalize(PxsBroadPhaseMBP* mbp);
						void				shiftOrigin(const PxVec3& shift);
//		private:
						PxU32				mNbPairs;
						PxU32				mNbBoxPruners;
						PxU32				mFirstFreeIndex;	// First free recycled index for mMBP_Objects
						PxU32				mFirstFreeIndexBP;	// First free recycled index for mBoxPruners
						Container			mBoxPruners;
						Container			mMBP_Objects;
						MBP_PairManager		mPairManager;

						BitArray			mUpdatedObjects;
						BitArray			mRemoved;
						Container			mHandles[MAX_NB_MBP+1];
						PxU32				mFirstFree[MAX_NB_MBP+1];
		PX_FORCE_INLINE	BoxPrunerHandle*	getHandles(MBP_Object& currentObject, PxU32 nbHandles)	const;
						void				purgeHandles(MBP_Object* PX_RESTRICT object, PxU32 nbHandles);
						void				storeHandles(MBP_Object* PX_RESTRICT object, PxU32 nbHandles, const BoxPrunerHandle* PX_RESTRICT handles);

						Container			mOutOfBoundsObjects;
						void				addToOutOfBoundsArray(PxU32 id);

#ifdef USE_FULLY_INSIDE_FLAG
						BitArray			mFullyInsideBitmap;
#endif
						void				populateNewRegion(const MBP_AABB& box);
						PxsAABBManager*		mManager;
	};

#ifdef MBP_SIMD_OVERLAP
	#define MBP_OVERLAP_TEST(x)	SIMD_OVERLAP_TEST(x)
#else
	#define MBP_OVERLAP_TEST(x)	if(intersect2D(box0, x))
#endif

#define DEFAULT_NB_ENTRIES	128
#define INVALID_USER_ID	0xffffffff

#ifdef MBP_USE_SENTINELS
#ifdef MBP_SIMD_OVERLAP
static PX_FORCE_INLINE void initSentinel(SIMD_AABB& box)
{
//	box.mMinX = encodeFloat(FLT_MAX)>>1;
	box.mMinX = 0xffffffff;
}
#ifdef PX_DEBUG
static PX_FORCE_INLINE bool isSentinel(const SIMD_AABB& box)
{
	return box.mMinX == 0xffffffff;
}
#endif
#else
static PX_FORCE_INLINE void initSentinel(MBP_AABB& box)
{
//	box.mMinX = encodeFloat(FLT_MAX)>>1;
	box.mMinX = 0xffffffff;
}
#ifdef PX_DEBUG
static PX_FORCE_INLINE bool isSentinel(const MBP_AABB& box)
{
	return box.mMinX == 0xffffffff;
}
#endif
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

static PX_FORCE_INLINE void sort(PxU32& id0, PxU32& id1)							{ if(id0>id1)	Ps::swap(id0, id1);		}
static PX_FORCE_INLINE bool differentPair(const MBP_Pair& p, PxU32 id0, PxU32 id1)	{ return (id0!=p.id0) || (id1!=p.id1);	}

///////////////////////////////////////////////////////////////////////////////

MBP_PairManager::MBP_PairManager() :
	mHashSize		(0),
	mMask			(0),
	mNbActivePairs	(0),
	mHashTable		(NULL),
	mNext			(NULL),
	mActivePairs	(NULL),
	mReservedMemory (0)
{
}

///////////////////////////////////////////////////////////////////////////////

MBP_PairManager::~MBP_PairManager()
{
	purge();
}

///////////////////////////////////////////////////////////////////////////////

void MBP_PairManager::purge()
{
	MBP_FREE(mNext);
	MBP_FREE(mActivePairs);
	MBP_FREE(mHashTable);
	mHashSize		= 0;
	mMask			= 0;
	mNbActivePairs	= 0;
}

///////////////////////////////////////////////////////////////////////////////

	// Thomas Wang's hash
	static PX_FORCE_INLINE int hash32Bits_1(int key)
	{
		key += ~(key << 15);
		key ^=  (key >> 10);
		key +=  (key << 3);
		key ^=  (key >> 6);
		key += ~(key << 11);
		key ^=  (key >> 16);
		return key;
	}

	static PX_FORCE_INLINE PxU32 hash(PxU32 id0, PxU32 id1)
	{
		return (PxU32)hash32Bits_1( int((id0&0xffff)|(id1<<16)) );
	}

const MBP_Pair* MBP_PairManager::findPair(PxU32 id0, PxU32 id1) const
{
	if(!mHashTable)
		return NULL;	// Nothing has been allocated yet

	// Order the ids
	sort(id0, id1);

	// Compute hash value for this pair
	const PxU32 hashValue = hash(id0, id1) & mMask;

	const MBP_Pair* PX_RESTRICT activePairs = mActivePairs;
	const PxU32* PX_RESTRICT next = mNext;

	// Look for it in the table
	PxU32 offset = mHashTable[hashValue];
	while(offset!=INVALID_ID && differentPair(activePairs[offset], id0, id1))
	{
		PX_ASSERT(activePairs[offset].id0!=INVALID_USER_ID);
		offset = next[offset];		// Better to have a separate array for this
	}
	if(offset==INVALID_ID)
		return NULL;
	PX_ASSERT(offset<mNbActivePairs);
	// Match mActivePairs[offset] => the pair is persistent
	return &activePairs[offset];
}

///////////////////////////////////////////////////////////////////////////////

// Internal version saving hash computation
PX_FORCE_INLINE MBP_Pair* MBP_PairManager::findPair(PxU32 id0, PxU32 id1, PxU32 hashValue) const
{
	if(!mHashTable)
		return NULL;	// Nothing has been allocated yet

	MBP_Pair* PX_RESTRICT activePairs = mActivePairs;
	const PxU32* PX_RESTRICT next = mNext;

	// Look for it in the table
	PxU32 offset = mHashTable[hashValue];
	while(offset!=INVALID_ID && differentPair(activePairs[offset], id0, id1))
	{
		PX_ASSERT(activePairs[offset].id0!=INVALID_USER_ID);
		offset = next[offset];		// Better to have a separate array for this
	}
	if(offset==INVALID_ID)
		return NULL;
	PX_ASSERT(offset<mNbActivePairs);
	// Match mActivePairs[offset] => the pair is persistent
	return &activePairs[offset];
}

///////////////////////////////////////////////////////////////////////////////

MBP_Pair* MBP_PairManager::addPair(PxU32 id0, PxU32 id1, const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* objects)
{
	PX_ASSERT(id0!=INVALID_ID);
	PX_ASSERT(id1!=INVALID_ID);

	if(groups)
	{
		const PxU32 index0 = DecodeHandle_Index(id0);
		const PxU32 index1 = DecodeHandle_Index(id1);

		const PxcBpHandle object0 = objects[index0].mUserID;
		const PxcBpHandle object1 = objects[index1].mUserID;

		if(groups[object0] == groups[object1])
			return NULL;
	}

	// Order the ids
	sort(id0, id1);

	PxU32 hashValue = hash(id0, id1) & mMask;

	{
		MBP_Pair* PX_RESTRICT p = findPair(id0, id1, hashValue);
		if(p)
		{
			p->isUpdated = true;
			return p;	// Persistent pair
		}
	}

	// This is a new pair
	if(mNbActivePairs >= mHashSize)
	{
		// Get more entries
		mHashSize = Ps::nextPowerOfTwo(mNbActivePairs+1);
		mMask = mHashSize-1;

		reallocPairs();

		// Recompute hash value with new hash size
		hashValue = hash(id0, id1) & mMask;
	}

	MBP_Pair* PX_RESTRICT p = &mActivePairs[mNbActivePairs];
	p->id0		= id0;	// ### CMOVs would be nice here
	p->id1		= id1;
	p->isNew	= true;
	p->isUpdated= false;
	mNext[mNbActivePairs] = mHashTable[hashValue];
	mHashTable[hashValue] = mNbActivePairs++;
	return p;
}

///////////////////////////////////////////////////////////////////////////////

void MBP_PairManager::removePair(PxU32 /*id0*/, PxU32 /*id1*/, PxU32 hashValue, PxU32 pairIndex)
{
	// Walk the hash table to fix mNext
	{
		PxU32 offset = mHashTable[hashValue];
		PX_ASSERT(offset!=INVALID_ID);

		PxU32 previous=INVALID_ID;
		while(offset!=pairIndex)
		{
			previous = offset;
			offset = mNext[offset];
		}

		// Let us go/jump us
		if(previous!=INVALID_ID)
		{
			PX_ASSERT(mNext[previous]==pairIndex);
			mNext[previous] = mNext[pairIndex];
		}
		// else we were the first
		else mHashTable[hashValue] = mNext[pairIndex];
		// we're now free to reuse mNext[pairIndex] without breaking the list
	}
#ifdef PX_DEBUG
	mNext[pairIndex]=INVALID_ID;
#endif
	// Invalidate entry

	// Fill holes
	if(1)
	{
		// 1) Remove last pair
		const PxU32 lastPairIndex = mNbActivePairs-1;
		if(lastPairIndex==pairIndex)
		{
			mNbActivePairs--;
		}
		else
		{
			const MBP_Pair* last = &mActivePairs[lastPairIndex];
			const PxU32 lastHashValue = hash(last->id0, last->id1) & mMask;

			// Walk the hash table to fix mNext
			PxU32 offset = mHashTable[lastHashValue];
			PX_ASSERT(offset!=INVALID_ID);

			PxU32 previous=INVALID_ID;
			while(offset!=lastPairIndex)
			{
				previous = offset;
				offset = mNext[offset];
			}

			// Let us go/jump us
			if(previous!=INVALID_ID)
			{
				PX_ASSERT(mNext[previous]==lastPairIndex);
				mNext[previous] = mNext[lastPairIndex];
			}
			// else we were the first
			else mHashTable[lastHashValue] = mNext[lastPairIndex];
			// we're now free to reuse mNext[lastPairIndex] without breaking the list

#ifdef PX_DEBUG
			mNext[lastPairIndex]=INVALID_ID;
#endif

			// Don't invalidate entry since we're going to shrink the array

			// 2) Re-insert in free slot
			mActivePairs[pairIndex] = mActivePairs[lastPairIndex];
#ifdef PX_DEBUG
			PX_ASSERT(mNext[pairIndex]==INVALID_ID);
#endif
			mNext[pairIndex] = mHashTable[lastHashValue];
			mHashTable[lastHashValue] = pairIndex;

			mNbActivePairs--;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

bool MBP_PairManager::removePair(PxU32 id0, PxU32 id1)
{
	// Order the ids
	sort(id0, id1);

	const PxU32 hashValue = hash(id0, id1) & mMask;
	const MBP_Pair* p = findPair(id0, id1, hashValue);
	if(!p)
		return false;
	PX_ASSERT(p->id0==id0);
	PX_ASSERT(p->id1==id1);

	removePair(id0, id1, hashValue, getPairIndex(p));

	shrinkMemory();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void MBP_PairManager::shrinkMemory()
{
	// Check correct memory against actually used memory
	const PxU32 correctHashSize = Ps::nextPowerOfTwo(mNbActivePairs);
	if(mHashSize==correctHashSize)
		return;

	if(mReservedMemory && correctHashSize < mReservedMemory)
		return;

	// Reduce memory used
	mHashSize = correctHashSize;
	mMask = mHashSize-1;

	reallocPairs();
}

///////////////////////////////////////////////////////////////////////////////

void MBP_PairManager::reallocPairs()
{
	MBP_FREE(mHashTable);
	mHashTable = (PxU32*)MBP_ALLOC(mHashSize*sizeof(PxU32));
	storeDwords(mHashTable, mHashSize, INVALID_ID);

	// Get some bytes for new entries
	MBP_Pair* newPairs	= (MBP_Pair*)MBP_ALLOC(mHashSize * sizeof(MBP_Pair));	PX_ASSERT(newPairs);
	PxU32* newNext		= (PxU32*)MBP_ALLOC(mHashSize * sizeof(PxU32));			PX_ASSERT(newNext);

	// Copy old data if needed
	if(mNbActivePairs)
		PxMemCopy(newPairs, mActivePairs, mNbActivePairs*sizeof(MBP_Pair));
	// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
	// yeah, since hash(id0, id1) is a constant
	// However it might not be needed to recompute them => only less efficient but still ok
	for(PxU32 i=0;i<mNbActivePairs;i++)
	{
		const PxU32 hashValue = hash(mActivePairs[i].id0, mActivePairs[i].id1) & mMask;	// New hash value with new mask
		newNext[i] = mHashTable[hashValue];
		mHashTable[hashValue] = i;
	}

	// Delete old data
	MBP_FREE(mNext);
	MBP_FREE(mActivePairs);

	// Assign new pointer
	mActivePairs = newPairs;
	mNext = newNext;
}

///////////////////////////////////////////////////////////////////////////////

void MBP_PairManager::reserveMemory(PxU32 memSize)
{
	if(!memSize)
		return;

	if(!Ps::isPowerOfTwo(memSize))
	{
		memSize = Ps::nextPowerOfTwo(memSize);
	}

	mHashSize = memSize;
	mMask = mHashSize-1;

	mReservedMemory = memSize;

	reallocPairs();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef MBP_SIMD_OVERLAP
//	#include "SIMD_Overlap.h"

//	#define SIMD_VERSION1
	#ifdef SIMD_VERSION1
		#define SIMD_OVERLAP_INIT													\
		const __m128i signBits = _mm_set1_epi32(SIGN_BITMASK);						\
		const __m128i signFlip = _mm_set_epi32(SIGN_BITMASK, SIGN_BITMASK, 0, 0);

		#define SIMD_OVERLAP_PRELOAD_BOX0											\
		__m128i b = _mm_loadu_si128((__m128i*)&box0.mMinY);							\
		b = _mm_shuffle_epi32(b, 78);

		#define SIMD_OVERLAP_TEST(x)												\
		__m128i a = _mm_loadu_si128((__m128i*)&x.mMinY);							\
		__m128i c = _mm_sub_epi32(b, a);											\
		__m128i d = _mm_and_si128(c, signBits);										\
		d = _mm_xor_si128(d, signFlip);												\
		int mask = _mm_movemask_epi8(d);											\
		if(!mask)
	#else
		#define SIMD_OVERLAP_INIT													\
		const __m128i cmpFlip = _mm_set_epi32(0xffffffff, 0xffffffff, 0, 0);

		#define SIMD_OVERLAP_PRELOAD_BOX0											\
		__m128i b = _mm_loadu_si128((__m128i*)&box0.mMinY);							\
		b = _mm_shuffle_epi32(b, 78);

		#define SIMD_OVERLAP_TEST(x)												\
		__m128i a = _mm_loadu_si128((__m128i*)&x.mMinY);							\
		__m128i d = _mm_cmpgt_epi32(a, b);											\
		d = _mm_xor_si128(d, cmpFlip);												\
		int mask = _mm_movemask_epi8(d);											\
		if(!mask)
	#endif

#else
	#define SIMD_OVERLAP_INIT
	#define SIMD_OVERLAP_PRELOAD_BOX0
#endif




#ifdef MBP_USE_NO_CMP_OVERLAP
/*static PX_FORCE_INLINE void initBox(IAABB& box, const PxBounds3& src)
{
	box.initFrom2(src);
}*/
#else
static PX_FORCE_INLINE void initBox(IAABB& box, const PxBounds3& src)
{
	box.initFrom(src);
}
#endif

BoxPruner::BoxPruner() :
	mNbObjects				(0),
	mMaxNbObjects			(0),
	mFirstFree				(INVALID_ID),
	mObjects				(NULL),
	mMaxNbStaticBoxes		(0),
	mNbStaticBoxes			(0),
	mMaxNbDynamicBoxes		(0),
	mNbDynamicBoxes			(0),
	mStaticBoxes			(NULL),
	mDynamicBoxes			(NULL),
	mInToOut_Static			(NULL),
	mInToOut_Dynamic		(NULL),
	mPosList				(NULL),
	mNbUpdatedBoxes			(0),
	mPrevNbUpdatedBoxes		(0),
	mNeedsSorting			(false),
	mNeedsSortingSleeping	(true)
{
}

BoxPruner::~BoxPruner()
{
	DELETEARRAY(mObjects);
	MBP_FREE(mPosList);
	MBP_FREE(mInToOut_Dynamic);
	MBP_FREE(mInToOut_Static);
	DELETEARRAY(mDynamicBoxes);
	DELETEARRAY(mStaticBoxes);
}

// Pre-sort static boxes
#define STACK_BUFFER_SIZE_STATIC_SORT	8192
	#define DEFAULT_NUM_DYNAMIC_BOXES 1024

void BoxPruner::staticSort()
{
	// For now this version is only compatible with:
	// MBP_USE_WORDS
	// MBP_USE_SENTINELS

	mNeedsSorting = false;

	const PxU32 nbStaticBoxes = mNbStaticBoxes;
	if(!nbStaticBoxes)
	{
		mStaticBits.empty();
		return;
	}

//	PxU32 Time;
//	StartProfile(Time);

	// Roadmap:
	// - gather updated/modified static boxes
	// - sort those, and those only
	// - merge sorted set with previously existing (and previously sorted set)

	// Separate things-to-sort and things-already-sorted
	const PxU32 totalSize = sizeof(PxU32)*nbStaticBoxes*4;
	PxU8 stackBuffer[STACK_BUFFER_SIZE_STATIC_SORT];
	PxU8* tempMemory = totalSize<=STACK_BUFFER_SIZE_STATIC_SORT ? stackBuffer : (PxU8*)MBP_ALLOC_TMP(totalSize);
	PxU32* minPosList_ToSort = (PxU32*)tempMemory;
	PxU32* minPosList_Sorted = (PxU32*)(tempMemory + sizeof(PxU32)*nbStaticBoxes);
	PxU32* boxIndices_ToSort = (PxU32*)(tempMemory + sizeof(PxU32)*nbStaticBoxes*2);
	PxU32* boxIndices_Sorted = (PxU32*)(tempMemory + sizeof(PxU32)*nbStaticBoxes*3);
	PxU32 nbToSort = 0;
	PxU32 nbSorted = 0;
	for(PxU32 i=0;i<nbStaticBoxes;i++)
	{
		if(mStaticBits.isSetChecked(i))	// ### optimize check in that thing
		{
			minPosList_ToSort[nbToSort] = mStaticBoxes[i].mMinX;
			boxIndices_ToSort[nbToSort] = i;
			nbToSort++;
		}
		else
		{
			minPosList_Sorted[nbSorted] = mStaticBoxes[i].mMinX;
			boxIndices_Sorted[nbSorted] = i;
			PX_ASSERT(nbSorted==0 || minPosList_Sorted[nbSorted-1]<=minPosList_Sorted[nbSorted]);
			nbSorted++;
		}
	}
	PX_ASSERT(nbSorted+nbToSort==nbStaticBoxes);

//	EndProfile(Time);
//	printf("Part1: %d\n", Time);

//	StartProfile(Time);

	// Sort things that need sorting
	const PxU32* sorted;
	RadixSortBuffered RS;
	if(nbToSort<DEFAULT_NUM_DYNAMIC_BOXES)
	{
		sorted = mRS.Sort(minPosList_ToSort, nbToSort, RADIX_UNSIGNED).GetRanks();
	}
	else
	{
		sorted = RS.Sort(minPosList_ToSort, nbToSort, RADIX_UNSIGNED).GetRanks();
	}

//	EndProfile(Time);
//	printf("Part2: %d\n", Time);

//	StartProfile(Time);

	// Allocate final buffers that wil contain the 2 (merged) streams
	MBP_Index* newMapping = (MBP_Index*)MBP_ALLOC(sizeof(MBP_Index)*mMaxNbStaticBoxes);
	const PxU32 nbStaticSentinels = 2;
#ifdef PX_PS3
	MBP_AABB* sortedBoxes = PX_NEW(MBP_AABB)[mMaxNbStaticBoxes+nbStaticSentinels+MBP_BOX_CACHE_SIZE];	// #####
#else
	MBP_AABB* sortedBoxes = PX_NEW(MBP_AABB)[mMaxNbStaticBoxes+nbStaticSentinels];
#endif
	initSentinel(sortedBoxes[nbStaticBoxes]);
	initSentinel(sortedBoxes[nbStaticBoxes+1]);

//	EndProfile(Time);
//	printf("Part2b: %d\n", Time);

//	StartProfile(Time);

	// Merge streams to final buffers
	PxU32 offsetSorted = 0;
	PxU32 offsetNonSorted = 0;

	PxU32 nextCandidateNonSorted = offsetNonSorted<nbToSort ? minPosList_ToSort[sorted[offsetNonSorted]] : 0xffffffff;
	PxU32 nextCandidateSorted = offsetSorted<nbSorted ? minPosList_Sorted[offsetSorted] : 0xffffffff;

	for(PxU32 i=0;i<nbStaticBoxes;i++)
	{
		PxU32 boxIndex;
		{
//			minPosList_Sorted[offsetSorted] = mStaticBoxes[boxIndices_Sorted[offsetSorted]].mMinX;

			if(nextCandidateNonSorted<nextCandidateSorted)
			{
				boxIndex = boxIndices_ToSort[sorted[offsetNonSorted]];
				offsetNonSorted++;

				nextCandidateNonSorted = offsetNonSorted<nbToSort ? minPosList_ToSort[sorted[offsetNonSorted]] : 0xffffffff;
			}
			else
			{
				boxIndex = boxIndices_Sorted[offsetSorted];
				offsetSorted++;

				nextCandidateSorted = offsetSorted<nbSorted ? minPosList_Sorted[offsetSorted] : 0xffffffff;
			}
		}

		const MBP_Index OwnerIndex = mInToOut_Static[boxIndex];
		sortedBoxes[i] = mStaticBoxes[boxIndex];
		newMapping[i] = OwnerIndex;

		PX_ASSERT(mObjects[OwnerIndex].mIndex==boxIndex);
		PX_ASSERT(mObjects[OwnerIndex].isStatic());
		mObjects[OwnerIndex].mIndex = i;
	}
	PX_ASSERT(offsetSorted+offsetNonSorted==nbStaticBoxes);

//	EndProfile(Time);
//	printf("Part3: %d\n", Time);

//	StartProfile(Time);

	if(tempMemory!=stackBuffer)
		MBP_FREE(tempMemory);

	DELETEARRAY(mStaticBoxes);
	mStaticBoxes = sortedBoxes;

	MBP_FREE(mInToOut_Static);
	mInToOut_Static = newMapping;

	mStaticBits.empty();

//	EndProfile(Time);
//	printf("Part4: %d\n", Time);
}

void BoxPruner::optimizeMemory()
{
	// TODO: resize static boxes/mapping, dynamic boxes/mapping, object array
}

void BoxPruner::resizeObjects()
{
	const PxU32 newMaxNbOjects = mMaxNbObjects ? mMaxNbObjects + DEFAULT_NB_ENTRIES : DEFAULT_NB_ENTRIES;
	MBPEntry* newObjects = PX_NEW(MBPEntry)[newMaxNbOjects];
	if(mNbObjects)
		PxMemCopy(newObjects, mObjects, mNbObjects*sizeof(MBPEntry));
#ifdef PX_DEBUG
	for(PxU32 i=mNbObjects;i<newMaxNbOjects;i++)
		newObjects[i].mUpdated = false;
#endif
	DELETEARRAY(mObjects);
	mObjects = newObjects;
	mMaxNbObjects = newMaxNbOjects;
}

static MBP_AABB* resizeBoxes(PxU32 oldNbBoxes, PxU32 newNbBoxes, const MBP_AABB* boxes)
{
	MBP_AABB* newBoxes = PX_NEW(MBP_AABB)[newNbBoxes];
	if(oldNbBoxes)
		PxMemCopy(newBoxes, boxes, oldNbBoxes*sizeof(MBP_AABB));
	DELETEARRAY(boxes);
	return newBoxes;
}

static MBP_Index* resizeMapping(PxU32 oldNbBoxes, PxU32 newNbBoxes, MBP_Index* mapping)
{
	MBP_Index* newMapping = (MBP_Index*)MBP_ALLOC(sizeof(MBP_Index)*newNbBoxes);
	if(oldNbBoxes)
		PxMemCopy(newMapping, mapping, oldNbBoxes*sizeof(MBP_Index));
	MBP_FREE(mapping);
	return newMapping;
}

static PX_FORCE_INLINE void MTF(MBP_AABB* PX_RESTRICT dynamicBoxes, MBP_Index* PX_RESTRICT inToOut_Dynamic, MBPEntry* PX_RESTRICT objects, const MBP_AABB& bounds, PxU32 frontIndex, MBPEntry& updatedObject)
{
	const PxU32 updatedIndex = updatedObject.mIndex;
	if(frontIndex!=updatedIndex)
	{
		const MBP_AABB box0 = dynamicBoxes[frontIndex];
		dynamicBoxes[frontIndex] = bounds;
		dynamicBoxes[updatedIndex] = box0;

		const MBP_Index index0 = inToOut_Dynamic[frontIndex];
		inToOut_Dynamic[frontIndex] = inToOut_Dynamic[updatedIndex];
		inToOut_Dynamic[updatedIndex] = index0;

		objects[index0].mIndex = updatedIndex;
		updatedObject.mIndex = frontIndex;
	}
	else
	{
		dynamicBoxes[frontIndex] = bounds;
	}
}

MBP_Index BoxPruner::addObject(const MBP_AABB& bounds, PxU32 mbpHandle, bool isStatic)
{
	PX_ASSERT((DecodeHandle_IsStatic(mbpHandle) && isStatic) || (!DecodeHandle_IsStatic(mbpHandle) && !isStatic));

	MBP_Index handle;
	if(mFirstFree!=INVALID_ID)
	{
		handle = MBP_Index(mFirstFree);
		mFirstFree = mObjects[handle].mIndex;
	}
	else
	{
		if(mMaxNbObjects==mNbObjects)
			resizeObjects();

		handle = MBP_Index(mNbObjects);
	}
	mNbObjects++;
#ifdef MBP_USE_WORDS
	PX_ASSERT(mNbObjects<0xffff);
#endif
	///

	PxU32 boxIndex;
	if(isStatic)
	{
		if(mMaxNbStaticBoxes==mNbStaticBoxes)
		{
			const PxU32 newMaxNbBoxes = mMaxNbStaticBoxes ? mMaxNbStaticBoxes + DEFAULT_NB_ENTRIES : DEFAULT_NB_ENTRIES;
			mStaticBoxes = resizeBoxes(mNbStaticBoxes, newMaxNbBoxes, mStaticBoxes);
			mInToOut_Static = resizeMapping(mNbStaticBoxes, newMaxNbBoxes, mInToOut_Static);
			mMaxNbStaticBoxes = newMaxNbBoxes;
		}

		boxIndex = mNbStaticBoxes++;
		mStaticBoxes[boxIndex] = bounds;
		mInToOut_Static[boxIndex] = handle;
		mNeedsSorting = true;
		mStaticBits.setBitChecked(boxIndex);
	}
	else
	{
		if(mMaxNbDynamicBoxes==mNbDynamicBoxes)
		{
			const PxU32 newMaxNbBoxes = mMaxNbDynamicBoxes ? mMaxNbDynamicBoxes + DEFAULT_NB_ENTRIES : DEFAULT_NB_ENTRIES;
			mDynamicBoxes = resizeBoxes(mNbDynamicBoxes, newMaxNbBoxes, mDynamicBoxes);
			mInToOut_Dynamic = resizeMapping(mNbDynamicBoxes, newMaxNbBoxes, mInToOut_Dynamic);
			mMaxNbDynamicBoxes = newMaxNbBoxes;

			MBP_FREE(mPosList);
			mPosList = (PxU32*)MBP_ALLOC((newMaxNbBoxes+1)*sizeof(PxU32));
		}

		boxIndex = mNbDynamicBoxes++;
		mDynamicBoxes[boxIndex] = bounds;
		mInToOut_Dynamic[boxIndex] = handle;
	}

	mObjects[handle].mIndex = boxIndex;
	mObjects[handle].mMBPHandle = mbpHandle;
#ifdef PX_DEBUG
	mObjects[handle].mUpdated = !isStatic;
#endif

	if(!isStatic)
	{
		MTF(mDynamicBoxes, mInToOut_Dynamic, mObjects, bounds, mNbUpdatedBoxes, mObjects[handle]);
		mNbUpdatedBoxes++;
		mPrevNbUpdatedBoxes = 0;
		mNeedsSortingSleeping = true;
		PX_ASSERT(mNbUpdatedBoxes<=mNbDynamicBoxes);
	}
	return handle;
}

// Moves box 'last_index' to location 'removedBoxIndex'
static PX_FORCE_INLINE void remove(MBPEntry* PX_RESTRICT objects, MBP_Index* PX_RESTRICT mapping, MBP_AABB* PX_RESTRICT boxes, PxU32 removedBoxIndex, PxU32 lastIndex)
{
	const PxU32 movedBoxHandle = mapping[lastIndex];
	boxes[removedBoxIndex] = boxes[lastIndex];		// Relocate box data
	mapping[removedBoxIndex] = MBP_Index(movedBoxHandle);		// Relocate mapping data
	MBPEntry& movedObject = objects[movedBoxHandle];
	PX_ASSERT(movedObject.mIndex==lastIndex);		// Checks index of moved box was indeed its old location
	movedObject.mIndex = removedBoxIndex;			// Adjust index of moved box to reflect its new location
}

void BoxPruner::removeObject(MBP_Index handle)
{
	PX_ASSERT(handle<mMaxNbObjects);

	MBPEntry& object = mObjects[handle];
	/*const*/ PxU32 removedBoxIndex = object.mIndex;

	MBP_Index* PX_RESTRICT mapping;
	MBP_AABB* PX_RESTRICT boxes;
	PxU32 lastIndex;
	PxU32 maxNbBoxes;
	if(!object.isStatic())
	{
		mPrevNbUpdatedBoxes = 0;
		mNeedsSortingSleeping = true;

		PX_ASSERT(mInToOut_Dynamic[removedBoxIndex]==handle);
		const bool isUpdated = removedBoxIndex<mNbUpdatedBoxes;
		PX_ASSERT(isUpdated==object.mUpdated);
		if(isUpdated)
		{
			PX_ASSERT(mNbUpdatedBoxes);
			if(mNbUpdatedBoxes!=mNbDynamicBoxes)
			{
				// Removing the object will create this pattern, which is wrong:
				// UUUUUUUUUUUNNNNNNNNN......... original
				// UUUUUU.UUUUNNNNNNNNN......... remove U
				// UUUUUUNUUUUNNNNNNNN.......... move N
				//
				// What we want instead is:
				// UUUUUUUUUUUNNNNNNNNN......... original
				// UUUUUU.UUUUNNNNNNNNN......... remove U
				// UUUUUUUUUU.NNNNNNNNN......... move U
				// UUUUUUUUUUNNNNNNNNN.......... move N
				const PxU32 lastUpdatedIndex = mNbUpdatedBoxes-1;

				remove(mObjects, mInToOut_Dynamic, mDynamicBoxes, removedBoxIndex, lastUpdatedIndex);	// Move last U to removed U
				//Remove(mObjects, mInToOut_Dynamic, mDynamicBoxes, lastUpdatedIndex, --mNbDynamicBoxes);	// Move last N to last U
				removedBoxIndex = lastUpdatedIndex;
			}
			mNbUpdatedBoxes--;
		}

//		remove(mObjects, mInToOut_Dynamic, mDynamicBoxes, removedBoxIndex, --mNbDynamicBoxes);
		mapping = mInToOut_Dynamic;
		boxes = mDynamicBoxes;
		lastIndex = --mNbDynamicBoxes;
		maxNbBoxes = mMaxNbDynamicBoxes;

		// ### adjust size of mPosList ?
	}
	else
	{
		PX_ASSERT(mInToOut_Static[removedBoxIndex]==handle);

		mNeedsSorting = true;
		mStaticBits.setBitChecked(removedBoxIndex);

//		remove(mObjects, mInToOut_Static, mStaticBoxes, removedBoxIndex, --mNbStaticBoxes);
		mapping = mInToOut_Static;
		boxes = mStaticBoxes;
		lastIndex = --mNbStaticBoxes;
		maxNbBoxes = mMaxNbStaticBoxes;
	}
	remove(mObjects, mapping, boxes, removedBoxIndex, lastIndex);

	PX_UNUSED(maxNbBoxes);
/*	if(lastIndex+DEFAULT_NB_ENTRIES<maxNbBoxes/2)
	{
		int stop=1;
	}*/

	object.mIndex		= mFirstFree;
	object.mMBPHandle	= INVALID_ID;
//	printf("Invalid: %d\n", handle);
	mFirstFree			= handle;
	mNbObjects--;

#ifdef PX_DEBUG
	object.mUpdated = false;
#endif
}

void BoxPruner::updateObject(const MBP_AABB& bounds, MBP_Index handle)
{
	MBPEntry& object = mObjects[handle];
	if(!object.isStatic())
	{
		// MTF on updated box
		const bool isContinuouslyUpdated = object.mIndex<mPrevNbUpdatedBoxes;
		if(!isContinuouslyUpdated)
			mNeedsSortingSleeping = true;
//		printf("%d: %d\n", handle, isContinuouslyUpdated);

		const bool isUpdated = object.mIndex<mNbUpdatedBoxes;
		PX_ASSERT(isUpdated==object.mUpdated);
		if(!isUpdated)
		{
#ifdef PX_DEBUG
			object.mUpdated = true;
#endif
			MTF(mDynamicBoxes, mInToOut_Dynamic, mObjects, bounds, mNbUpdatedBoxes, object);
			mNbUpdatedBoxes++;
			PX_ASSERT(mNbUpdatedBoxes<=mNbDynamicBoxes);
		}
		else
		{
			mDynamicBoxes[object.mIndex] = bounds;
		}
	}
	else
	{
		mStaticBoxes[object.mIndex] = bounds;
		mNeedsSorting = true;	// ### not always!
		mStaticBits.setBitChecked(object.mIndex);
	}
}

PxU32 BoxPruner::retrieveBounds(MBP_AABB& bounds, MBP_Index handle) const
{
	const MBPEntry& object = mObjects[handle];
	if(!object.isStatic())
		bounds = mDynamicBoxes[object.mIndex];
	else
		bounds = mStaticBoxes[object.mIndex];

	return object.mMBPHandle;
}

void BoxPruner::setBounds(MBP_Index handle, const MBP_AABB& bounds)
{
	PX_ASSERT(handle < mNbObjects);
	const MBPEntry& object = mObjects[handle];
	if(!object.isStatic())
	{
		PX_ASSERT(object.mIndex < mNbDynamicBoxes);
		mDynamicBoxes[object.mIndex] = bounds;
	}
	else
	{
		PX_ASSERT(object.mIndex < mNbStaticBoxes);
		mStaticBoxes[object.mIndex] = bounds;
	}
}

#ifndef MBP_SIMD_OVERLAP
static PX_FORCE_INLINE Ps::IntBool intersect2D(const MBP_AABB& a, const MBP_AABB& b)
{
#ifdef MBP_USE_NO_CMP_OVERLAP
		// PT: warning, only valid with the special encoding in InitFrom2
		const PxU32 bits0 = (b.mMaxY - a.mMinY)&0x80000000;
		const PxU32 bits1 = (b.mMaxZ - a.mMinZ)&0x80000000;
		const PxU32 bits2 = (a.mMaxY - b.mMinY)&0x80000000;
		const PxU32 bits3 = (a.mMaxZ - b.mMinZ)&0x80000000;
		const PxU32 mask = bits0|(bits1>>1)|(bits2>>2)|(bits3>>3);
		return !mask;

	/*	const PxU32 d0 = (b.mMaxY<<16)|a.mMaxY;
		const PxU32 d0b = (b.mMaxZ<<16)|a.mMaxZ;
		const PxU32 d1 = (a.mMinY<<16)|b.mMinY;
		const PxU32 d1b = (a.mMinZ<<16)|b.mMinZ;
		const PxU32 mask = (d0 - d1) | (d0b - d1b);
		return !(mask & 0x80008000);*/
#else
	if(//mMaxX < a.mMinX || a.mMaxX < mMinX
//		||
		b.mMaxY < a.mMinY || a.mMaxY < b.mMinY
	||
		b.mMaxZ < a.mMinZ || a.mMaxZ < b.mMinZ
	)
		return FALSE;
	return TRUE;
#endif
}
#endif

#ifdef MBP_USE_NO_CMP_OVERLAP_3D
static PX_FORCE_INLINE BOOL intersect3D(const IAABB& a, const IAABB& b)
{
	// PT: warning, only valid with the special encoding in InitFrom2
	const PxU32 bits0 = (b.mMaxY - a.mMinY)&0x80000000;
	const PxU32 bits1 = (b.mMaxZ - a.mMinZ)&0x80000000;
	const PxU32 bits2 = (a.mMaxY - b.mMinY)&0x80000000;
	const PxU32 bits3 = (a.mMaxZ - b.mMinZ)&0x80000000;
	const PxU32 bits4 = (b.mMaxX - a.mMinX)&0x80000000;
	const PxU32 bits5 = (a.mMaxX - b.mMinX)&0x80000000;
	const PxU32 mask = bits0|(bits1>>1)|(bits2>>2)|(bits3>>3)|(bits4>>4)|(bits5>>5);
	return !mask;
}
#endif

#ifdef CHECK_NB_OVERLAPS
static PxU32 gNbOverlaps = 0;
#endif

#ifdef TRY_COMPRESSION
static PxU32	gBitMask = 0;
static PxU32	gBitCount = 0;
static PxU32	gCompressedSize = 0;
static PxU32	gCompressed[8182];

static void InitCompression()
{
	gCompressedSize = gBitMask = gBitCount = 0;
}

static void outputBit(bool bit)
{
	gBitMask<<=1;
	gBitMask |= PxU32(bit);
	gBitCount++;
	if(gBitCount==32)
	{
		gBitCount = 0;
		gCompressed[gCompressedSize++] = gBitMask;
	}
}

static void outputBits(PxU32 bits, PxU32 nbBits)
{
	PxU32 mask = 1<<(nbBits-1);
	while(mask)
	{
		outputBit((bits & mask)!=0);
		mask >>= 1;
	}
}

static void endBits()
{
	while(gBitCount)
		outputBit(false);
}
#endif

static PX_FORCE_INLINE void outputPair_DynamicDynamic(
												MBP_PairManager& pairManager,
												PxU32 index0, PxU32 index1,
												const MBP_Index* PX_RESTRICT inToOut_Dynamic,
												const MBPEntry* PX_RESTRICT objects,
												const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* mbpObjects
												)
{
#ifdef CHECK_NB_OVERLAPS
	gNbOverlaps++;
#endif

#ifdef TRY_COMPRESSION
	if(0)
	{
		static PxU32 uncompressed = 0;
		static PxU32 prevIndex0 = 0;
		static PxU32 prevIndex1 = 0;
		if(prevIndex0>index0)
		{
			prevIndex0 = 0;
			prevIndex1 = 0;
			endBits();
			printf("Total: %d | %d | %f\n", gCompressedSize*4, uncompressed, float(gCompressedSize*4)/float(uncompressed));
			uncompressed = 0;
			InitCompression();
		}
		PxU32 encoded0 = index0-prevIndex0;
		PxU32 encoded1 = index1-index0;
//		PxU32 encoded1 = index1-prevIndex1;
//		printf("%d | %d\n", encoded0, encoded1);

		if(encoded0<16 && encoded1<128)
		{
			// 1 bit control
			outputBit(true);
			// 4+7 bits indices
			outputBits(encoded0, 4);
			outputBits(encoded1, 7);
		}
		else if(encoded0<256 && encoded1<256)
		{
			// 1 bit control
			outputBit(false);
			// 8*2 bits for indices
			outputBits(encoded0, 8);
			outputBits(encoded1, 8);
		}
		else
		{
			printf("OOPS\n");
		}

/*		if(encoded0<16 && encoded1<16)
		{
			// 1 bit control
			compressed += 1;
			// 4*2 bits indices
			compressed += 8;
		}
		else if(encoded0<256 && encoded1<256)
		{
			// 1 bit control
			compressed += 1;
			// 8*2 bits for indices
			compressed += 16;
		}
		else
		{
			printf("OOPS\n");
		}*/

/*		if(encoded0==0 && encoded1<127)
		{
			// 1 byte
			compressed += 1;
//			printf("Code0: %d\n", encoded1);
		}
		else if(encoded0<127 && encoded1<256)
		{
			// 2 bytes
			compressed += 2;
//			printf("Code1: %d | %d\n", encoded0, encoded1);
		}
		else
		{
			// 4 bytes
			compressed += 4;
//			printf("Code2: %d | %d\n", encoded0, encoded1);
		}*/
		prevIndex0 = index0;
		prevIndex1 = index1;
		uncompressed += sizeof(PxU32)*2;
	}
#endif

	PX_ASSERT(index0!=index1);
	const PxU32 objectIndex0 = inToOut_Dynamic[index0];
	const PxU32 objectIndex1 = inToOut_Dynamic[index1];
	const PxU32 id0 = objects[objectIndex0].mMBPHandle;
	const PxU32 id1 = objects[objectIndex1].mMBPHandle;
	pairManager.addPair(id0, id1, groups, mbpObjects);
}

static PX_FORCE_INLINE void outputPair_DynamicStatic(
												MBP_PairManager& pairManager,
												PxU32 index0, PxU32 index1,
												const MBP_Index* PX_RESTRICT inToOut_Dynamic, const MBP_Index* PX_RESTRICT inToOut_Static,
												const MBPEntry* PX_RESTRICT objects,
												const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* mbpObjects
												)
{
#ifdef CHECK_NB_OVERLAPS
	gNbOverlaps++;
#endif
	const PxU32 objectIndex0 = inToOut_Dynamic[index0];
	const PxU32 objectIndex1 = inToOut_Static[index1];
	const PxU32 id0 = objects[objectIndex0].mMBPHandle;
	const PxU32 id1 = objects[objectIndex1].mMBPHandle;
//	printf("2: %d %d\n", index0, index1);
//	printf("3: %d %d\n", objectIndex0, objectIndex1);
	pairManager.addPair(id0, id1, groups, mbpObjects);
}



MBPOS_TmpBuffers::MBPOS_TmpBuffers() :
	mNbSleeping					(0),
	mNbUpdated					(0),
	mInToOut_Dynamic_Sleeping	(NULL),
	mSleepingDynamicBoxes		(NULL),
	mUpdatedDynamicBoxes		(NULL)
{
}

MBPOS_TmpBuffers::~MBPOS_TmpBuffers()
{
//	printf("mNbSleeping: %d\n", mNbSleeping);
	if(mInToOut_Dynamic_Sleeping!=mInToOut_Dynamic_Sleeping_Stack)
		MBP_FREE(mInToOut_Dynamic_Sleeping);

	if(mSleepingDynamicBoxes!=mSleepingDynamicBoxes_Stack)
	{
#ifdef PX_PS3
		Ps::AlignedAllocator<128>().deallocate(mSleepingDynamicBoxes);
		mSleepingDynamicBoxes = NULL;
#else
		DELETEARRAY(mSleepingDynamicBoxes);
#endif
	}

	if(mUpdatedDynamicBoxes!=mUpdatedDynamicBoxes_Stack)
	{
#ifdef PX_PS3
		Ps::AlignedAllocator<128>().deallocate(mUpdatedDynamicBoxes);
		mUpdatedDynamicBoxes = NULL;
#else
		DELETEARRAY(mUpdatedDynamicBoxes);
#endif
	}
	mNbSleeping = 0;
	mNbUpdated = 0;
}

void MBPOS_TmpBuffers::allocateSleeping(PxU32 nbSleeping, PxU32 nbSentinels)
{
	if(nbSleeping>mNbSleeping)
	{
		if(mInToOut_Dynamic_Sleeping!=mInToOut_Dynamic_Sleeping_Stack)
			MBP_FREE(mInToOut_Dynamic_Sleeping);
		if(mSleepingDynamicBoxes!=mSleepingDynamicBoxes_Stack)
		{
#ifdef PX_PS3
			Ps::AlignedAllocator<128>().deallocate(mSleepingDynamicBoxes);
			mSleepingDynamicBoxes = NULL;
#else
			DELETEARRAY(mSleepingDynamicBoxes);
#endif
		}

		if(nbSleeping+nbSentinels<=STACK_BUFFER_SIZE)
		{
			mSleepingDynamicBoxes = mSleepingDynamicBoxes_Stack;
			mInToOut_Dynamic_Sleeping = mInToOut_Dynamic_Sleeping_Stack;
		}
		else
		{
#ifdef PX_PS3
//			mSleepingDynamicBoxes = PX_NEW_TEMP(MBP_AABB)[nbSleeping+nbSentinels+MBP_BOX_CACHE_SIZE];	// #######
			mSleepingDynamicBoxes = (MBP_AABB*)Ps::AlignedAllocator<128>().allocate(sizeof(MBP_AABB)*(nbSleeping+nbSentinels+MBP_BOX_CACHE_SIZE), __FILE__, __LINE__);
#else
			mSleepingDynamicBoxes = PX_NEW_TEMP(MBP_AABB)[nbSleeping+nbSentinels];
#endif
			mInToOut_Dynamic_Sleeping = (MBP_Index*)MBP_ALLOC(sizeof(MBP_Index)*nbSleeping);
		}
		mNbSleeping = nbSleeping;
	}
}

void MBPOS_TmpBuffers::allocateUpdated(PxU32 nbUpdated, PxU32 nbSentinels)
{
	if(nbUpdated>mNbUpdated)
	{
		if(mUpdatedDynamicBoxes!=mUpdatedDynamicBoxes_Stack)
		{
#ifdef PX_PS3
			Ps::AlignedAllocator<128>().deallocate(mUpdatedDynamicBoxes);
			mUpdatedDynamicBoxes = NULL;
#else
			DELETEARRAY(mUpdatedDynamicBoxes);
#endif
		}

		if(nbUpdated+nbSentinels<=STACK_BUFFER_SIZE)
			mUpdatedDynamicBoxes = mUpdatedDynamicBoxes_Stack;
		else
		{
#ifdef PX_PS3
//			mUpdatedDynamicBoxes = PX_NEW_TEMP(MBP_AABB)[nbUpdated+nbSentinels+MBP_BOX_CACHE_SIZE];	// #######
			mUpdatedDynamicBoxes = (MBP_AABB*)Ps::AlignedAllocator<128>().allocate(sizeof(MBP_AABB)*(nbUpdated+nbSentinels+MBP_BOX_CACHE_SIZE), __FILE__, __LINE__);
#else
			mUpdatedDynamicBoxes = PX_NEW_TEMP(MBP_AABB)[nbUpdated+nbSentinels];
#endif
		}
		mNbUpdated = nbUpdated;
	}
}

//#define TEST2

void BoxPruner::preparePruning(MBPOS_TmpBuffers& buffers)
{
PxU32 _saved = mNbUpdatedBoxes;
mNbUpdatedBoxes = 0;

	if(mPrevNbUpdatedBoxes!=_saved)
		mNeedsSortingSleeping = true;

	PxU32 nb = mNbDynamicBoxes;
	if(!nb)
	{
		mInput.mNeeded = false;
		mPrevNbUpdatedBoxes = 0;
		mNeedsSortingSleeping = true;
		return;
	}
	const MBP_AABB* PX_RESTRICT dynamicBoxes = mDynamicBoxes;
	PxU32* PX_RESTRICT posList = mPosList;

#ifdef PX_DEBUG
	PxU32 verifyNbUpdated = 0;
	for(PxU32 i=0;i<mMaxNbObjects;i++)
	{
		if(mObjects[i].mUpdated)
			verifyNbUpdated++;
	}
	PX_ASSERT(verifyNbUpdated==_saved);
#endif

	// Build main list using the primary axis

	PxU32 nbUpdated = 0;
	PxU32 nbNonUpdated = 0;
	{
		nbUpdated = _saved;
		nbNonUpdated = nb - _saved;
		for(PxU32 i=0;i<nbUpdated;i++)
		{
#ifdef PX_DEBUG
			const PxU32 objectIndex = mInToOut_Dynamic[i];
			PX_ASSERT(mObjects[objectIndex].mUpdated);
			mObjects[objectIndex].mUpdated = false;
#endif
			posList[i] = dynamicBoxes[i].mMinX;
		}
		if(mNeedsSortingSleeping)
		{
			for(PxU32 i=0;i<nbNonUpdated;i++)
			{
#ifdef PX_DEBUG
				const PxU32 objectIndex = mInToOut_Dynamic[i];
				PX_ASSERT(!mObjects[objectIndex].mUpdated);
#endif
				PxU32 j = i + nbUpdated;
				posList[j] = dynamicBoxes[j].mMinX;
			}
		}
#ifdef PX_DEBUG
		else
		{
			for(PxU32 i=0;i<nbNonUpdated;i++)
			{
				const PxU32 objectIndex = mInToOut_Dynamic[i];
				PX_ASSERT(!mObjects[objectIndex].mUpdated);
				PxU32 j = i + nbUpdated;
				PX_ASSERT(posList[j] == dynamicBoxes[j].mMinX);
			}
		}
#endif
	}
	PX_ASSERT(nbUpdated==verifyNbUpdated);
	PX_ASSERT(nbUpdated+nbNonUpdated==nb);
	mNbUpdatedBoxes = nbUpdated;
	if(!nbUpdated)
	{
		mInput.mNeeded = false;
		mPrevNbUpdatedBoxes = 0;
		mNeedsSortingSleeping = true;
		return;
	}

	mPrevNbUpdatedBoxes = mNbUpdatedBoxes;

	///////

	// ### TODO: no need to recreate those buffers each frame!
	MBP_Index* PX_RESTRICT inToOut_Dynamic_Sleeping = NULL;
	MBP_AABB* PX_RESTRICT sleepingDynamicBoxes = NULL;
	if(nbNonUpdated)
	{
		if(mNeedsSortingSleeping)
		{
			const PxU32* PX_RESTRICT sorted = mRS.Sort(posList+nbUpdated, nbNonUpdated, RADIX_UNSIGNED).GetRanks();

#ifdef MBP_USE_SENTINELS
			const PxU32 nbSentinels = 2;
#else
			const PxU32 nbSentinels = 0;
#endif
			buffers.allocateSleeping(nbNonUpdated, nbSentinels);
			sleepingDynamicBoxes = buffers.mSleepingDynamicBoxes;
			inToOut_Dynamic_Sleeping = buffers.mInToOut_Dynamic_Sleeping;
			for(PxU32 i=0;i<nbNonUpdated;i++)
			{
				const PxU32 sortedIndex = nbUpdated+sorted[i];
				sleepingDynamicBoxes[i] = dynamicBoxes[sortedIndex];
				inToOut_Dynamic_Sleeping[i] = mInToOut_Dynamic[sortedIndex];
			}
#ifdef MBP_USE_SENTINELS
			initSentinel(sleepingDynamicBoxes[nbNonUpdated]);
			initSentinel(sleepingDynamicBoxes[nbNonUpdated+1]);
#endif
			mNeedsSortingSleeping = false;
		}
		else
		{
			sleepingDynamicBoxes = buffers.mSleepingDynamicBoxes;
			inToOut_Dynamic_Sleeping = buffers.mInToOut_Dynamic_Sleeping;
#ifdef PX_DEBUG
			for(PxU32 i=0;i<nbNonUpdated-1;i++)
				PX_ASSERT(sleepingDynamicBoxes[i].mMinX<=sleepingDynamicBoxes[i+1].mMinX);
#endif
		}
	}
	else
	{
		mNeedsSortingSleeping = true;
	}

	///////

//	posList[nbUpdated] = MAX_PxU32;
//	nb = nbUpdated;

	// Sort the list
//	const PxU32* PX_RESTRICT sorted = mRS.Sort(posList, nbUpdated+1, RADIX_UNSIGNED).GetRanks();
	const PxU32* PX_RESTRICT sorted = mRS.Sort(posList, nbUpdated, RADIX_UNSIGNED).GetRanks();

#ifdef MBP_USE_SENTINELS
	const PxU32 nbSentinels = 2;
#else
	const PxU32 nbSentinels = 0;
#endif
	buffers.allocateUpdated(nbUpdated, nbSentinels);
	MBP_AABB* PX_RESTRICT updatedDynamicBoxes = buffers.mUpdatedDynamicBoxes;
	MBP_Index* PX_RESTRICT inToOut_Dynamic = (MBP_Index*)mRS.GetRecyclable();
	for(PxU32 i=0;i<nbUpdated;i++)
	{
		const PxU32 sortedIndex = sorted[i];
		updatedDynamicBoxes[i] = dynamicBoxes[sortedIndex];
		inToOut_Dynamic[i] = mInToOut_Dynamic[sortedIndex];
	}
#ifdef MBP_USE_SENTINELS
	initSentinel(updatedDynamicBoxes[nbUpdated]);
	initSentinel(updatedDynamicBoxes[nbUpdated+1]);
#endif
	dynamicBoxes = updatedDynamicBoxes;

	mInput.mObjects						= mObjects;					// Can be shared (1)
	mInput.mUpdatedDynamicBoxes			= updatedDynamicBoxes;		// Can be shared (2) => buffers.mUpdatedDynamicBoxes;
	mInput.mSleepingDynamicBoxes		= sleepingDynamicBoxes;
	mInput.mInToOut_Dynamic				= inToOut_Dynamic;			// Can be shared (3) => (MBP_Index*)mRS.GetRecyclable();
	mInput.mInToOut_Dynamic_Sleeping	= inToOut_Dynamic_Sleeping;
	mInput.mNbUpdated					= nbUpdated;				// Can be shared (4)
	mInput.mNbNonUpdated				= nbNonUpdated;
	mInput.mNeeded						= true;
}

void BoxPruner::prepareBIPPruning(const MBPOS_TmpBuffers& buffers)
{
	if(!mNbUpdatedBoxes || !mNbStaticBoxes)
	{
		mInput.mBIPInput.mNeeded = false;
		return;
	}

	mInput.mBIPInput.mObjects			= mObjects;						// Can be shared (1)
	mInput.mBIPInput.mNbUpdatedBoxes	= mNbUpdatedBoxes;				// Can be shared (4)
	mInput.mBIPInput.mNbStaticBoxes		= mNbStaticBoxes;
//	mInput.mBIPInput.mDynamicBoxes		= mDynamicBoxes;
	mInput.mBIPInput.mDynamicBoxes		= buffers.mUpdatedDynamicBoxes;	// Can be shared (2)
	mInput.mBIPInput.mStaticBoxes		= mStaticBoxes;
	mInput.mBIPInput.mInToOut_Static	= mInToOut_Static;
	mInput.mBIPInput.mInToOut_Dynamic	= (const MBP_Index*)mRS.GetRecyclable();	// Can be shared (3)
	mInput.mBIPInput.mNeeded			= true;
}

static void DoCompleteBoxPruning(MBP_PairManager* PX_RESTRICT pairManager, const BoxPruning_Input& input, const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* mbpObjects)
{
	const MBPEntry* PX_RESTRICT objects						= input.mObjects;
	const MBP_AABB* PX_RESTRICT updatedDynamicBoxes			= input.mUpdatedDynamicBoxes;
	const MBP_AABB* PX_RESTRICT sleepingDynamicBoxes		= input.mSleepingDynamicBoxes;
	const MBP_Index* PX_RESTRICT inToOut_Dynamic			= input.mInToOut_Dynamic;
	const MBP_Index* PX_RESTRICT inToOut_Dynamic_Sleeping	= input.mInToOut_Dynamic_Sleeping;
	const PxU32 nbUpdated 									= input.mNbUpdated;
	const PxU32 nbNonUpdated								= input.mNbNonUpdated;

	//

	if(nbNonUpdated)
	{
		const PxU32 nb0 = nbUpdated;
		const PxU32 nb1 = nbNonUpdated;

		//
		const PxU32 lastSortedIndex0 = nb0;
		const PxU32 lastSortedIndex1 = nb1;
		PxU32 sortedIndex0 = 0;
		PxU32 runningIndex1 = 0;

		SIMD_OVERLAP_INIT

		while(runningIndex1<lastSortedIndex1 && sortedIndex0<lastSortedIndex0)
		{
			const PxU32 index0 = sortedIndex0++;
			const MBP_AABB& box0 = updatedDynamicBoxes[index0];
			const PxU32 limit = box0.mMaxX;
	//const PxU32 id0 = mObjects[inToOut_Dynamic[index0]].mMBPHandle;
			SIMD_OVERLAP_PRELOAD_BOX0

			const PxU32 l = box0.mMinX;
			while(
#ifndef MBP_USE_SENTINELS
				runningIndex1<lastSortedIndex1 &&
#endif
				sleepingDynamicBoxes[runningIndex1].mMinX<l)
				runningIndex1++;

			PxU32 index1 = runningIndex1;

			while(
#ifndef MBP_USE_SENTINELS
				index1<lastSortedIndex1 &&
#endif
				sleepingDynamicBoxes[index1].mMinX<=limit)
			{
				MBP_OVERLAP_TEST(sleepingDynamicBoxes[index1])
				{
					outputPair_DynamicStatic(
						*pairManager,
						index0, index1, inToOut_Dynamic, inToOut_Dynamic_Sleeping, objects, groups, mbpObjects
						);
				}

#ifdef TEST2
				if(
	#ifndef MBP_USE_SENTINELS
					index1+1<lastSortedIndex1 &&
	#endif
					sleepingDynamicBoxes[index1+1].mMinX<=limit)
				{
					MBP_OVERLAP_TEST(sleepingDynamicBoxes[index1+1])
					{
						outputPair_DynamicStatic(
							*pairManager,
							index0, index1+1, inToOut_Dynamic, inToOut_Dynamic_Sleeping, objects, groups
							);
					}
				}
				index1+=2;
#else
				index1++;
#endif
			}
		}

		////

		PxU32 sortedIndex1 = 0;
		PxU32 runningIndex0 = 0;
		while(runningIndex0<lastSortedIndex0 && sortedIndex1<lastSortedIndex1)
		{
			const PxU32 index0 = sortedIndex1++;
			const MBP_AABB& box0 = sleepingDynamicBoxes[index0];
			const PxU32 limit = box0.mMaxX;
	//const PxU32 id1 = mObjects[mInToOut_Static[index0]].mMBPHandle;
			SIMD_OVERLAP_PRELOAD_BOX0

//			const PxU32 l = sleepingDynamicBoxes[index0].mMinX;
			const PxU32 l = box0.mMinX;	// ### PhysX change
			while(updatedDynamicBoxes[runningIndex0].mMinX<=l)
				runningIndex0++;

			PxU32 index1 = runningIndex0;

			while(updatedDynamicBoxes[index1].mMinX<=limit)
			{
				MBP_OVERLAP_TEST(updatedDynamicBoxes[index1])
				{
					outputPair_DynamicStatic(
						*pairManager,
						index1, index0, inToOut_Dynamic, inToOut_Dynamic_Sleeping, objects, groups, mbpObjects
						);
				}

#ifdef TEST2
				if(updatedDynamicBoxes[index1+1].mMinX<=limit)
				{
					MBP_OVERLAP_TEST(updatedDynamicBoxes[index1+1])
					{
						outputPair_DynamicStatic(
							*pairManager,
							index1+1, index0, inToOut_Dynamic, inToOut_Dynamic_Sleeping, objects, groups
							);
					}
				}
				index1+=2;
#else
				index1++;
#endif
			}
		}
	}

	///////

	SIMD_OVERLAP_INIT

	// Prune the list
	const PxU32 lastSortedIndex = nbUpdated;
	PxU32 sortedIndex = 0;
	PxU32 runningIndex = 0;
	while(runningIndex<lastSortedIndex && sortedIndex<lastSortedIndex)
	{
		const PxU32 index0 = sortedIndex++;
		const MBP_AABB& box0 = updatedDynamicBoxes[index0];
		const PxU32 limit = box0.mMaxX;

		SIMD_OVERLAP_PRELOAD_BOX0

//		const PxU32 l = updatedDynamicBoxes[index0].mMinX;
		const PxU32 l = box0.mMinX;	// ### PhysX change
		while(updatedDynamicBoxes[runningIndex++].mMinX<l);

		if(runningIndex<lastSortedIndex)
		{
			PxU32 index1 = runningIndex;
			while(updatedDynamicBoxes[index1].mMinX<=limit)
			{
				MBP_OVERLAP_TEST(updatedDynamicBoxes[index1])
				{
					outputPair_DynamicDynamic(
						*pairManager,
						index0, index1, inToOut_Dynamic, objects, groups, mbpObjects
						);
				}
#ifdef TEST2
				if(updatedDynamicBoxes[index1+1].mMinX<=limit)
				{
					MBP_OVERLAP_TEST(updatedDynamicBoxes[index1+1])
					{
						outputPair_DynamicDynamic(
							*pairManager,
							index0, index1+1, inToOut_Dynamic, objects, groups
							);
					}
				}
				index1+=2;
#else
				index1++;
#endif
			}
		}
	}
}


#define TWO_AT_A_TIME

static void DoBipartiteBoxPruning(MBP_PairManager* PX_RESTRICT pairManager, const BIP_Input& input, const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* mbpObjects)
{
	// ### crashes because the code expects the dynamic array to be sorted, but mDynamicBoxes is not
	// ### we should instead modify mNbUpdatedBoxes so that mNbUpdatedBoxes == mNbDynamicBoxes, and
	// ### then the proper sorting happens in CompleteBoxPruning (right?)

	const PxU32 nb0 = input.mNbUpdatedBoxes;
	const PxU32 nb1 = input.mNbStaticBoxes;

	const MBPEntry* PX_RESTRICT mObjects			= input.mObjects;
	const MBP_AABB* PX_RESTRICT dynamicBoxes		= input.mDynamicBoxes;
	const MBP_AABB* PX_RESTRICT staticBoxes			= input.mStaticBoxes;
	const MBP_Index* PX_RESTRICT inToOut_Static		= input.mInToOut_Static;
	const MBP_Index* PX_RESTRICT inToOut_Dynamic	= input.mInToOut_Dynamic;

#ifdef MBP_USE_SENTINELS
	PX_ASSERT(isSentinel(staticBoxes[nb1]));
	PX_ASSERT(isSentinel(staticBoxes[nb1+1]));
//	const MBP_AABB Saved = staticBoxes[nb1];
//	const MBP_AABB Saved1 = staticBoxes[nb1+1];
//	initSentinel(((MBP_AABB* PX_RESTRICT)staticBoxes)[nb1]);
//	initSentinel(((MBP_AABB* PX_RESTRICT)staticBoxes)[nb1+1]);
#endif

	//
	const PxU32 lastSortedIndex0 = nb0;
	const PxU32 lastSortedIndex1 = nb1;
	PxU32 sortedIndex0 = 0;
	PxU32 runningIndex1 = 0;

	SIMD_OVERLAP_INIT

	while(runningIndex1<lastSortedIndex1 && sortedIndex0<lastSortedIndex0)
	{
		const PxU32 index0 = sortedIndex0++;
		const MBP_AABB& box0 = dynamicBoxes[index0];
		const PxU32 limit = box0.mMaxX;
//const PxU32 id0 = mObjects[inToOut_Dynamic[index0]].mMBPHandle;
		SIMD_OVERLAP_PRELOAD_BOX0

		const PxU32 l = box0.mMinX;
		while(
#ifndef MBP_USE_SENTINELS
			runningIndex1<lastSortedIndex1 &&
#endif
			staticBoxes[runningIndex1].mMinX<l)
			runningIndex1++;

		PxU32 index1 = runningIndex1;

		while(
#ifndef MBP_USE_SENTINELS
			index1<lastSortedIndex1 &&
#endif
			staticBoxes[index1].mMinX<=limit)
		{
			{
			MBP_OVERLAP_TEST(staticBoxes[index1])
			{
				outputPair_DynamicStatic(
					*pairManager,
					index0, index1, inToOut_Dynamic, inToOut_Static, mObjects, groups, mbpObjects
					);
			}
			}
#ifdef TWO_AT_A_TIME
			if(
	#ifndef MBP_USE_SENTINELS
				index1+1<lastSortedIndex1 &&
	#endif
				staticBoxes[index1+1].mMinX<=limit)
			{
				MBP_OVERLAP_TEST(staticBoxes[index1+1])
				{
					outputPair_DynamicStatic(
						*pairManager,
						index0, index1+1, inToOut_Dynamic, inToOut_Static, mObjects, groups, mbpObjects
						);
				}
			}
			index1+=2;
#else
			index1++;
#endif
		}
	}

	////

	PxU32 sortedIndex1 = 0;
	PxU32 runningIndex0 = 0;
	while(runningIndex0<lastSortedIndex0 && sortedIndex1<lastSortedIndex1)
	{
		const PxU32 index0 = sortedIndex1++;
		const MBP_AABB& box0 = staticBoxes[index0];
		const PxU32 limit = box0.mMaxX;
//const PxU32 id1 = mObjects[inToOut_Static[index0]].mMBPHandle;
		SIMD_OVERLAP_PRELOAD_BOX0

//		const PxU32 l = staticBoxes[index0].mMinX;
		const PxU32 l = box0.mMinX;	// ### PhysX
		while(dynamicBoxes[runningIndex0].mMinX<=l)
			runningIndex0++;

		PxU32 index1 = runningIndex0;

		while(dynamicBoxes[index1].mMinX<=limit)
		{
			{
			MBP_OVERLAP_TEST(dynamicBoxes[index1])
			{
				outputPair_DynamicStatic(
					*pairManager,
					index1, index0, inToOut_Dynamic, inToOut_Static, mObjects, groups, mbpObjects
					);
			}
			}
#ifdef TWO_AT_A_TIME
			if(dynamicBoxes[index1+1].mMinX<=limit)
			{
				MBP_OVERLAP_TEST(dynamicBoxes[index1+1])
				{
					outputPair_DynamicStatic(
						*pairManager,
						index1+1, index0, inToOut_Dynamic, inToOut_Static, mObjects, groups, mbpObjects
						);
				}
			}
			index1+=2;
#else
			index1++;
#endif
		}
	}

//	MBP_FREE(inToOut_Dynamic);

#ifdef MBP_USE_SENTINELS
//	((MBP_AABB* PX_RESTRICT)staticBoxes)[nb1] = Saved;
//	((MBP_AABB* PX_RESTRICT)staticBoxes)[nb1+1] = Saved1;
#endif
}

#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
void BoxPruner::prepareOverlaps()
{
	if(!mNbUpdatedBoxes
		&& !mNeedsSorting	//### bugfix added for PhysX integration
		)
		return;

	if(mNeedsSorting)
	{
		staticSort();

		// PT: when a static object is added/removed/updated we need to compute the overlaps again
		// even if no dynamic box has been updated. The line below forces all dynamic boxes to be
		// sorted in PreparePruning() and tested for overlaps in BipartiteBoxPruning(). It would be
		// more efficient to:
		// a) skip the actual pruning in PreparePruning() (we only need to re-sort)
		// b) so BipartiteBoxPruning() with the new/modified boxes, not all of them
		// Well, not done yet.
		mNbUpdatedBoxes = mNbDynamicBoxes;	// ### PhysX
#ifdef PX_DEBUG
		for(PxU32 i=0;i<mNbDynamicBoxes;i++)
		{
			const PxU32 objectIndex = mInToOut_Dynamic[i];
			mObjects[objectIndex].mUpdated = true;
		}
#endif
	}
}
#endif

void BoxPruner::prepareOverlapsMT()
{
	if(!mNbUpdatedBoxes
		&& !mNeedsSorting	//### bugfix added for PhysX integration
		)
		return;

	if(mNeedsSorting)
	{
		staticSort();

		// PT: when a static object is added/removed/updated we need to compute the overlaps again
		// even if no dynamic box has been updated. The line below forces all dynamic boxes to be
		// sorted in PreparePruning() and tested for overlaps in BipartiteBoxPruning(). It would be
		// more efficient to:
		// a) skip the actual pruning in PreparePruning() (we only need to re-sort)
		// b) so BipartiteBoxPruning() with the new/modified boxes, not all of them
		// Well, not done yet.
		mNbUpdatedBoxes = mNbDynamicBoxes;	// ### PhysX
		mPrevNbUpdatedBoxes = 0;
		mNeedsSortingSleeping = true;
#ifdef PX_DEBUG
		for(PxU32 i=0;i<mNbDynamicBoxes;i++)
		{
			const PxU32 objectIndex = mInToOut_Dynamic[i];
			mObjects[objectIndex].mUpdated = true;
		}
#endif
	}

	preparePruning(mTmpBuffers);
	prepareBIPPruning(mTmpBuffers);
}

#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
void BoxPruner::findOverlaps(MBP_PairManager& pairManager, MBPOS_TmpBuffers& buffers)
{
	PX_ASSERT(!mNeedsSorting);
	if(!mNbUpdatedBoxes)
		return;

	preparePruning(buffers);
	prepareBIPPruning(buffers);

	if(mInput.mNeeded)
		DoCompleteBoxPruning(&pairManager, mInput);

	if(mInput.mBIPInput.mNeeded)
		DoBipartiteBoxPruning(&pairManager, mInput.mBIPInput);

	mNbUpdatedBoxes = 0;
}
#endif

void BoxPruner::findOverlapsMT(MBP_PairManager& pairManager, const PxcBpHandle* PX_RESTRICT groups, const MBP_Object* PX_RESTRICT mbpObjects)
{
	PX_ASSERT(!mNeedsSorting);
	if(!mNbUpdatedBoxes)
		return;

	if(mInput.mNeeded)
		DoCompleteBoxPruning(&pairManager, mInput, groups, mbpObjects);

	if(mInput.mBIPInput.mNeeded)
		DoBipartiteBoxPruning(&pairManager, mInput.mBIPInput, groups, mbpObjects);

	mNbUpdatedBoxes = 0;
}

///////////////////////////////////////////////////////////////////////////

static const PxU32 MBP_ObjectSize = sizeof(MBP_Object)/sizeof(PxU32);

// We have one of those for each BoxPruner within the MBP
struct BoxPrunerData : public Ps::UserAllocated
{
	MBP_AABB	mBox;		// Volume of space controlled by this BoxPruner
	BoxPruner*	mBP;		// Pointer to BoxPruner itself
	Ps::IntBool	mOverlap;	// True if overlaps other pruners
	void*		mUserData;	// Region identifier, reused to contain "first free ID"
};
static const PxU32 BoxPrunerDataSize = sizeof(BoxPrunerData)/sizeof(PxU32);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MBP::MBP(PxsAABBManager* manager) :
	mNbPairs			(0),
	mNbBoxPruners		(0),
	mFirstFreeIndex		(INVALID_ID),
	mFirstFreeIndexBP	(INVALID_ID),
	mManager			(manager)
{
	for(PxU32 i=0;i<MAX_NB_MBP+1;i++)
		mFirstFree[i] = INVALID_ID;
}

MBP::~MBP()
{
/*	for(PxU32 i=1;i<MAX_NB_MBP;i++)
	{
		if(mHandles[i].GetNbEntries())
		{
			const PxU32 SizeOfBundle = sizeof(BoxPrunerHandle)*i;
//			printf("Handles %d: %d\n", i, mHandles[i].GetNbEntries()*sizeof(PxU32)/SizeOfBundle);
		}
	}*/

	reset();
}

void MBP::freeBuffers()
{
	mRemoved.empty();
	mOutOfBoundsObjects.Empty();
}

bool MBP::preallocate(PxU32 nbBoxPruners, PxU32 nbObjects)
{
	if(nbBoxPruners)
		mBoxPruners.SetSize(nbBoxPruners*BoxPrunerDataSize);

	if(nbObjects)
	{
		mMBP_Objects.SetSize(nbObjects*MBP_ObjectSize);
#ifdef USE_FULLY_INSIDE_FLAG
		mFullyInsideBitmap.init(nbObjects);
#endif
	}

	mPairManager.reserveMemory(nbObjects*2);

	return true;
}

void MBP::addToOutOfBoundsArray(PxU32 id)
{
	PX_ASSERT(!mOutOfBoundsObjects.Contains(id));
	mOutOfBoundsObjects.Add(id);
}

static void setupOverlapFlags(PxU32 nbPruners, BoxPrunerData* PX_RESTRICT pruners)
{
	for(PxU32 i=0;i<nbPruners;i++)
		pruners[i].mOverlap = false;

	for(PxU32 i=0;i<nbPruners;i++)
	{
		if(!pruners[i].mBP)
			continue;

		for(PxU32 j=i+1;j<nbPruners;j++)
		{
			if(!pruners[j].mBP)
				continue;

			if(pruners[i].mBox.intersectNoTouch(pruners[j].mBox))	
			{
				pruners[i].mOverlap = true;
				pruners[j].mOverlap = true;
			}
		}
	}
}

// PT: automatically populate new region with overlapping objects.
// Brute-force version checking all existing objects, potentially optimized using "fully inside" flags.
void MBP::populateNewRegion(const MBP_AABB& box)
{
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();
	const PxU32 nbObjects = mMBP_Objects.GetNbEntries() / MBP_ObjectSize;
	MBP_Object* PX_RESTRICT objects = (MBP_Object*)mMBP_Objects.GetEntries();
#ifdef PX_DEBUG
	PxU32 nbObjectsFound = 0;
	PxU32 nbObjectsTested = 0;
#endif
	for(PxU32 j=0;j<nbObjects;j++)
	{
#ifdef USE_FULLY_INSIDE_FLAG
		// PT: if an object A is fully contained inside all the regions S it overlaps, we don't need to test it against the new region R.
		// The rationale is that even if R does overlap A, any new object B must touch S to overlap with A. So B would be added to S and
		// the (A,B) overlap would be detected in S, even if it's not detected in R.
//		if(currentObject.mFlags & MBP_FULLY_INSIDE)
		if(mFullyInsideBitmap.isSet(j))
			continue;
	#ifdef PX_DEBUG
		nbObjectsTested++;
	#endif
#endif

		MBP_Object& currentObject = objects[j];

		if(currentObject.mFlags & MBP_REMOVED)
			continue;	// PT: object is in the free list

		const PxU32 nbHandles = currentObject.mNbHandles;
		if(nbHandles)
		{
			BoxPrunerHandle* PX_RESTRICT handles = getHandles(currentObject, nbHandles);
			// PT: no need to test all pruners since they should contain the same info. Just retrieve bounds from the first one.
//			for(PxU32 i=0;i<nbHandles;i++)
			PxU32 i=0;
			{
				const BoxPrunerHandle& h = handles[i];
				const BoxPrunerData& currentBoxPruner = boxPruners[h.mInternalBPHandle];
	//			if(currentBoxPruner.mBP)
				PX_ASSERT(currentBoxPruner.mBP);

				MBP_AABB bounds;
				const PxU32 mbpHandle = currentBoxPruner.mBP->retrieveBounds(bounds, h.mHandle);

//				if(DecodeHandle_IsStatic(mbpHandle))
//					int stop=1;

				if(bounds.intersect(box))
				{
					updateObject(mbpHandle, bounds);
#ifdef PX_DEBUG
					nbObjectsFound++;
#endif
				}
			}
		}
		else
		{
			// PT: if the object is out-of-bounds, we're out-of-luck. We don't have the object bounds, so we need to retrieve them
			// from the AABB manager - and then re-encode them. This is not very elegant or efficient, but it should rarely happen
			// so this is good enough for now.
			const PxBounds3 decodedBounds = mManager->getBPBounds(currentObject.mUserID);

			MBP_AABB bounds;
			bounds.initFrom2(decodedBounds);

			if(bounds.intersect(box))
			{
				updateObject(currentObject.mHandlesIndex, bounds);
#ifdef PX_DEBUG
				nbObjectsFound++;
#endif
			}
		}
	}
#ifdef PX_DEBUG
	//printf("Populating new region with %d objects (tested %d/%d object)\n", nbObjectsFound, nbObjectsTested, nbObjects);
#endif
}

PxU32 MBP::addBoxPruner(const PxBroadPhaseRegion& region, bool populateRegion)
{
	PxU32 regionHandle;
	BoxPrunerData* PX_RESTRICT buffer;

	if(mFirstFreeIndexBP!=INVALID_ID)
	{
		regionHandle = mFirstFreeIndexBP;

		buffer = (BoxPrunerData*)mBoxPruners.GetEntries();
		buffer += regionHandle;

		mFirstFreeIndexBP = (PxU32)(size_t)(buffer->mUserData);
	}
	else
	{
		if(mNbBoxPruners>=MAX_NB_MBP)
		{
			Ps::getFoundation().error(PxErrorCode::eOUT_OF_MEMORY, __FILE__, __LINE__, "MBP::addBoxPruner: max number of regions reached.");
			return INVALID_ID;
		}

		regionHandle = mNbBoxPruners++;
		buffer = (BoxPrunerData*)mBoxPruners.Reserve(BoxPrunerDataSize);
	}

	BoxPruner* newBP = PX_NEW(BoxPruner);
	buffer->mBox.initFrom2(region.bounds);
	buffer->mBP			= newBP;
	buffer->mUserData	= region.userData;

	setupOverlapFlags(mNbBoxPruners, (BoxPrunerData*)mBoxPruners.GetEntries());

	// PT: automatically populate new region with overlapping objects
	if(populateRegion)
		populateNewRegion(buffer->mBox);

	return regionHandle;
}

// ### TODO: recycle box pruners, make sure objects are properly deleted/transferred, etc
// ### TODO: what happens if we delete a zone then immediately add it back? Do objects get deleted?
// ### TODO: in fact if we remove a zone but we keep the objects, what happens to their current overlaps? Are they kept or discarded?
bool MBP::removeBoxPruner(PxU32 handle)
{
	if(handle>=mNbBoxPruners)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "MBP::removeBoxPruner: invalid handle.");
		return false;
	}

	BoxPrunerData* PX_RESTRICT boxPruner = (BoxPrunerData*)mBoxPruners.GetEntries();
	boxPruner += handle;

	BoxPruner* bp = boxPruner->mBP;
	if(!bp)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "MBP::removeBoxPruner: invalid handle.");
		return false;
	}

	PxBounds3 empty;
	empty.setEmpty();
	boxPruner->mBox.initFrom2(empty);

	if(1)
	{
		// We are going to remove the pruner but it can still contain objects. We need to update
		// those objects so that their handles and out-of-bounds status are modified.
		//
		// Unfortunately there is no way to iterate over active objects in a pruner, so we need
		// to iterate over the max amount of objects. ### TODO: optimize this
		const PxU32 maxNbObjects = bp->mMaxNbObjects;
		MBPEntry* PX_RESTRICT objects = bp->mObjects;
		for(PxU32 j=0;j<maxNbObjects;j++)
		{
			// The handle is INVALID_ID for non-active entries
			if(objects[j].mMBPHandle!=INVALID_ID)
			{
//				printf("Object to update!\n");
				updateObject(objects[j].mMBPHandle, bp);
			}
		}
	}

	PX_DELETE(bp);
	boxPruner->mBP = NULL;
	boxPruner->mUserData = reinterpret_cast<void*>( static_cast<size_t>(mFirstFreeIndexBP));
	mFirstFreeIndexBP = handle;

	// A pruner has been removed so we need to update the overlap flags for all remaining pruners
	// ### TODO: optimize this
	setupOverlapFlags(mNbBoxPruners, (BoxPrunerData*)mBoxPruners.GetEntries());
	return true;
}

const BoxPruner* MBP::getBoxPruner(PxU32 i) const
{
	if(i>=mNbBoxPruners)
		return NULL;

	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();
	return boxPruners[i].mBP;
}

PX_FORCE_INLINE BoxPrunerHandle* MBP::getHandles(MBP_Object& currentObject, PxU32 nbHandles) const
{
	BoxPrunerHandle* handles;
	if(nbHandles==1)
		handles = &currentObject.mHandle;
	else
	{
		const PxU32 handlesIndex = currentObject.mHandlesIndex;
		const Container& c = mHandles[nbHandles];
		handles = (BoxPrunerHandle*)(c.GetEntries()+handlesIndex);
	}
	return handles;
}

void MBP::purgeHandles(MBP_Object* PX_RESTRICT object, PxU32 nbHandles)
{
	if(nbHandles>1)
	{
		const PxU32 handlesIndex = object->mHandlesIndex;
		Container& c = mHandles[nbHandles];
		PxU32* recycled = c.GetEntries() + handlesIndex;
		*recycled = mFirstFree[nbHandles];
		mFirstFree[nbHandles] = handlesIndex;
	}
}

void MBP::storeHandles(MBP_Object* PX_RESTRICT object, PxU32 nbHandles, const BoxPrunerHandle* PX_RESTRICT handles)
{
	if(nbHandles==1)
	{
		object->mHandle = handles[0];
	}
	else if(nbHandles)	// ### PhysX change
	{
		Container& c = mHandles[nbHandles];
		const PxU32 firstFree = mFirstFree[nbHandles];
		PxU32* handlesMemory;
		if(firstFree!=INVALID_ID)
		{
			object->mHandlesIndex = firstFree;
			handlesMemory = c.GetEntries() + firstFree;
			mFirstFree[nbHandles] = *handlesMemory;
		}
		else
		{
			const PxU32 handlesIndex = c.GetNbEntries();
			object->mHandlesIndex = handlesIndex;
			handlesMemory = c.Reserve(sizeof(BoxPrunerHandle)*nbHandles/sizeof(PxU32));
		}
		PxMemCopy(handlesMemory, handles, sizeof(BoxPrunerHandle)*nbHandles);
	}
}

PxU32 MBP::addObject(const MBP_AABB& box, PxcBpHandle userID, bool isStatic)
{
	PxU32 MBPObjectIndex;
	MBP_Object* objectMemory;
	PxU32 flipFlop;
	if(mFirstFreeIndex!=INVALID_ID)
	{
		MBPObjectIndex = mFirstFreeIndex;
		MBP_Object* objects = (MBP_Object*)mMBP_Objects.GetEntries();
		objectMemory = &objects[MBPObjectIndex];
		mFirstFreeIndex = objectMemory->mHandlesIndex;
		flipFlop = (PxU32)objectMemory->getFlipFlop();
	}
	else
	{
		MBPObjectIndex = mMBP_Objects.GetNbEntries() / MBP_ObjectSize;
		objectMemory = (MBP_Object*)mMBP_Objects.Reserve(MBP_ObjectSize);
		flipFlop = 0;
	}
	const PxU32 MBPObjectHandle = EncodeHandle(MBPObjectIndex, flipFlop, isStatic);

//	mMBP_Objects.Shrink();

	PxU32 nbHandles = 0;
#ifdef USE_FULLY_INSIDE_FLAG
	bool newObjectIsFullyInsideRegions = true;
#endif

	const PxU32 nb = mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();

	BoxPrunerHandle tmpHandles[MAX_NB_MBP+1];
	for(PxU32 i=0;i<nb;i++)
	{
#ifdef MBP_USE_NO_CMP_OVERLAP_3D
		if(Intersect3D(boxPruners[i].mBox, box))
#else
		if(boxPruners[i].mBox.intersect(box))
#endif
		{
#ifdef USE_FULLY_INSIDE_FLAG
			if(!box.isInside(boxPruners[i].mBox))
				newObjectIsFullyInsideRegions = false;
#endif
			BoxPrunerHandle& h = tmpHandles[nbHandles++];

			h.mHandle = boxPruners[i].mBP->addObject(box, MBPObjectHandle, isStatic);
			h.mInternalBPHandle = Ps::to16(i);
		}
	}
	storeHandles(objectMemory, nbHandles, tmpHandles);

	objectMemory->mNbHandles	= Ps::to16(nbHandles);
	PxU16 flags = 0;
	if(flipFlop)
		flags |= MBP_FLIP_FLOP;
#ifdef USE_FULLY_INSIDE_FLAG
	if(nbHandles && newObjectIsFullyInsideRegions)
//		flags |= MBP_FULLY_INSIDE;
		mFullyInsideBitmap.setBitChecked(MBPObjectIndex);
	else
		mFullyInsideBitmap.clearBitChecked(MBPObjectIndex);
#endif
//	PX_ASSERT(nbHandles);
	if(!nbHandles)
	{
		objectMemory->mHandlesIndex = MBPObjectHandle;
		addToOutOfBoundsArray(PxU32(userID));
	}

	if(!isStatic)
	{
		mUpdatedObjects.setBitChecked(MBPObjectIndex);
	}
//	objectMemory->mUpdated		= !isStatic;
	objectMemory->mFlags		= flags;
	objectMemory->mUserID		= userID;

	return MBPObjectHandle;
}

bool MBP::removeObject(PxU32 handle)
{
	const PxU32 MBPObjectIndex = DecodeHandle_Index(handle);

	MBP_Object* PX_RESTRICT objects = (MBP_Object*)mMBP_Objects.GetEntries();
	MBP_Object& currentObject = objects[MBPObjectIndex];
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();

	// Parse previously overlapping box pruners. If still overlapping, update object. Else remove from box pruner.
	const PxU32 nbHandles = currentObject.mNbHandles;
	if(nbHandles)	// ### PhysX change
	{
		BoxPrunerHandle* handles = getHandles(currentObject, nbHandles);
		for(PxU32 i=0;i<nbHandles;i++)
		{
			const BoxPrunerHandle& h = handles[i];
			const BoxPrunerData& currentBoxPruner = boxPruners[h.mInternalBPHandle];
//			if(currentBoxPruner.mBP)
			PX_ASSERT(currentBoxPruner.mBP);
			currentBoxPruner.mBP->removeObject(h.mHandle);
		}

		purgeHandles(&currentObject, nbHandles);
	}

	currentObject.mNbHandles	= 0;
	currentObject.mFlags		|= MBP_REMOVED;
	currentObject.mHandlesIndex	= mFirstFreeIndex;
//	if(!DecodeHandle_IsStatic(handle))
//	if(!currentObject.IsStatic())
	{
		mUpdatedObjects.setBitChecked(MBPObjectIndex);
	}
	mFirstFreeIndex				= MBPObjectIndex;

	mRemoved.setBitChecked(MBPObjectIndex);
	return true;
}

static PX_FORCE_INLINE bool stillIntersects(PxU32 handle, PxU32& _nb, PxU32* PX_RESTRICT currentOverlaps)
{
	const PxU32 nb = _nb;
	for(PxU32 i=0;i<nb;i++)
	{
		if(currentOverlaps[i]==handle)
		{
			_nb = nb-1;
			currentOverlaps[i] = currentOverlaps[nb-1];
			return true;
		}
	}
	return false;
}

bool MBP::updateObject(PxU32 handle, const MBP_AABB& box)
{
	const PxU32 MBPObjectIndex = DecodeHandle_Index(handle);
	const PxU32 isStatic = DecodeHandle_IsStatic(handle);

	const PxU32 nbBoxPruners = mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();

	MBP_Object* PX_RESTRICT objects = (MBP_Object*)mMBP_Objects.GetEntries();
	MBP_Object& currentObject = objects[MBPObjectIndex];

//	if(!isStatic)	// ### removed for PhysX integration (bugfix)
//	if(!currentObject.IsStatic())
	{
		mUpdatedObjects.setBitChecked(MBPObjectIndex);
	}

	const PxU32 nbHandles = currentObject.mNbHandles;
	if(nbHandles==1)
	{
		const BoxPrunerHandle& h = currentObject.mHandle;
		const BoxPrunerData& currentBoxPruner = boxPruners[h.mInternalBPHandle];
		if(!currentBoxPruner.mOverlap && box.isInside(currentBoxPruner.mBox))
		{
#ifdef USE_FULLY_INSIDE_FLAG
			// PT: it is possible that this flag is not set already when reaching this place:
			// - object touches 2 regions
			// - then in one frame:
			//    - object moves fully inside one region
			//    - the other region is removed
			// => nbHandles changes from 2 to 1 while MBP_FULLY_INSIDE is not set
//			currentObject.mFlags |= MBP_FULLY_INSIDE;
			mFullyInsideBitmap.setBitChecked(MBPObjectIndex);
#endif
			currentBoxPruner.mBP->updateObject(box, h.mHandle);
			return true;
		}
	}

	// Find box pruners overlapping object's new position
#ifdef USE_FULLY_INSIDE_FLAG
	bool objectIsFullyInsideRegions = true;
#endif
	PxU32 nbCurrentOverlaps = 0;
	PxU32 currentOverlaps[MAX_NB_MBP+1];
	for(PxU32 i=0;i<nbBoxPruners;i++)
	{
#ifdef MBP_USE_NO_CMP_OVERLAP_3D
		if(Intersect3D(boxPruners[i].mBox, box))
#else
		if(boxPruners[i].mBox.intersect(box))
#endif
		{
#ifdef USE_FULLY_INSIDE_FLAG
			if(!box.isInside(boxPruners[i].mBox))
				objectIsFullyInsideRegions = false;
#endif
			PX_ASSERT(nbCurrentOverlaps<MAX_NB_MBP);
			currentOverlaps[nbCurrentOverlaps++] = i;
		}
	}

	// New data for this frame
	PxU32 nbNewHandles = 0;
	BoxPrunerHandle newHandles[MAX_NB_MBP+1];

	// Parse previously overlapping box pruners. If still overlapping, update object. Else remove from box pruner.
	BoxPrunerHandle* handles = getHandles(currentObject, nbHandles);
	for(PxU32 i=0;i<nbHandles;i++)
	{
		const BoxPrunerHandle& h = handles[i];
		PX_ASSERT(h.mInternalBPHandle<nbBoxPruners);
		const BoxPrunerData& currentBoxPruner = boxPruners[h.mInternalBPHandle];
		// We need to update object even if it then gets removed, as the removal
		// doesn't actually report lost pairs - and we need this.
//		currentBoxPruner.mBP->UpdateObject(box, h.mHandle);
		if(stillIntersects(h.mInternalBPHandle, nbCurrentOverlaps, currentOverlaps))
		{
			currentBoxPruner.mBP->updateObject(box, h.mHandle);
			// Still collides => keep handle for this frame
			newHandles[nbNewHandles++] = h;
		}
		else
		{
			PX_ASSERT(!currentBoxPruner.mBox.intersect(box));
//			if(currentBoxPruner.mBP)
			PX_ASSERT(currentBoxPruner.mBP);
			currentBoxPruner.mBP->removeObject(h.mHandle);
		}
	}

	// Add to new box pruners if needed
	for(PxU32 i=0;i<nbCurrentOverlaps;i++)
	{
//		if(currentOverlaps[i]==INVALID_ID)
//			continue;
		const PxU32 boxPrunerIndex = currentOverlaps[i];
		const PxU32 BPHandle = boxPruners[boxPrunerIndex].mBP->addObject(box, handle, isStatic!=0);
		newHandles[nbNewHandles].mHandle = Ps::to16(BPHandle);
		newHandles[nbNewHandles].mInternalBPHandle = Ps::to16(boxPrunerIndex);
		nbNewHandles++;
	}

	if(nbHandles==nbNewHandles)
	{
		for(PxU32 i=0;i<nbNewHandles;i++)
			handles[i] = newHandles[i];
	}
	else
	{
		purgeHandles(&currentObject, nbHandles);
		storeHandles(&currentObject, nbNewHandles, newHandles);
	}

	currentObject.mNbHandles = Ps::to16(nbNewHandles);
//	PX_ASSERT(nbNewHandles);
	if(!nbNewHandles && nbHandles)
	{
		currentObject.mHandlesIndex = handle;
		addToOutOfBoundsArray(PxU32(currentObject.mUserID));
	}

//	for(PxU32 i=0;i<nbNewHandles;i++)
//		currentObject.mHandles[i] = newHandles[i];

#ifdef USE_FULLY_INSIDE_FLAG
	if(objectIsFullyInsideRegions)
//		currentObject.mFlags |= MBP_FULLY_INSIDE;
		mFullyInsideBitmap.setBitChecked(MBPObjectIndex);
	else
//		currentObject.mFlags &= ~MBP_FULLY_INSIDE;
		mFullyInsideBitmap.clearBitChecked(MBPObjectIndex);
#endif

	return true;
}

bool MBP::updateObject(PxU32 handle, BoxPruner* removedPruner)
{
	PX_ASSERT(removedPruner);

	const PxU32 MBPObjectIndex = DecodeHandle_Index(handle);

	const PxU32 nbBoxPruners = mNbBoxPruners;
	PX_UNUSED(nbBoxPruners);
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();

	MBP_Object* PX_RESTRICT objects = (MBP_Object*)mMBP_Objects.GetEntries();
	MBP_Object& currentObject = objects[MBPObjectIndex];

	// Mark the object as updated so that its pairs are considered for removal. If we don't do this an out-of-bounds object
	// resting on another non-out-of-bounds object still collides with that object and the memory associated with that pair
	// is not released. If we mark it as updated the pair is lost, and the out-of-bounds object falls through.
	//
	// However if we do this any pair involving the object will be marked as lost, even the ones involving other regions.
	// Typically the pair will then get lost one frame and get recreated the next frame.
//	mUpdatedObjects.setBitChecked(MBPObjectIndex);

	const PxU32 nbHandles = currentObject.mNbHandles;
	PX_ASSERT(nbHandles);

	// New handles
	PxU32 nbNewHandles = 0;
	BoxPrunerHandle newHandles[MAX_NB_MBP+1];

	// Parse previously overlapping box pruners. Keep all of them except removed one.
	BoxPrunerHandle* handles = getHandles(currentObject, nbHandles);
	for(PxU32 i=0;i<nbHandles;i++)
	{
		const BoxPrunerHandle& h = handles[i];
		PX_ASSERT(h.mInternalBPHandle<nbBoxPruners);
		if(boxPruners[h.mInternalBPHandle].mBP!=removedPruner)
			newHandles[nbNewHandles++] = h;
	}
#ifdef USE_FULLY_INSIDE_FLAG
	// PT: in theory we should update the inside flag here but we don't do that for perf reasons.
	// - If the flag is set, it means the object was fully inside all its regions. Removing one of them does not invalidate the flag.
	// - If the flag is not set, removing one region might allow us to set the flag now. However not doing so simply makes the
	//   populateNewRegion() function run a bit slower, it does not produce wrong results. This is only until concerned objects are
	//   updated again anyway, so we live with this.
#endif

	PX_ASSERT(nbNewHandles==nbHandles-1);
	purgeHandles(&currentObject, nbHandles);
	storeHandles(&currentObject, nbNewHandles, newHandles);

	currentObject.mNbHandles = Ps::to16(nbNewHandles);
//	PX_ASSERT(nbNewHandles);
	if(!nbNewHandles)
	{
		currentObject.mHandlesIndex = handle;
		addToOutOfBoundsArray(PxU32(currentObject.mUserID));
	}
	return true;
}

bool MBP_PairManager::removeMarkedPairs(const MBP_Object* objects, PxsBroadPhaseMBP* mbp, const BitArray& updated, const BitArray& removed)
{
	PxU32 i=0;
	PxU32 nbActivePairs = mNbActivePairs;
	while(i<nbActivePairs)
	{
		MBP_Pair& p = mActivePairs[i];

		if(p.isNew)
		{
			const PxU32 id0 = p.id0;
			const PxU32 id1 = p.id1;
			PX_ASSERT(id0!=INVALID_ID);
			PX_ASSERT(id1!=INVALID_ID);
			const PxU32 index0 = DecodeHandle_Index(id0);
			const PxU32 index1 = DecodeHandle_Index(id1);

			const PxcBpHandle object0 = objects[index0].mUserID;
			const PxcBpHandle object1 = objects[index1].mUserID;
			if(mbp->mGroups[object0]!=mbp->mGroups[object1])
				mbp->mCreated.pushBack(PxcBroadPhasePair(object0, object1));

			// New pair
			p.isNew = false;
			p.isUpdated = false;
			i++;
		}
		else if(p.isUpdated)
		{
			// Persistent pair
			p.isUpdated = false;
			i++;
		}
		else
		{
			// Lost pair
			const PxU32 id0 = p.id0;
			const PxU32 id1 = p.id1;
			PX_ASSERT(id0!=INVALID_ID);
			PX_ASSERT(id1!=INVALID_ID);
			const PxU32 index0 = DecodeHandle_Index(id0);
			const PxU32 index1 = DecodeHandle_Index(id1);
			if(updated.isSetChecked(index0) || updated.isSetChecked(index1))
			{
				//Don't report pairs that involve an aabb that has been removed
	//			const PxU32 last = removed.findLast();	// PT: TODO: not needed for each pair!!
	//			const physx::Ps::IntBool test0 = (id0 <= last) ? removed.test(id0) : false;
	//			const physx::Ps::IntBool test1 = (id1 <= last) ? removed.test(id1) : false;
	//			if(0==removed.getWordCount() || (!test0 && !test1))
				if(!removed.isSetChecked(index0) && !removed.isSetChecked(index1))
				{
					const PxcBpHandle object0 = objects[index0].mUserID;
					const PxcBpHandle object1 = objects[index1].mUserID;
					if(mbp->mGroups[object0]!=mbp->mGroups[object1])
						mbp->mDeleted.pushBack(PxcBroadPhasePair(object0, object1));
				}

			const PxU32 hashValue = hash(id0, id1) & mMask;
			removePair(id0, id1, hashValue, i);
			nbActivePairs--;
			}
			else i++;
		}
	}

	shrinkMemory();
	return true;
}

#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
void MBP::prepareOverlaps()
{
	PxU32 nb = mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData* PX_RESTRICT)mBoxPruners.GetEntries();
	for(PxU32 i=0;i<nb;i++)
		if(boxPruners[i].mBP)
			boxPruners[i].mBP->prepareOverlaps();
}
#endif

void MBP::prepareOverlapsMT()
{
	const PxU32 nb = mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();
	for(PxU32 i=0;i<nb;i++)
	{
		if(boxPruners[i].mBP)
			boxPruners[i].mBP->prepareOverlapsMT();
	}
}

#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
void MBP::findOverlaps()
{
	/*static*/ MBPOS_TmpBuffers TmpBuffers;	// ####

	PxU32 nb = mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData* PX_RESTRICT)mBoxPruners.GetEntries();
	for(PxU32 i=0;i<nb;i++)
	{
		if(boxPruners[i].mBP)
			boxPruners[i].mBP->findOverlaps(mPairManager, TmpBuffers);
	}
}
#endif

void MBP::findOverlapsMT(const PxcBpHandle* PX_RESTRICT groups)
{
	PxU32 nb = mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mBoxPruners.GetEntries();
	const MBP_Object* objects = (const MBP_Object*)mMBP_Objects.GetEntries();
	for(PxU32 i=0;i<nb;i++)
	{
		if(boxPruners[i].mBP)
			boxPruners[i].mBP->findOverlapsMT(mPairManager, groups, objects);
	}
}

PxU32 MBP::finalize(PxsBroadPhaseMBP* mbp)
{
	const MBP_Object* objects = (const MBP_Object*)mMBP_Objects.GetEntries();
	mPairManager.removeMarkedPairs(objects, mbp, mUpdatedObjects, mRemoved);

	mUpdatedObjects.clearAll();

	return mPairManager.mNbActivePairs;
}

void MBP::reset()
{
	PxU32 nb = mNbBoxPruners;
	BoxPrunerData* PX_RESTRICT boxPruners = (BoxPrunerData*)mBoxPruners.GetEntries();
	while(nb--)
	{
//		printf("%d objects in box pruner\n", boxPruners->mBP->mNbObjects);
		DELETESINGLE(boxPruners->mBP);
		boxPruners++;
	}

	mNbPairs			= 0;
	mNbBoxPruners		= 0;
	mFirstFreeIndex		= INVALID_ID;
	mFirstFreeIndexBP	= INVALID_ID;
	for(PxU32 i=0;i<MAX_NB_MBP+1;i++)
	{
		mHandles[i].Empty();
		mFirstFree[i] = INVALID_ID;
	}

	mBoxPruners.Empty();
	mMBP_Objects.Empty();
	mPairManager.purge();
	mUpdatedObjects.empty();
	mRemoved.empty();
	mOutOfBoundsObjects.Empty();
#ifdef USE_FULLY_INSIDE_FLAG
	mFullyInsideBitmap.empty();
#endif
}

void MBP::shiftOrigin(const PxVec3& shift)
{
	const PxU32 size = mNbBoxPruners;
	BoxPrunerData* PX_RESTRICT boxPruners = (BoxPrunerData*)mBoxPruners.GetEntries();

	//
	// regions
	//
	for(PxU32 i=0; i < size; i++)
	{
		if(boxPruners[i].mBP)
		{
#ifdef MBP_SIMD_OVERLAP
			SIMD_AABB& box = boxPruners[i].mBox;
#else
			IAABB& box = boxPruners[i].mBox;
#endif
			PxBounds3 bounds;
			box.decode(bounds);

			bounds.minimum -= shift;
			bounds.maximum -= shift;

			box.initFrom2(bounds);
		}
	}

	//
	// object bounds
	//
	const PxU32 nbObjects = mMBP_Objects.GetNbEntries() / MBP_ObjectSize;
	MBP_Object* objects = (MBP_Object*)mMBP_Objects.GetEntries();
	
	for(PxU32 i=0; i < nbObjects; i++)
	{
		MBP_Object& obj = objects[i];

		const PxU32 nbHandles = obj.mNbHandles;
		if(nbHandles)
		{
			MBP_AABB bounds;
			const PxBounds3 decodedBounds = mManager->getBPBounds(obj.mUserID);
			bounds.initFrom2(decodedBounds);

			BoxPrunerHandle* PX_RESTRICT handles = getHandles(obj, nbHandles);
			for(PxU32 j=0; j < nbHandles; j++)
			{
				const BoxPrunerHandle& h = handles[j];
				const BoxPrunerData& currentBoxPruner = boxPruners[h.mInternalBPHandle];
				PX_ASSERT(currentBoxPruner.mBP);
				currentBoxPruner.mBP->setBounds(h.mHandle, bounds);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

// Below is the PhysX wrapper = link between AABBManager and MBP

#define DEFAULT_CREATED_DELETED_PAIRS_CAPACITY 1024

#include "PxSceneDesc.h"

PxsBroadPhaseMBP::PxsBroadPhaseMBP(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler, const PxSceneDesc& desc, PxsAABBManager* manager) :
	mMBPUpdateWorkTask(scratchAllocator, eventProfiler),
	mMBPPostUpdateWorkTask(scratchAllocator, eventProfiler),
	mMapping	(NULL),
	mCapacity	(0),
	mGroups		(NULL)
{
	mMBP = PX_NEW(MBP)(manager);

	const PxU32 nbRegions = desc.limits.maxNbRegions;
	const PxU32 nbObjects = desc.limits.maxNbStaticShapes + desc.limits.maxNbDynamicShapes;
	mMBP->preallocate(nbRegions, nbObjects);

	mCreated.reserve(DEFAULT_CREATED_DELETED_PAIRS_CAPACITY);
	mDeleted.reserve(DEFAULT_CREATED_DELETED_PAIRS_CAPACITY);
}

PxsBroadPhaseMBP::~PxsBroadPhaseMBP()
{
	DELETESINGLE(mMBP);
	PX_FREE(mMapping);
}

bool PxsBroadPhaseMBP::getCaps(PxBroadPhaseCaps& caps) const
{
	caps.maxNbRegions			= 256;
	caps.maxNbObjects			= 0;
	caps.needsPredefinedBounds	= true;
	return true;
}

PxU32 PxsBroadPhaseMBP::getNbRegions() const
{
	// PT: we need to count active regions here, as we only keep track of the total number of
	// allocated regions internally - and some of which might have been removed.
	const PxU32 size = mMBP->mNbBoxPruners;
/*	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData* PX_RESTRICT)mMBP->mBoxPruners.GetEntries();
	PxU32 nbActiveRegions = 0;
	for(PxU32 i=0;i<size;i++)
	{
		if(boxPruners[i].mBP)
			nbActiveRegions++;
	}
	return nbActiveRegions;*/
	return size;
}

PxU32 PxsBroadPhaseMBP::getRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	const PxU32 size = mMBP->mNbBoxPruners;
	const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mMBP->mBoxPruners.GetEntries();
	boxPruners += startIndex;

	const PxU32 writeCount = PxMin(size, bufferSize);
	for(PxU32 i=0;i<writeCount;i++)
	{
#ifdef MBP_SIMD_OVERLAP
		const SIMD_AABB& box = boxPruners[i].mBox;
#else
		const IAABB& box = boxPruners[i].mBox;
#endif
		box.decode(userBuffer[i].region.bounds);
		if(boxPruners[i].mBP)
		{
			PX_ASSERT(userBuffer[i].region.bounds.isValid());
			userBuffer[i].region.userData	= boxPruners[i].mUserData;
			userBuffer[i].active			= true;
			userBuffer[i].overlap			= boxPruners[i].mOverlap!=0;
			userBuffer[i].nbStaticObjects	= boxPruners[i].mBP->mNbStaticBoxes;
			userBuffer[i].nbDynamicObjects	= boxPruners[i].mBP->mNbDynamicBoxes;
		}
		else
		{
			userBuffer[i].region.bounds.setEmpty();
			userBuffer[i].region.userData	= NULL;
			userBuffer[i].active			= false;
			userBuffer[i].overlap			= false;
			userBuffer[i].nbStaticObjects	= 0;
			userBuffer[i].nbDynamicObjects	= 0;
		}
	}
	return writeCount;
}

PxU32 PxsBroadPhaseMBP::addRegion(const PxBroadPhaseRegion& region, bool populateRegion)
{
	return mMBP->addBoxPruner(region, populateRegion);
}

bool PxsBroadPhaseMBP::removeRegion(PxU32 handle)
{
	return mMBP->removeBoxPruner(handle);
}

void PxsBroadPhaseMBP::destroy()
{
	delete this;
}

void PxsBroadPhaseMBP::update(const PxU32 numCpuTasks, const PxU32 numSpus, const PxcBroadPhaseUpdateData& updateData, PxBaseTask* continuation)
{
	setUpdateData(updateData);

	mMBPPostUpdateWorkTask.setBroadphase(this);
	mMBPUpdateWorkTask.setBroadphase(this);

	mMBPPostUpdateWorkTask.setNumCpuTasks(numCpuTasks);
	mMBPUpdateWorkTask.setNumCpuTasks(numCpuTasks);

	mMBPPostUpdateWorkTask.setNumSpus(numSpus);
	mMBPUpdateWorkTask.setNumSpus(numSpus);

	mMBPPostUpdateWorkTask.setContinuation(continuation);
	mMBPUpdateWorkTask.setContinuation(&mMBPPostUpdateWorkTask);

	mMBPPostUpdateWorkTask.removeReference();
	mMBPUpdateWorkTask.removeReference();
}

void PxsBroadPhaseMBP::setUpdateData(const PxcBroadPhaseUpdateData& updateData)
{
	const PxU32 newCapacity = updateData.getCapacity();
	if(newCapacity>mCapacity)
	{
		PxU32* newMapping = (PxU32*)PX_ALLOC(sizeof(PxU32)*newCapacity, PX_DEBUG_EXP("MBP"));
		if(mCapacity)
			PxMemCopy(newMapping, mMapping, mCapacity*sizeof(PxU32));
		for(PxU32 i=mCapacity;i<newCapacity;i++)
			newMapping[i] = PX_INVALID_U32;
		PX_FREE(mMapping);
		mMapping = newMapping;
		mCapacity = newCapacity;
	}

#ifdef PX_CHECKED
	if(!PxcBroadPhaseUpdateData::isValid(updateData, *this))
	{
		PX_CHECK_MSG(false, "Illegal PxcBroadPhaseUpdateData \n");
		return;
	}
#endif

	const IntegerAABB* PX_RESTRICT boundsXYZ = updateData.getAABBs();
	const PxcBpHandle* PX_RESTRICT groups = updateData.getGroups();	// ### why are those 'handles'?
	mGroups = groups;

	// ### TODO: handle groups inside MBP
	// ### TODO: get rid of AABB conversions

	const PxcBpHandle* removed = updateData.getRemovedHandles();
	if(removed)
	{
		PxU32 nbToGo = updateData.getNumRemovedHandles();
		while(nbToGo--)
		{
			const PxcBpHandle index = *removed++;
			PX_ASSERT(index<mCapacity);

			const bool status = mMBP->removeObject(mMapping[index]);
			PX_ASSERT(status);
			PX_UNUSED(status);

			mMapping[index] = PX_INVALID_U32;
		}
	}

	const PxcBpHandle* created = updateData.getCreatedHandles();
	if(created)
	{
		PxU32 nbToGo = updateData.getNumCreatedHandles();
		while(nbToGo--)
		{
			const PxcBpHandle index = *created++;
			PX_ASSERT(index<mCapacity);

			const PxU32 group = groups[index];
			const bool isStatic = group==BP_GROUP_STATICS;

			MBP_AABB aabb;
			aabb.mMinX	= (boundsXYZ[index].mMinMax[IntegerAABB::MIN_X])>>1;
			aabb.mMinY	= (boundsXYZ[index].mMinMax[IntegerAABB::MIN_Y])>>1;
			aabb.mMinZ	= (boundsXYZ[index].mMinMax[IntegerAABB::MIN_Z])>>1;
			aabb.mMaxX	= (boundsXYZ[index].mMinMax[IntegerAABB::MAX_X])>>1;
			aabb.mMaxY	= (boundsXYZ[index].mMinMax[IntegerAABB::MAX_Y])>>1;
			aabb.mMaxZ	= (boundsXYZ[index].mMinMax[IntegerAABB::MAX_Z])>>1;

/*			aabb.mMinX	&= ~1;
			aabb.mMinY	&= ~1;
			aabb.mMinZ	&= ~1;
			aabb.mMaxX	|= 1;
			aabb.mMaxY	|= 1;
			aabb.mMaxZ	|= 1;*/

#ifdef PX_DEBUG
			PxBounds3 decodedBox;
			PxU32* bin = (PxU32*)&decodedBox.minimum.x;
			bin[0] = PxsBpDecodeFloat(boundsXYZ[index].mMinMax[IntegerAABB::MIN_X]);
			bin[1] = PxsBpDecodeFloat(boundsXYZ[index].mMinMax[IntegerAABB::MIN_Y]);
			bin[2] = PxsBpDecodeFloat(boundsXYZ[index].mMinMax[IntegerAABB::MIN_Z]);
			bin[3] = PxsBpDecodeFloat(boundsXYZ[index].mMinMax[IntegerAABB::MAX_X]);
			bin[4] = PxsBpDecodeFloat(boundsXYZ[index].mMinMax[IntegerAABB::MAX_Y]);
			bin[5] = PxsBpDecodeFloat(boundsXYZ[index].mMinMax[IntegerAABB::MAX_Z]);

			MBP_AABB PrunerBox;
			PrunerBox.initFrom2(decodedBox);
			PX_ASSERT(PrunerBox.mMinX==aabb.mMinX);
			PX_ASSERT(PrunerBox.mMinY==aabb.mMinY);
			PX_ASSERT(PrunerBox.mMinZ==aabb.mMinZ);
			PX_ASSERT(PrunerBox.mMaxX==aabb.mMaxX);
			PX_ASSERT(PrunerBox.mMaxY==aabb.mMaxY);
			PX_ASSERT(PrunerBox.mMaxZ==aabb.mMaxZ);
#endif

			const PxU32 mbpHandle = mMBP->addObject(aabb, index, isStatic);
			mMapping[index] = mbpHandle;
		}
	}

	const PxcBpHandle* updated = updateData.getUpdatedHandles();
	if(updated)
	{
		PxU32 nbToGo = updateData.getNumUpdatedHandles();
		while(nbToGo--)
		{
			const PxcBpHandle index = *updated++;
			PX_ASSERT(index<mCapacity);

			MBP_AABB aabb;
			aabb.mMinX	= (boundsXYZ[index].mMinMax[IntegerAABB::MIN_X])>>1;
			aabb.mMinY	= (boundsXYZ[index].mMinMax[IntegerAABB::MIN_Y])>>1;
			aabb.mMinZ	= (boundsXYZ[index].mMinMax[IntegerAABB::MIN_Z])>>1;
			aabb.mMaxX	= (boundsXYZ[index].mMinMax[IntegerAABB::MAX_X])>>1;
			aabb.mMaxY	= (boundsXYZ[index].mMinMax[IntegerAABB::MAX_Y])>>1;
			aabb.mMaxZ	= (boundsXYZ[index].mMinMax[IntegerAABB::MAX_Z])>>1;

/*			aabb.mMinX	&= ~1;
			aabb.mMinY	&= ~1;
			aabb.mMinZ	&= ~1;
			aabb.mMaxX	|= 1;
			aabb.mMaxY	|= 1;
			aabb.mMaxZ	|= 1;*/

/*			PxBounds3 aabb;
			PxU32* bin = (PxU32*)&aabb.minimum.x;
			bin[0] = PxsBpDecodeFloat(boundsX[index*2]);
			bin[1] = PxsBpDecodeFloat(boundsY[index*2]);
			bin[2] = PxsBpDecodeFloat(boundsZ[index*2]);
			bin[3] = PxsBpDecodeFloat(boundsX[index*2+1]);
			bin[4] = PxsBpDecodeFloat(boundsY[index*2+1]);
			bin[5] = PxsBpDecodeFloat(boundsZ[index*2+1]);*/

			const bool status = mMBP->updateObject(mMapping[index], aabb);
			PX_ASSERT(status);
			PX_UNUSED(status);
		}
	}

	PX_ASSERT(!mCreated.size());
	PX_ASSERT(!mDeleted.size());

#ifdef USE_SINGLE_THREADED_REFERENCE_CODE
	mMBP->prepareOverlaps();
	mMBP->findOverlaps();
#else
	mMBP->prepareOverlapsMT();
#endif
}

void PxsBroadPhaseMBP::updatePPU(PxBaseTask* /*continuation*/)
{
#ifndef USE_SINGLE_THREADED_REFERENCE_CODE
	#ifdef CHECK_NB_OVERLAPS
	gNbOverlaps = 0;
	#endif
	mMBP->findOverlapsMT(mGroups);
	#ifdef CHECK_NB_OVERLAPS
	printf("PPU: %d overlaps\n", gNbOverlaps);
	#endif
#endif
}

void PxsBroadPhaseMBP::postUpdatePPU(PxBaseTask* /*continuation*/)
{
#ifndef USE_SINGLE_THREADED_REFERENCE_CODE
	{
		PxU32 Nb = mMBP->mNbBoxPruners;
		const BoxPrunerData* PX_RESTRICT boxPruners = (const BoxPrunerData*)mMBP->mBoxPruners.GetEntries();
		for(PxU32 i=0;i<Nb;i++)
		{
			if(boxPruners[i].mBP)
				boxPruners[i].mBP->mNbUpdatedBoxes = 0;
		}
	}

	mMBP->finalize(this);
#endif
}

PxU32 PxsBroadPhaseMBP::getNumCreatedPairs() const
{
	return mCreated.size();
}

PxcBroadPhasePair* PxsBroadPhaseMBP::getCreatedPairs()
{
	return mCreated.begin();
}

PxU32 PxsBroadPhaseMBP::getNumDeletedPairs() const
{
	return mDeleted.size();
}

PxcBroadPhasePair* PxsBroadPhaseMBP::getDeletedPairs()
{
	return mDeleted.begin();
}

PxU32 PxsBroadPhaseMBP::getNumOutOfBoundsObjects() const
{
	return mMBP->mOutOfBoundsObjects.GetNbEntries();
}

const PxU32* PxsBroadPhaseMBP::getOutOfBoundsObjects() const
{
	return mMBP->mOutOfBoundsObjects.GetEntries();
}

static void freeBuffer(Ps::Array<PxcBroadPhasePair>& buffer)
{
	const PxU32 size = buffer.size();
	if(size>DEFAULT_CREATED_DELETED_PAIRS_CAPACITY)
	{
		buffer.reset();
		buffer.reserve(DEFAULT_CREATED_DELETED_PAIRS_CAPACITY);
	}
	else
	{
		buffer.clear();
	}
}

void PxsBroadPhaseMBP::freeBuffers()
{
	mMBP->freeBuffers();
	freeBuffer(mCreated);
	freeBuffer(mDeleted);
}

#ifdef PX_CHECKED
bool PxsBroadPhaseMBP::isValid(const PxcBroadPhaseUpdateData& updateData) const
{
	const PxcBpHandle* created = updateData.getCreatedHandles();
	if(created)
	{
		PxU32 nbToGo = updateData.getNumCreatedHandles();
		while(nbToGo--)
		{
			const PxcBpHandle index = *created++;
			PX_ASSERT(index<mCapacity);

			PxU32 nbObjects = mMBP->mMBP_Objects.GetNbEntries() / MBP_ObjectSize;
			const MBP_Object* PX_RESTRICT objects = (const MBP_Object*)mMBP->mMBP_Objects.GetEntries();
			while(nbObjects--)
			{
				if(!(objects->mFlags & MBP_REMOVED))
				{
					if(objects->mUserID==index)
						return false;	// This object has been added already
				}
				objects++;
			}
		}
	}

	const PxcBpHandle* updated = updateData.getUpdatedHandles();
	if(updated)
	{
		PxU32 nbToGo = updateData.getNumUpdatedHandles();
		while(nbToGo--)
		{
			const PxcBpHandle index = *updated++;
			PX_ASSERT(index<mCapacity);

			if(mMapping[index]==PX_INVALID_U32)
				return false;	// This object has been removed already, or never been added
		}
	}

	const PxcBpHandle* removed = updateData.getRemovedHandles();
	if(removed)
	{
		PxU32 nbToGo = updateData.getNumRemovedHandles();
		while(nbToGo--)
		{
			const PxcBpHandle index = *removed++;
			PX_ASSERT(index<mCapacity);

			if(mMapping[index]==PX_INVALID_U32)
				return false;	// This object has been removed already, or never been added
		}
	}
	return true;
}
#endif


void PxsBroadPhaseMBP::shiftOrigin(const PxVec3& shift)
{
	mMBP->shiftOrigin(shift);
}

#ifdef PX_PS3
#include "PxsMBPTasks.h"
#include "PS3Support.h"
#include "PxcScratchAllocator.h"

MBPTaskSPU::MBPTaskSPU(PxU32 numSpusToUse) :
	PxSpuTask			(gPS3GetElfImage(SPU_ELF_MBP_TASK), gPS3GetElfSize(SPU_ELF_MBP_TASK), numSpusToUse),
	mMBP			(NULL),
	mNbBoxPruners	(0),
#ifdef MBP_USE_SCRATCHPAD
	mAllocator		(NULL),
#endif
	mRestartCount	(0)
{
	for(PxU32 i=0;i<256;i++)
		mBoxPruners[i] = NULL;

#ifdef MBP_USE_SCRATCHPAD
	for(PxU32 i=0;i<6;i++)
	{
		mScratchAddresses[i] = NULL;
		mScratchAddressesAligned[i] = NULL;
	}
#endif
#ifdef MBP_SINGLE_SPU
	numSpusToUse = 1;
#endif
	setNbSPUs(numSpusToUse);
}

void MBPTaskSPU::setNbSPUs(PxU32 numSpusToUse)
{
#ifdef MBP_SINGLE_SPU
	numSpusToUse = 1;
	mNbDone = 0;
#else
	for(PxU32 i=0;i<6;i++)
		mNbDone[i] = 0;
#endif
	setSpuCount(numSpusToUse);
}

//#define CHECK_PPU_TIME

#ifdef CHECK_PPU_TIME
	#include "PsTime.h"
#endif

#ifdef MBP_SINGLE_SPU
void MBPTaskSPU::initPruners(PxU32 nb, PxU32 offset)
{
	PX_ASSERT(nb<=256);
	mCellMBPSPUInput.mNbBoxPruners = nb;
	for(PxU32 i=0;i<nb;i++)
	{
		mCellMBPSPUInput.mInput[i] = &mBoxPruners[i+offset]->mInput;
//		printf("PPU: %d | %d\n", mCellMBPSPUInput.mInput[i]->mNeeded, PxU32(mCellMBPSPUInput.mInput[i]->mUpdatedDynamicBoxes));
	}
}
#else
void MBPTaskSPU::initPruners(PxU32 spuIndex, PxU32 nb, PxU32 offset)
{
	PX_ASSERT(nb<=256);
	mCellMBPSPUInput[spuIndex].mNbBoxPruners = nb;
	for(PxU32 i=0;i<nb;i++)
	{
		mCellMBPSPUInput[spuIndex].mInput[i] = &mBoxPruners[i+offset]->mInput;
	}
}
#endif

void MBPTaskSPU::release()
{
	PX_ASSERT(mMBP);

#ifdef CHECK_PPU_TIME
	PxU64 startTime = shdfnd::Time::getCurrentCounterValue();
#endif

#ifdef CHECK_NB_OVERLAPS
	PxU32 total = 0;
#else
	MBP_PairManager& pairManager = mMBP->mPairManager;
#endif

	bool needToRestart = false;
#ifdef MBP_SINGLE_SPU
	PxU32 firstFailing = 0;
#else
	PxU32 firstFailing[6];
	for(PxU32 i=0;i<6;i++)
	{
		firstFailing[i] = 0;
	}
#endif

#ifndef MBP_SINGLE_SPU
	const PxU32 nbSpus = getSpuCount();
	for(PxU32 spuIndex=0;spuIndex<nbSpus;spuIndex++)
#endif
	{
#ifndef CHECK_NB_OVERLAPS
	#ifdef MBP_SINGLE_SPU
		const MBP_Overlap* PX_RESTRICT overlaps = mOverlaps;
		const PxU32 nb = mCellMBPSPUInput.mNbBoxPruners;
	#else
		#ifdef MBP_USE_SCRATCHPAD
		const MBP_Overlap* PX_RESTRICT overlaps = reinterpret_cast<const MBP_Overlap*>(mScratchAddressesAligned[spuIndex]);
		#else
		const MBP_Overlap* PX_RESTRICT overlaps = mOverlaps + spuIndex*MBP_MAX_NB_OVERLAPS;
		#endif
		const PxU32 nb = mCellMBPSPUInput[spuIndex].mNbBoxPruners;
	#endif
#endif

/*#ifdef MBP_SINGLE_SPU
		const PxU16 low = mCellMBPSPUOutput.mNbOverlaps[nb].mNbOverlaps_UpNonUp;
		const PxU16 high = mCellMBPSPUOutput.mNbOverlaps[nb].mNbOverlaps_UpUp;
#else
		const PxU16 low = mCellMBPSPUOutput[spuIndex].mNbOverlaps[nb].mNbOverlaps_UpNonUp;
		const PxU16 high = mCellMBPSPUOutput[spuIndex].mNbOverlaps[nb].mNbOverlaps_UpUp;
#endif
		const PxU32 elapsedTime = PxU32(low)|(PxU32(high)<<16);
		printf("SPU%d: elapsed time: %d\n", spuIndex, elapsedTime);
*/
		for(PxU32 i=0;i<nb;i++)
		{
#ifdef MBP_SINGLE_SPU
			PxU32 nbToGo_UpNonUp = mCellMBPSPUOutput.mNbOverlaps[i].mNbOverlaps_UpNonUp;
			PxU32 nbToGo_UpUp = mCellMBPSPUOutput.mNbOverlaps[i].mNbOverlaps_UpUp;
			PxU32 nbToGo_StDn = mCellMBPSPUOutput.mNbOverlaps[i].mNbOverlaps_StDn;
#else
			PxU32 nbToGo_UpNonUp = mCellMBPSPUOutput[spuIndex].mNbOverlaps[i].mNbOverlaps_UpNonUp;
			PxU32 nbToGo_UpUp = mCellMBPSPUOutput[spuIndex].mNbOverlaps[i].mNbOverlaps_UpUp;
			PxU32 nbToGo_StDn = mCellMBPSPUOutput[spuIndex].mNbOverlaps[i].mNbOverlaps_StDn;
#endif
			if(nbToGo_UpUp==0xffff || nbToGo_UpNonUp==0xffff)
			{
				needToRestart = true;
#ifdef MBP_SINGLE_SPU
				firstFailing = i;
#else
				firstFailing[spuIndex] = i;
#endif
				break;
			}
#ifdef CHECK_NB_OVERLAPS
			total += nbToGo_UpNonUp;
			total += nbToGo_UpUp;
#else
	#ifdef MBP_SINGLE_SPU
			const BoxPruning_Input* PX_RESTRICT input = mCellMBPSPUInput.mInput[i];
	#else
			const BoxPruning_Input* PX_RESTRICT input = mCellMBPSPUInput[spuIndex].mInput[i];
	#endif
			const MBPEntry* PX_RESTRICT objects = input->mObjects;
			const MBP_Index* PX_RESTRICT inToOut_Dynamic = input->mInToOut_Dynamic;
			const MBP_Index* PX_RESTRICT inToOut_Dynamic_Sleeping = input->mInToOut_Dynamic_Sleeping;
			const MBP_Index* PX_RESTRICT inToOut_Static = input->mBIPInput.mInToOut_Static;

//			printf("nbToGo: %d %d\n", nbToGo_UpNonUp, nbToGo_UpUp);
			while(nbToGo_UpNonUp--)
			{
//				printf("0: %d %d\n", overlaps->mIndex0, overlaps->mIndex1);
				const PxU32 ObjectIndex0 = inToOut_Dynamic[overlaps->mIndex0];
				const PxU32 ObjectIndex1 = inToOut_Dynamic_Sleeping[overlaps->mIndex1];
				const PxU32 id0 = objects[ObjectIndex0].mMBPHandle;
				const PxU32 id1 = objects[ObjectIndex1].mMBPHandle;
				pairManager.addPair(id0, id1);
//				printf("%d %d\n", id0, id1);
				overlaps++;
			}

			while(nbToGo_UpUp--)
			{
//				printf("1: %d %d\n", overlaps->mIndex0, overlaps->mIndex1);
				PX_ASSERT(overlaps->mIndex0!=overlaps->mIndex1);
				const PxU32 ObjectIndex0 = inToOut_Dynamic[overlaps->mIndex0];
				const PxU32 ObjectIndex1 = inToOut_Dynamic[overlaps->mIndex1];
				const PxU32 id0 = objects[ObjectIndex0].mMBPHandle;
				const PxU32 id1 = objects[ObjectIndex1].mMBPHandle;
				pairManager.addPair(id0, id1);
				overlaps++;
			}

			while(nbToGo_StDn--)
			{
//				printf("2: %d %d\n", overlaps->mIndex0, overlaps->mIndex1);
				const PxU32 ObjectIndex0 = inToOut_Dynamic[overlaps->mIndex0];
				const PxU32 ObjectIndex1 = inToOut_Static[overlaps->mIndex1];
//				printf("3: %d %d\n", ObjectIndex0, ObjectIndex1);
				const PxU32 id0 = objects[ObjectIndex0].mMBPHandle;
				const PxU32 id1 = objects[ObjectIndex1].mMBPHandle;
				pairManager.addPair(id0, id1);
				overlaps++;
			}
#endif
#ifdef MBP_SINGLE_SPU
			mNbDone++;
#else
			mNbDone[spuIndex]++;
#endif
		}
	}

#ifdef CHECK_PPU_TIME
	PxU64 endTime = shdfnd::Time::getCurrentCounterValue();
	printf("PPU Time: %llu\n", endTime - startTime);		// about 110 us
#endif

#ifdef CHECK_NB_OVERLAPS
	printf("SPU: %d overlaps\n", total);
#endif

	mRestartCount++;
	
	if(needToRestart)
	{
#ifndef MBP_SINGLE_SPU
		for(PxU32 spuIndex=0;spuIndex<nbSpus;spuIndex++)
#endif
		{
#ifdef MBP_SINGLE_SPU
			mCellMBPSPUInput.mRestartCount	= mRestartCount;
			PX_ASSERT(firstFailing);	// PT: else even the first box pruner generated too many overlaps
			PX_UNUSED(firstFailing);

	//		printf("firstFailing: %d\n", firstFailing);

			const PxU32 nbRemain = mNbBoxPruners - mNbDone;
			mCellMBPSPUInput.mOutput = &mCellMBPSPUOutput;

			initPruners(nbRemain, mNbDone);
#else
			mCellMBPSPUInput[spuIndex].mRestartCount	= mRestartCount;
			PX_ASSERT(firstFailing[spuIndex]);	// PT: else even the first box pruner generated too many overlaps
			PX_UNUSED(firstFailing[spuIndex]);

	//		printf("firstFailing: %d\n", firstFailing[spuIndex]);

//			const PxU32 nbRemain = mNbBoxPruners - mNbDone;
			const PxU32 nbRemain = mCellMBPSPUInput[spuIndex].mTotalToDo - mNbDone[spuIndex];
//			mCellMBPSPUInput.mOutput = &mCellMBPSPUOutput;

//			initPruners(0, nbRemain, mNbDone);
			initPruners(spuIndex, nbRemain, mCellMBPSPUInput[spuIndex].mBaseIndex + mNbDone[spuIndex]);
#endif
		}
		setContinuation(getContinuation());
		removeReference();
	}

#ifdef MBP_USE_SCRATCHPAD
	for(PxU32 i=0; i<6; i++)
	{
		void* scratchAddress = mScratchAddresses[5-i];
		if(scratchAddress)
		{
			mAllocator->free(mScratchAddresses[5-i]);
			mScratchAddresses[5-i] = NULL;
			mScratchAddressesAligned[5-i] = NULL;
		}
	}
#endif

	if(!needToRestart)
	{
		for(PxU32 i=0;i<mRestartCount;i++)
			PxSpuTask::release();
	}
}

void MBPTaskSPU::init(PxcScratchAllocator& allocator, const PxsBroadPhaseMBP& bp)
{
	MBP* mbp = bp.mMBP;
	mMBP			= mbp;
	mRestartCount	= 0;

#ifdef MBP_SINGLE_SPU
	mNbDone = 0;
#else
	for(PxU32 i=0;i<6;i++)
		mNbDone[i]	= 0;
#endif

	const PxU32 nb = mbp->getNbBoxPruners();
	PX_ASSERT(nb<=256);

//	mNbBoxPruners					= nb;
//	for(PxU32 i=0;i<nb;i++)
//		mBoxPruners[i] = mbp->GetBoxPruner(i);

	PxU32 nbNeeded = 0;
	for(PxU32 i=0;i<nb;i++)
	{
		const BoxPruner* bp1 = mbp->getBoxPruner(i);
		if(bp1 && bp1->mNbUpdatedBoxes && bp1->mInput.mNeeded && bp1->mInput.mUpdatedDynamicBoxes)
		{
			mBoxPruners[nbNeeded++] = bp1;
		}
	}
	mNbBoxPruners = nbNeeded;

#ifdef MBP_SINGLE_SPU
	setArgs(0, (unsigned int)&mOverlaps, (unsigned int)&mCellMBPSPUInput);

	mCellMBPSPUInput.mRestartCount	= 0;
	mCellMBPSPUInput.mOutput		= &mCellMBPSPUOutput;
	initPruners(nbNeeded, 0);
#else
	const PxU32 nbSpus = getSpuCount();
	PX_ASSERT(nbSpus<=6);

#ifdef MBP_USE_SCRATCHPAD
	mAllocator = &allocator;
	for(PxU32 i=0; i<6; i++)
	{
		mScratchAddresses[i] = NULL;
		mScratchAddressesAligned[i] = NULL;
	}

	// PT: we need to align the scratch block to 16 for SPU DMAs
	const PxU32 desiredSize = 16 + MBP_MAX_NB_OVERLAPS*sizeof(MBP_Overlap);
#endif
	for(PxU32 i=0; i<nbSpus; i++)
	{
#ifdef MBP_USE_SCRATCHPAD
		void* buffer = allocator.alloc(desiredSize, true);
		PX_ASSERT(buffer);
		mScratchAddresses[i] = buffer;
		char* buffer8 = reinterpret_cast<char*>(buffer);
		const PxI32 bytesToPad = Cm::getPadding(size_t(buffer8), 16);
		buffer8 += bytesToPad;
		mScratchAddressesAligned[i] = buffer8;

		setArgs(i, (unsigned int)buffer8, (unsigned int)&mCellMBPSPUInput[i]);
#else
		setArgs(i, (unsigned int)&mOverlaps[i*MBP_MAX_NB_OVERLAPS], (unsigned int)&mCellMBPSPUInput[i]);
#endif
	}

	if(nbSpus==1)
	{
		mCellMBPSPUInput[0].mRestartCount	= 0;
		mCellMBPSPUInput[0].mTotalToDo		= nbNeeded;
		mCellMBPSPUInput[0].mBaseIndex		= 0;
		mCellMBPSPUInput[0].mOutput			= &mCellMBPSPUOutput[0];
		initPruners(0, nbNeeded, 0);
	}
	else
	{
		PxU32 counters[6];
		for(PxU32 i=0;i<6;i++)
			counters[i] = 0;

		// PT: load-balancing: there are 'nbNeeded' non-empty box-pruners to go through,
		// and 'nbSpus' available SPUs. We simply assign pruners to SPUs in linear order for now.
		for(PxU32 i=0;i<nbNeeded;i++)
		{
			const PxU32 index = i % nbSpus;
			counters[index]++;
		}

		PxU32 histogram[6];
		histogram[0] = 0;
		for(PxU32 i=1;i<6;i++)
			histogram[i] = histogram[i-1] + counters[i-1];

		for(PxU32 i=0;i<nbSpus;i++)
		{
			mCellMBPSPUInput[i].mRestartCount	= 0;
			mCellMBPSPUInput[i].mTotalToDo		= counters[i];
			mCellMBPSPUInput[i].mBaseIndex		= histogram[i];
			mCellMBPSPUInput[i].mOutput			= &mCellMBPSPUOutput[i];
			initPruners(i, counters[i], histogram[i]);
		}
	}
#endif
}

#endif

// TA16933
/*
	typedef void*	PxBPHandle;

	enum PxBPType
	{
		PX_BP_TYPE_SAP,
		PX_BP_TYPE_MBP,
	};

	enum PxBPObjectType
	{
		PX_BP_TYPE_STATIC,
		PX_BP_TYPE_DYNAMIC,
	};

	// Defines a "region of interest" for the BP
	class PxBPRegion
	{
		public:
				PxBounds3	mBounds;	// World space bounds
	};

	struct PxBPCaps
	{
		bool	mNeedsPredefinedBounds;	// If true, BP needs 'regions' to work (MBP). If false, it doesn't (SAP)
		PxU32	mMaxNbRegions;			// Max number of regions supported by the BP
		PxU32	mMaxNbObjects;			// Max number of objects supported by the BP
	};

	class PxBPOverlapCallback
	{
		public:
		// Created when a pair starts overlapping. Possibly returns a per-pair user-data to solve things like TA16952.
		virtual	void*		onOverlapCreated(void* object0UserData, void* object1UserData)						= 0;

		// Created when a pair stops overlapping.
		virtual	void		onOverlapRemoved(void* object0UserData, void* object1UserData, void* pairUserData)	= 0;
	};

	class PxBroadPhase
	{
		public:

		virtual	PxBPType	getType()														const	= 0;
		virtual	void		getCaps(PxBPCaps& caps)											const	= 0;

		virtual	bool		preallocate(PxU32 nbRegions, PxU32 nbObjects)							= 0;
		virtual	void		release()																= 0;

		virtual	bool		addRegion(const PxBPRegion& region)										= 0;
		virtual	PxU32		getNbRegions()													const	= 0;
		virtual	bool		getRegion(PxU32 i, PxBPRegion& region)							const	= 0;

		virtual	PxBPHandle	addObject(const PxBounds3& box, void* userData, PxBPObjectType type)	= 0;
		virtual	bool		removeObject(PxBPHandle handle)											= 0;
		virtual	bool		updateObject(PxBPHandle handle, const PxBounds3& box)					= 0;

		virtual	void		findOverlaps(PxBPOverlapCallback& callback)								= 0;
	};


//	PxBroadPhase*	PxPhysics::createBroadPhase(PxBPType type)	= 0;

	struct PxBPDesc
	{
		PxBPObjectType		mType;
		PxU32				mNbRegions;
		const PxBPRegion*	mRegions;
	};
*/

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

#ifndef SQ_BUCKETPRUNER_H
#define SQ_BUCKETPRUNER_H

#include "SqPruningPool.h"
#include "GuRevisitedRadixBuffered.h"
#include "PsHashMap.h"

#define FREE_PRUNER_SIZE	16

namespace physx
{
namespace Sq
{
	typedef PxU32	BucketWord;
	
#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

	PX_ALIGN_PREFIX(16)	struct BucketBox
	{
		PxVec3	mCenter;
		PxU32	mData0;		// Integer-encoded min value along sorting axis
		PxVec3	mExtents;
		PxU32	mData1;		// Integer-encoded max value along sorting axis
#ifndef PX_PS3
	#ifdef _DEBUG
		// PT: we need the original min value for debug checks. Using the center/extents version
		// fails because recomputing the min from them introduces FPU accuracy errors in the values.
		float	mDebugMin;
	#endif
#endif

		PX_FORCE_INLINE	PxVec3	getMin()	const
		{
			return mCenter - mExtents;
		}

		PX_FORCE_INLINE	PxVec3	getMax()	const
		{
			return mCenter + mExtents;
		}

		PX_FORCE_INLINE void	setEmpty()
		{
			mCenter = PxVec3(0.0f);
			mExtents = PxVec3(-PX_MAX_BOUNDS_EXTENTS);
#ifndef PX_PS3
	#ifdef _DEBUG
			mDebugMin = PX_MAX_BOUNDS_EXTENTS;
	#endif
#endif
		}
	}PX_ALIGN_SUFFIX(16);

	PX_ALIGN_PREFIX(16) struct BucketPrunerNode
	{
					BucketPrunerNode();

		void		classifyBoxes(	float limitX, float limitZ,
									PxU32 nb,
									BucketBox* PX_RESTRICT boxes,
									const PrunerPayload* PX_RESTRICT objects,
									BucketBox* PX_RESTRICT sortedBoxes,
									PrunerPayload* PX_RESTRICT sortedObjects,
									bool isCrossBucket, PxU32 sortAxis);

		PX_FORCE_INLINE	void	initCounters()
		{
			for(PxU32 i=0;i<5;i++)
				mCounters[i] = 0;
			for(PxU32 i=0;i<5;i++)
				mOffsets[i] = 0;
		}

		BucketWord	mCounters[5];	// Number of objects in each of the 5 children
		BucketWord	mOffsets[5];	// Start index of objects for each of the 5 children
		BucketBox	mBucketBox[5];	// AABBs around objects for each of the 5 children
		PxU16		mOrder[8];		// PNS: 5 children => 3 bits/index => 3*5=15 bits total, for each of the 8 canonical directions
	}PX_ALIGN_SUFFIX(16);

	PX_FORCE_INLINE PxU32 hash64(const PxU64 key)
	{
		PxU64 k = key;
		k += ~(k << 32);
		k ^= (k >> 22);
		k += ~(k << 13);
		k ^= (k >> 8);
		k += (k << 3);
		k ^= (k >> 15);
		k += ~(k << 27);
		k ^= (k >> 31);
		return (PxU32)(PX_MAX_U32 & k);
	}

	PX_FORCE_INLINE PxU32 hash(const PrunerPayload& payload)
	{
#if defined(PX_P64)
		const PxU32 h0 = Ps::hash((const void*)payload.data[0]);
		const PxU32 h1 = Ps::hash((const void*)payload.data[1]);
		return hash64(PxU64(h0)|(PxU64(h1)<<32));
#else
		return hash64(PxU64(payload.data[0])|(PxU64(payload.data[1])<<32));
#endif
	}

	typedef Ps::HashMap<PrunerPayload, PxU32> BucketPrunerMap;

	class BucketPrunerCore : public Ps::UserAllocated
	{
		public:
											BucketPrunerCore(bool externalMemory=true);
											~BucketPrunerCore();

						void				release();

						void				setExternalMemory(PxU32 nbObjects, PxBounds3* boxes, PrunerPayload* objects);

						bool				addObject(const PrunerPayload& object, const PxBounds3& worldAABB);
						bool				removeObject(const PrunerPayload& object);
						bool				updateObject(const PxBounds3& worldAABB, const PrunerPayload& object);

						PxAgain				raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const;
						PxAgain				overlap(const ShapeData& queryVolume, PrunerCallback&) const;
						PxAgain				sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const;

						void				shiftOrigin(const PxVec3& shift);

						void				visualize(Cm::RenderOutput& out, PxU32 color) const;

		PX_FORCE_INLINE	void				build()					{ classifyBoxes();	}

//		private:
						PxU32				mCoreNbObjects;		// Current number of objects in core arrays
						PxU32				mCoreCapacity;		// Capacity of core arrays
						PxBounds3*			mCoreBoxes;			// Core array
						PrunerPayload*		mCoreObjects;		// Core array
						PxU32*				mCoreRemap;			// Remaps core index to sorted index, i.e. sortedIndex = mCoreRemap[coreIndex]

						BucketBox*			mSortedWorldBoxes;	// Sorted array
						PrunerPayload*		mSortedObjects;		// Sorted array

						PxU32				mNbFree;						// Current number of objects in the "free array" (mFreeObjects/mFreeBounds)
						PrunerPayload		mFreeObjects[FREE_PRUNER_SIZE];	// mNbFree objects are stored here
						PxBounds3			mFreeBounds[FREE_PRUNER_SIZE];	// mNbFree object bounds are stored here

						BucketPrunerMap		mMap;			// Maps (PrunerPayload) object to corresponding index in core array.
															// Objects in the free array do not appear in this map.
						PxU32				mSortedNb;
						PxU32				mSortedCapacity;
						PxU32				mSortAxis;

						BucketBox			mGlobalBox;		// Global bounds around all objects in the structure (except the ones in the "free" array)
						BucketPrunerNode	mLevel1;
						BucketPrunerNode	mLevel2[5];
						BucketPrunerNode	mLevel3[5][5];

						bool				mDirty;
						bool				mOwnMemory;
		private:
						void				classifyBoxes();
						void				allocateSortedMemory(PxU32 nb);
						void				resizeCore();
		PX_FORCE_INLINE	void				addObjectInternal(const PrunerPayload& object, const PxBounds3& worldAABB);
	};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

	class BucketPruner
#if !PX_IS_SPU
		: public Pruner
#endif
	{
		public:
											BucketPruner();
		virtual								~BucketPruner();

						bool				addObjects(PrunerHandle* results, const PxBounds3* bounds, const PrunerPayload* payload, PxU32 count);
						void				removeObjects(const PrunerHandle* handles, PxU32 count);
						void				updateObjects(const PrunerHandle* handles, const PxBounds3* newBounds, PxU32 count);

						PxAgain				raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const;
						PxAgain				overlap(const ShapeData& queryVolume, PrunerCallback&) const;
						PxAgain				sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const;

						const PrunerPayload& getPayload(const PrunerHandle& h) const { return mPool.getPayload(h); }
						
						void				preallocate(PxU32 entries) { mPool.preallocate(entries); }

		virtual			void				commit();

		virtual			void				shiftOrigin(const PxVec3& shift);

		virtual			void				visualize(Cm::RenderOutput& out, PxU32 color) const;

		private:
						BucketPrunerCore	mCore;
						PruningPool			mPool;
	};

} // namespace Sq

}

#endif // SQ_BUCKETPRUNER_H

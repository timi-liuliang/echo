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

#ifndef OPC_AABBTREE_H
#define OPC_AABBTREE_H

#include "PxMemory.h"
#include "PsUserAllocated.h"
#include "PsVecMath.h"
#include "PxBounds3.h"

#define SUPPORT_PROGRESSIVE_BUILDING
#define SUPPORT_REFIT_BITMASK
#define SUPPORT_UPDATE_ARRAY	128	// PT: consumes SUPPORT_UPDATE_ARRAY*sizeof(PxU32) bytes per AABBTree

namespace physx
{

using namespace shdfnd::aos;

namespace Sq
{

static const VecU32V ff = VecU32VLoadXYZW(0xFF, 0xFF, 0xFF, 0xFF);
static const Vec4V   scaleMul4 = V4Load(1.0f/10000.0f);
static const PxF32   scaleMul1 = 1.0f/10000.0f;

	class Plane;
	class Container;
	class AABBTreeBuilder;

	class BitArray
	{
		public:
										BitArray() : mBits(NULL), mSize(0) {}
										BitArray(PxU32 nb_bits) { init(nb_bits); }
										~BitArray() { PX_FREE_AND_RESET(mBits); mBits = NULL; }

						bool			init(PxU32 nb_bits);

		// Data management
		PX_FORCE_INLINE	void			setBit(PxU32 bit_number)
										{
											mBits[bit_number>>5] |= 1<<(bit_number&31);
										}
		PX_FORCE_INLINE	void			clearBit(PxU32 bit_number)
										{
											mBits[bit_number>>5] &= ~(1<<(bit_number&31));
										}
		PX_FORCE_INLINE	void			toggleBit(PxU32 bit_number)
										{
											mBits[bit_number>>5] ^= 1<<(bit_number&31);
										}

		PX_FORCE_INLINE	void			clearAll()			{ PxMemZero(mBits, mSize*4);		}
		PX_FORCE_INLINE	void			setAll()			{ PxMemSet(mBits, 0xff, mSize*4);	}

		// Data access
		PX_FORCE_INLINE	Ps::IntBool		isSet(PxU32 bit_number)	const
										{
											return Ps::IntBool(mBits[bit_number>>5] & (1<<(bit_number&31)));
										}

		PX_FORCE_INLINE	const PxU32*	getBits()	const	{ return mBits;		}
		PX_FORCE_INLINE	PxU32			getSize()	const	{ return mSize;		}

		protected:
						PxU32*			mBits;		//!< Array of bits
						PxU32			mSize;		//!< Size of the array in dwords
	};


#ifdef SUPPORT_PROGRESSIVE_BUILDING
	class FIFOStack2;
#endif
	class AABBTreeNode;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	User-callback, called for each node by the walking code.
	 *	\param		current		[in] current node
	 *	\param		depth		[in] current node's depth
	 *	\param		user_data	[in] user-defined data
	 *	\return		true to recurse through children, else false to bypass them
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef		bool				(*WalkingCallback)	(const AABBTreeNode* current, PxU32 depth, void* user_data);

	class AABBTree : public Ps::UserAllocated
	{
		public:
											AABBTree();
											~AABBTree();
		// Build
						bool				build(AABBTreeBuilder* builder);
#ifdef SUPPORT_PROGRESSIVE_BUILDING
						PxU32				progressiveBuild(AABBTreeBuilder* builder, PxU32 progress, PxU32 limit);
#endif
						void				release();

		// Data access
		PX_FORCE_INLINE	const PxU32*		getIndices()		const	{ return mIndices;		}
		PX_FORCE_INLINE	PxU32*				getIndices()				{ return mIndices;		}
		PX_FORCE_INLINE	PxU32				getNbNodes()		const	{ return mTotalNbNodes;	}
		PX_FORCE_INLINE	PxU32				getTotalPrims()		const	{ return mTotalPrims;	}
		PX_FORCE_INLINE	const AABBTreeNode*	getNodes()			const	{ return mPool;			}
		PX_FORCE_INLINE	AABBTreeNode*		getNodes()					{ return mPool;			}

		// Stats
						PxU32				computeDepth()		const;
						PxU32				walk(WalkingCallback callback, void* user_data) const;
						void				walk2(WalkingCallback callback, void* user_data) const;

						bool				refit2(AABBTreeBuilder* builder, PxU32* indices);
#ifndef SUPPORT_REFIT_BITMASK
						bool				refit3(PxU32 nb_objects, const PxBounds3* boxes, const Container& indices);
#endif
#ifdef PX_DEBUG 
						void				validate() const;
#endif
						// adds node[index] to a list of nodes to refit when refitMarked is called
						// Note that this includes updating the hierarchy up the chain
						void				markForRefit(PxU32 index);
						void				refitMarked(PxU32 nb_objects, const PxBounds3* boxes, PxU32* indices);

						void				shiftOrigin(const PxVec3& shift);
#if PX_IS_SPU // SPU specific pointer patching
		PX_FORCE_INLINE	void				setNodes(AABBTreeNode* lsPool) {  mPool = lsPool; }
#endif
		private:
						PxU32*				mIndices;	//!< Indices in the app list. Indices are reorganized during build (permutation).
						AABBTreeNode*		mPool;		//!< Linear pool of nodes for complete trees. NULL otherwise. [Opcode 1.3]
						BitArray			mRefitBitmask; //!< bit is set for each node index in markForRefit
						PxU32				mRefitHighestSetWord;
#ifdef SUPPORT_UPDATE_ARRAY
						PxU32				mNbRefitNodes;
						PxU32				mRefitArray[SUPPORT_UPDATE_ARRAY];
#endif
		// Stats
						PxU32				mTotalNbNodes;		//!< Number of nodes in the tree.
						PxU32				mTotalPrims;
#ifdef SUPPORT_PROGRESSIVE_BUILDING
						FIFOStack2*			mStack;
#endif
						friend class AABBTreeNode;
	};

	// AP: adding #pragma pack(push,4) makes linux64 crash but saves 4 bytes per node
	// currently on 64 bit platforms the size is 24 bytes
	class AABBTreeNode : public Ps::UserAllocated
	{
		public:	
		PX_FORCE_INLINE					AABBTreeNode() : mBitfield(0) {}
		PX_FORCE_INLINE					~AABBTreeNode() {}
		// Data access
		// Decompress this node's bounds given parent center and parent extents
		PX_INLINE	const PxVec3&		getAABBCenter() const { return *(PxVec3*)&mCx; }
		PX_INLINE	PxVec3				getAABBExtents() const { PxI32 scale = PxI32(getExtentScale() << 8); return PxVec3(PxReal(mEx & 0xFF), PxReal(mEy & 0xFF), PxReal(mEz & 0xFF)) * PX_FR(scale) * scaleMul1; }
		PX_FORCE_INLINE void			getAABBCenterExtentsV(Vec3V* center, Vec3V* extents) const;

		// Child/parent access
		PX_INLINE	const AABBTreeNode*	getPos(const AABBTreeNode* base) const { PX_ASSERT(!isLeaf()); PxU32 pos = getPosOrNodePrimitives(); return base + pos; } // 0 returns base, so backptr to root equivalent to NULL
		PX_INLINE	const AABBTreeNode*	getNeg(const AABBTreeNode* base) const { const AABBTreeNode* P = getPos(base); return P != base ? P+1 : base; }
		PX_INLINE	const AABBTreeNode*	getParent(const AABBTreeNode* base) const { return base + getNbBuildPrimitivesOrParent(); }

		// Leaf flag access
		PX_INLINE	PxU32				isLeaf()		const	{ return getIsLeaf(); }
		PX_INLINE	void				setLeaf()				{ setIsLeaf(1); }
		PX_INLINE	void				clearLeaf()				{ setIsLeaf(0); }

		// Stats
		PX_INLINE	PxU32				getNodeSize()	const	{ return sizeof(*this);	}
		PX_INLINE	void				setPos(PxU32 val) { PX_ASSERT(!isLeaf()); setPosOrNodePrimitives(val); }
		PX_INLINE	void				setNodePrimitives(PxU32 val) { PX_ASSERT(!isLeaf()); setPosOrNodePrimitives(val); }
		PX_INLINE	void				setParent(PxU32 val) { setNbBuildPrimitivesOrParent(val); }
		PX_INLINE	void				setNbBuildPrimitives(PxU32 val) { setNbBuildPrimitivesOrParent(val); }

		// WARNING: typically you want to use Compress<tWriteBack=1>
		// calling this function with tWriteBack=0 must be followed by a call to writeBack
		// passing the values previously returned from Compress
		template<int tWriteBack>
		PX_FORCE_INLINE	void			compress(const Vec3V exactBoundsMin, const Vec3V exactBoundsMax, VecU32V* result1 = NULL, Vec3V* result2 = NULL);
		PX_FORCE_INLINE void			writeBack(const VecU32V& scale, const Vec3V& pos)
										{
											V3StoreU(pos, *(PxVec3*)&mCx);
											setExtentScale( (((PxU32*)&scale)[1] >> 8) );
										}
		// Data access
		PX_INLINE	const PxU32*		getPrimitives(const PxU32* treeIndices) const { return treeIndices + getPosOrNodePrimitives(); }
		PX_INLINE	PxU32*				getPrimitives(PxU32* treeIndices) const { return treeIndices + getPosOrNodePrimitives(); }
		PX_INLINE	PxU32				getNodePrimitives() const { PX_ASSERT(isLeaf()); return getPosOrNodePrimitives(); }
		PX_INLINE	PxU32				getNbBuildPrimitives() const { return getNbBuildPrimitivesOrParent(); }

					// Compressed node AABB center and extents
					// upper 24 bits of mCx,mCy,mCz are used for the float representation of center's xyz
					// lower 8 bits of mEx,mEy,mEz are used for extents.
					// The extents are computed from compressed representation by multiplying by mExtentScale*GetExtentScaleMultiplier()
					// mExtentScale is stored as a clipped float (lower 8 bits of mantissa are dropped)
					// see GetAABBCenter(), GetAABBExtents() for decompression code and Compress() for compression code
					// GetAABBCenterExtentsV() is a SIMD optimized version that decompresses both center and extents in one shot
					union { PxF32 mCx; PxU32 mEx; };
					union { PxF32 mCy; PxU32 mEy; };
					union { PxF32 mCz; PxU32 mEz; };

					// the bitfield layout is, in this order, most significant bits first:
					// mExtentScale,
					// mPosOrNodePrimitives,
					// mNbBuildPrimitivesOrParent,
					// mNbRuntimePrimitives,
					// mIsLeaf
					enum { EXSC_BITS = 24, POSNODE_BITS = 19, BUILDPARENT_BITS = 19, NBRUNTIME_BITS = 1, ISLEAF_BITS = 1 };
					enum { EXSC_OFFS = 63, POSNODE_OFFS = EXSC_OFFS-EXSC_BITS, BUILDPARENT_OFFS = POSNODE_OFFS-POSNODE_BITS,
						NBRUNTIME_OFFS = BUILDPARENT_OFFS-BUILDPARENT_BITS, ISLEAF_OFFS = NBRUNTIME_OFFS-NBRUNTIME_BITS };

					// start is the index of the bit in mBitfield, count the number of bits down from there
					PX_INLINE PxU32 getBits(PxU32 start, PxU32 count) const
					{
						PX_ASSERT(count >= 1 && count <= 63);
						PX_ASSERT(start <= 63); // check that start and count are within range
						PX_ASSERT(PxI32(start+1) - PxI32(count) >= 0);
						return Ps::to32((mBitfield << (63-start)) >> (64-count));
					}

					PX_INLINE void setBits(PxU64 val, PxU32 start, PxU32 count)
					{
						PX_ASSERT(val < PxU32(PxU64(1)<<count));
						PX_ASSERT(count >= 1 && count <= 63);
						PX_ASSERT(start <= 63);
						PX_ASSERT(PxI32(start+1) - PxI32(count) >= 0);
								#ifdef PX_DEBUG // verify that we preserved the bits on the boundaries
								PxU32 oldHighBit = (start < 63) ? getBits(start+1, 1) : 0;
								PxU32 oldLowBit = (PxI32(start)-PxI32(count) >= 0) ? getBits(start-count, 1) : 0;
								#endif
						PxU64 clearMask = ~(((PxU64(1)<<count)-1)<<(start+1-count));
						mBitfield = (mBitfield & clearMask) | (val << (start+1-count));
						PX_ASSERT(getBits(start, count) == val); // verify that we read what we just wrote
								#ifdef PX_DEBUG
								// verify that we preserved bits on the boundaries
								PX_ASSERT(start < 63 ? oldHighBit == getBits(start+1, 1) : true);
								PX_ASSERT(PxI32(start)-PxI32(count) >= 0 ? oldLowBit == getBits(start-count, 1) : true);
								#endif
					}

					PX_INLINE PxU32	getExtentScale() const					
					{ 						
						#if PX_IS_INTEL
						return ((const PxU32*)&mBitfield)[1]>>8;
						#else
						return getBits(EXSC_OFFS, EXSC_BITS);
						#endif
					}
					PX_INLINE void	setExtentScale(PxU32 val)				{ setBits(val, EXSC_OFFS, EXSC_BITS); }
					PX_INLINE PxU32	getPosOrNodePrimitives() const			{ return getBits(POSNODE_OFFS, POSNODE_BITS); }
					PX_INLINE void	setPosOrNodePrimitives(PxU32 val)		{ setBits(val, POSNODE_OFFS, POSNODE_BITS); }
					PX_INLINE PxU32	getNbBuildPrimitivesOrParent() const	{ return getBits(BUILDPARENT_OFFS, BUILDPARENT_BITS); }
					PX_INLINE void	setNbBuildPrimitivesOrParent(PxU32 val)	{ setBits(val, BUILDPARENT_OFFS, BUILDPARENT_BITS); }
					PX_INLINE PxU32	getNbRuntimePrimitives() const			{ return getBits(NBRUNTIME_OFFS, NBRUNTIME_BITS); }
					PX_INLINE void	setNbRunTimePrimitives(PxU32 val)		{ setBits((val & ((1<<NBRUNTIME_BITS)-1)), NBRUNTIME_OFFS, NBRUNTIME_BITS); }
					PX_INLINE PxU32	getIsLeaf() const						{ return getBits(ISLEAF_OFFS, ISLEAF_BITS); }
					PX_INLINE void	setIsLeaf(PxU32 val)					{ setBits(val, ISLEAF_OFFS, ISLEAF_BITS); }

					PxU64			mBitfield; 

					// Internal methods
					PX_INLINE PxU32		split(const PxBounds3& exactBounds, PxU32 axis, AABBTreeBuilder* builder, PxU32* indices);
					PX_INLINE bool		subdivide(const PxBounds3& exactBounds, AABBTreeBuilder* builder, PxU32* indices);
					PX_INLINE void		_buildHierarchy(AABBTreeBuilder* builder, PxU32* indices);
	};

#define PX_IS_DENORM(x) (x != 0.0f && PxAbs(x) < FLT_MIN)
#define PX_AABB_COMPRESSION_MAX 1e33f // max value supported by compression

static const FloatV one256th = FLoad(1.0f/254.999f);
// original scale*=(1+1/255), bake this into invExtentScaleMul
static const Vec3V invExtentScaleMul = V3Recip(Vec3V_From_Vec4V(V4Load(1.0f/10000.0f)));
static const FloatV half = FHalf();
static const FloatV ep4 = FLoad(1e-4f);
static const FloatV fmin4 = FLoad(2.0f*FLT_MIN);
static const VecU32V ff00 = VecU32VLoadXYZW(0xFFffFF00, 0xFFffFF00, 0xFFffFF00, 0xFFffFF00);
static const FloatV ones = FOne();
static const Vec3V clampBounds = Vec3V_From_FloatV(FLoad(PX_AABB_COMPRESSION_MAX)); // clamp the bounds to compression allowed max


// The specs for compression are:
// 1. max(abs(bounds.min.xyz), abs(bounds.max.xyz))<PX_AABB_COMPRESSION_MAX
// 2. decompressedOutput.contains(originalInput) for extents >= 0
// 3. if originalInput.getExtents().xyz < 0 then decompressedOutput.getExtents() will also be negative but there are no inclusion or any other guarantees
template<int tWriteBack>
PX_FORCE_INLINE void AABBTreeNode::compress(
	const Vec3V aExactBoundsMin, const Vec3V aExactBoundsMax, VecU32V* result1, Vec3V* result2)
{
	// assert that aabb min is under compression max and aabb max is over minus compression min to save clamping instructions
	PX_ASSERT_WITH_MESSAGE(FStore(V3ExtractMax(aExactBoundsMin))<PX_AABB_COMPRESSION_MAX ,"Bounds out of range in AABB::Compress");
	PX_ASSERT_WITH_MESSAGE(FStore(V3ExtractMin(aExactBoundsMax))>-PX_AABB_COMPRESSION_MAX ,"Bounds out of range in AABB::Compress");
	Vec3V exactBoundsMin = V3Max(aExactBoundsMin, V3Neg(clampBounds)); // only clamp the min from below to save 2 instructions
	Vec3V exactBoundsMax = V3Min(aExactBoundsMax, clampBounds); // only clamp the max from above to save 2 instructions

	Vec3V exactCenter = V3Scale(V3Add(exactBoundsMin, exactBoundsMax), half); // compute exact center
	Vec3V exactExtents = V3Scale(V3Sub(exactBoundsMax, exactBoundsMin), half); // compute exact extents

	// see how much in float space we can be most off by in max(x,y,z) after overwriting the low 8 bits
	// errCenter = maximum error introduced by clipping the low 8 bits of exactCenter
	// add ep4 do avoid a divide by zero later
	Vec3V errCenter = V3Add(V3Abs(V3Mul(exactCenter, Vec3V_From_FloatV(ep4))), Vec3V_From_FloatV(ep4));

	// forward formula:
	// PxI32 scale = PxI32(mExtentScale) << 8;
	// return PxVec3(mEx & 0xFF, mEy & 0xFF, mEz & 0xFF) * PX_FR(scale) * GetExtentScaleMultiplier(); }

	// we now try to convert 3x float extents to 3x8 bit values with a common 24-bit multiplier
	// we do this by finding the maximum of 3 extents and a multiplier such that dequantization produces conservative bounds
	Vec3V errCenterExactExtents = V3Add(errCenter, exactExtents); // we incorporate error from center compression by inflating extents
	Vec3V splatExtentX = Vec3V_From_Vec4V(V4SplatElement<0>(Vec4V_From_Vec3V(errCenterExactExtents))); // includes center error
	Vec3V splatExtentY = Vec3V_From_Vec4V(V4SplatElement<1>(Vec4V_From_Vec3V(errCenterExactExtents))); // includes center error
	Vec3V splatExtentZ = Vec3V_From_Vec4V(V4SplatElement<2>(Vec4V_From_Vec3V(errCenterExactExtents))); // includes center error
	Vec3V maxExtentXYZ = V3Max(splatExtentX, V3Max(splatExtentY, splatExtentZ)); // find the maximum of extent.x, extent.y, extent.z
	Vec3V maxExtent255 = V3Scale(maxExtentXYZ, one256th); // divide maxExtent by 255 so we can fit into 0..255 range
	// add error, inv scale maxExtent255*= 1.0f + ep4. scaleMul4 is to improve precision
	Vec3V maxExtent255e = V3Mul(V3Add(maxExtent255, V3Mul(maxExtent255, Vec3V_From_FloatV(ep4))), invExtentScaleMul);
	VecU32V u;
	if (tWriteBack)
		V4U32StoreAligned(VecU32V_ReinterpretFrom_Vec4V(Vec4V_From_Vec3V(maxExtent255e)), &u);
	else
		V4U32StoreAligned(VecU32V_ReinterpretFrom_Vec4V(Vec4V_From_Vec3V(maxExtent255e)), result1);

	Vec3V valXYZ = V3Mul(V3Add(exactExtents, errCenter), V3Recip(maxExtent255));
	VecI32V valXYZi = VecI32V_From_Vec4V(Vec4V_From_Vec3V(V3Add(valXYZ, Vec3V_From_FloatV(ones))));
	valXYZ = Vec3V_From_Vec4V(Vec4V_ReinterpretFrom_VecI32V(valXYZi));


#ifdef PX_DEBUG
	PX_ALIGN_PREFIX(16) PxU32 test[4] PX_ALIGN_SUFFIX(16) = {255, 255, 255, 255};
	V4U32StoreAligned(VecU32V_ReinterpretFrom_Vec4V(Vec4V_From_Vec3V(valXYZ)), (VecU32V*)test);
	PX_ASSERT(test[0]<=255 && test[1]<=255 && test[2]<=255 && test[3]<=255);
#endif
	// the input (exact center in floats) has to be correct meaning it's either zero or non-denorm
	// if its a non-denorm we can put anything into the low 8 bits and maintain non-denorm
	// if it's a strict zero we replace it with fmin4
	// next we mask off the low 8 bits of mantissa, logical or with quantized 8-bit xyz extents
	Vec3V zero = V3Sub(Vec3V_From_FloatV(ones), Vec3V_From_FloatV(ones)); // should be faster than loading FZero() i think
	Vec3V nonzeroExactCenter = V3Sel(V3IsEq(exactCenter, zero), Vec3V_From_FloatV(fmin4), exactCenter); 
	VecU32V resu32 =
		V4U32or(
			V4U32and(VecU32V_ReinterpretFrom_Vec4V(Vec4V_From_Vec3V( nonzeroExactCenter )), ff00),
			VecU32V_ReinterpretFrom_Vec4V(Vec4V_From_Vec3V(valXYZ)));
	Vec3V res = Vec3V_From_Vec4V(Vec4V_ReinterpretFrom_VecU32V(resu32));
	if (tWriteBack)
	{
		V3StoreU(res, *(PxVec3*)&mCx);
		PX_ASSERT(!PX_IS_DENORM(mCx) && !PX_IS_DENORM(mCy) && !PX_IS_DENORM(mCz));
		setExtentScale( (((PxU32*)&u)[1] >> 8) );
	} else
		*result2 = res;

#ifdef PX_DEBUG
	if (!tWriteBack)
		return;
	else
	{
		// verify that compression produced conservative bounds
		PxI32 es = PxI32(getExtentScale() << 8);
		PX_ASSERT(!PX_IS_DENORM(PX_FR(es)));
		PxF32 escale = PX_FR(es) * scaleMul1; PX_UNUSED(escale);
		PxVec3 deqCenter = getAABBCenter(), deqExtents = getAABBExtents();
		PxBounds3 exactBounds;
		V3StoreU(exactBoundsMin, exactBounds.minimum);
		V3StoreU(exactBoundsMax, exactBounds.maximum);
		PxBounds3 deqBounds = PxBounds3::centerExtents(deqCenter, deqExtents);
		PX_ASSERT(((exactBounds.minimum.x > exactBounds.maximum.x) && (deqBounds.minimum.x > deqBounds.maximum.x)) || exactBounds.isInside(deqBounds));

		PX_ASSERT(!PX_IS_DENORM(mCx) && !PX_IS_DENORM(mCy) && !PX_IS_DENORM(mCz));
	}
#endif
}


#if (defined(PX_X86) || defined (PX_X64))
PX_FORCE_INLINE Vec4V convertScale(PxI32 scale)
{
#if COMPILE_VECTOR_INTRINSICS
	return _mm_castsi128_ps(_mm_cvtsi32_si128(scale));
#else
	PX_ASSERT(false);
	return Vec4V();
#endif
}
#endif

#if (defined(PX_X86) || defined (PX_X64))
PX_FORCE_INLINE Vec4V splatScale(Vec4V s)
{
#if COMPILE_VECTOR_INTRINSICS
	return _mm_shuffle_ps(s, s, 0);  //isn't this the same as V4SplatElement<0>(s) ?
#else
	PX_ASSERT(false);
	return Vec4V();
#endif
}
#endif

PX_FORCE_INLINE	void AABBTreeNode::getAABBCenterExtentsV(Vec3V* center, Vec3V* extents) const
{
	Vec3V xyz = V3LoadU(*(PxVec3*)&mCx);
	PX_ASSERT(!PX_IS_DENORM(mCx) && !PX_IS_DENORM(mCy) && !PX_IS_DENORM(mCz));
	*center = xyz;
#if defined(PX_X360) // not the same bitfield layout on PS3 PPU
	// AP: load straight from a bitmask into vec4v. sketchy but works. eliminates a couple LHS. verification code below.
	Vec4V xyzs = V4LoadU(&mCy);
	xyzs = V4Andc(V4SplatElement<3>(xyzs), ff); // splat scale
	FloatV multiplier = V4Mul(xyzs, scaleMul4);
	#ifdef PX_DEBUG
		PxI32 verify; PxI32_From_VecI32V(VecI32V_ReinterpretFrom_Vec4V(xyzs), &verify);
		PX_ASSERT((PxI32(getExtentScale()) << 8) == verify); // verify that the bitfield layout is in sync with load code and hasn't changed
	#endif
#else
	PxI32 scale = PxI32(getExtentScale()) << 8;
	PX_ASSERT(!PX_IS_DENORM(scale));

	#if (defined(PX_X86) || defined (PX_X64))
		Vec4V s = convertScale(scale); // AP todo: make this cross-platform
		Vec4V scaleV =splatScale(s);
		Vec4V multiplier = V4Mul(scaleMul4, scaleV);
	#else
		FloatV multiplier = FLoad(PX_FR(scale) * scaleMul1);
	#endif
#endif
	VecU32V toInt = V4U32and(VecU32V_ReinterpretFrom_Vec4V(Vec4V_From_Vec3V(xyz)), ff);
	Vec4V extents3 = Vec4V_From_VecI32V((VecI32V&)toInt);
	// we know xyz has a 0 in W, hence so does toInt, hence also extents3. 

	#if (defined(PX_X86) || defined (PX_X64))
		*extents = Vec3V_From_Vec4V_WUndefined(V4Mul(extents3, multiplier));
	#else
		*extents = Vec3V_From_Vec4V_WUndefined(V4Mul(extents3, Vec4V_From_FloatV(multiplier)));
	#endif
	PX_ASSERT(!PX_IS_DENORM(((PxVec3*)extents)->x));
	PX_ASSERT(!PX_IS_DENORM(((PxVec3*)extents)->y));
	PX_ASSERT(!PX_IS_DENORM(((PxVec3*)extents)->z));
	PX_ASSERT(((PxVec4*)extents)->w==0);
}

} // namespace Gu

}

#endif // OPC_AABBTREE_H

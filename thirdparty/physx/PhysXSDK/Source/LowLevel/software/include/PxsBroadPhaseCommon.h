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


#ifndef PXS_BROADPHASE_COMMON_H
#define PXS_BROADPHASE_COMMON_H

#include "CmPhysXCommon.h"
#include "PsVecMath.h"
#include "PsFPU.h"
#include "PxBounds3.h"

namespace physx
{

typedef PxU32 PxcBPValType;

class PxvBroadPhase;

PX_FORCE_INLINE PxU32 PxsBpEncodeFloat(PxU32 newPos)
{
	//we may need to check on -0 and 0
	//But it should make no practical difference.
	if(newPos & 0x80000000) //negative?
		return ~newPos;//reverse sequence of negative numbers
	else
		return newPos | 0x80000000; // flip sign
}

PX_FORCE_INLINE PxU32 PxsBpDecodeFloat(PxU32 ir)
{
	if(ir & 0x80000000) //positive?
		return ir & ~0x80000000; //flip sign
	else
		return ~ir; //undo reversal
}

#define PX_BPVAL_SNAP_TO_GRID 1
#define PX_BPVAL_GRID_SNAP_VAL 4

PX_FORCE_INLINE PxcBPValType PxsBpEncodeFloatMin(PxU32 source)
{
	return ((PxsBpEncodeFloat(source) >> PX_BPVAL_GRID_SNAP_VAL) - 1) << PX_BPVAL_GRID_SNAP_VAL;
}

PX_FORCE_INLINE PxcBPValType PxsBpEncodeFloatMax(PxU32 source)
{
	return ((PxsBpEncodeFloat(source) >> PX_BPVAL_GRID_SNAP_VAL) + 1) << PX_BPVAL_GRID_SNAP_VAL;
}

//////////////////////////////////////////////////////////////

#if PX_USE_16_BIT_HANDLES
typedef PxU16 PxcBpHandle;
#define PX_INVALID_BP_HANDLE	0xffff
#else
typedef PxU32 PxcBpHandle;
#define PX_INVALID_BP_HANDLE	0x3fffffff
#endif

//////////////////////////////////////////////////////////////

struct IntegerAABB
{
	enum
	{
		MIN_X = 0,
		MIN_Y,
		MIN_Z,
		MAX_X,
		MAX_Y,
		MAX_Z
	};

	PX_FORCE_INLINE PxcBPValType	getMin(PxU32 i)	const	{	return (mMinMax)[MIN_X+i];	}
	PX_FORCE_INLINE PxcBPValType	getMax(PxU32 i)	const	{	return (mMinMax)[MAX_X+i];	}

	PX_FORCE_INLINE PxcBPValType	getExtent(PxU32 isMax, PxU32 index) const
	{
		PX_ASSERT(isMax<=1);
		return (mMinMax)[3*isMax+index];
	}

	PX_FORCE_INLINE PxcBPValType getMinX() const { return mMinMax[MIN_X]; }
	PX_FORCE_INLINE PxcBPValType getMinY() const { return mMinMax[MIN_Y]; }
	PX_FORCE_INLINE PxcBPValType getMinZ() const { return mMinMax[MIN_Z]; }
	PX_FORCE_INLINE PxcBPValType getMaxX() const { return mMinMax[MAX_X]; }
	PX_FORCE_INLINE PxcBPValType getMaxY() const { return mMinMax[MAX_Y]; }
	PX_FORCE_INLINE PxcBPValType getMaxZ() const { return mMinMax[MAX_Z]; }

	PX_FORCE_INLINE	void encode(const PxBounds3& bounds)
	{
		const PxU32* PX_RESTRICT min = PxUnionCast<const PxU32*, const PxF32*>(&bounds.minimum.x);
		const PxU32* PX_RESTRICT max = PxUnionCast<const PxU32*, const PxF32*>(&bounds.maximum.x);
		//Avoid min=max by enforcing the rule that mins are even and maxs are odd.
		mMinMax[MIN_X] = (PxsBpEncodeFloatMin(min[0]) + 1) & ~1;
		mMinMax[MIN_Y] = (PxsBpEncodeFloatMin(min[1]) + 1) & ~1;
		mMinMax[MIN_Z] = (PxsBpEncodeFloatMin(min[2]) + 1) & ~1;
		mMinMax[MAX_X] = PxsBpEncodeFloatMax(max[0]) | 1;
		mMinMax[MAX_Y] = PxsBpEncodeFloatMax(max[1]) | 1;
		mMinMax[MAX_Z] = PxsBpEncodeFloatMax(max[2]) | 1;
	}

	PX_FORCE_INLINE	void decode(PxBounds3& bounds)	const
	{
		PxU32* PX_RESTRICT min = PxUnionCast<PxU32*, PxF32*>(&bounds.minimum.x);
		PxU32* PX_RESTRICT max = PxUnionCast<PxU32*, PxF32*>(&bounds.maximum.x);
		min[0] = PxsBpDecodeFloat(mMinMax[MIN_X]);
		min[1] = PxsBpDecodeFloat(mMinMax[MIN_Y]);
		min[2] = PxsBpDecodeFloat(mMinMax[MIN_Z]);
		max[0] = PxsBpDecodeFloat(mMinMax[MAX_X]);
		max[1] = PxsBpDecodeFloat(mMinMax[MAX_Y]);
		max[2] = PxsBpDecodeFloat(mMinMax[MAX_Z]);
	}

	PX_FORCE_INLINE void shift(const PxVec3& shift)
	{
		PxBounds3 elemBounds;
		decode(elemBounds);
		elemBounds.minimum -= shift;
		elemBounds.maximum -= shift;
		encode(elemBounds);
	}

	PX_INLINE bool isInside(const IntegerAABB& box) const
	{
		if(box.mMinMax[MIN_X]>mMinMax[MIN_X])	return false;
		if(box.mMinMax[MIN_Y]>mMinMax[MIN_Y])	return false;
		if(box.mMinMax[MIN_Z]>mMinMax[MIN_Z])	return false;
		if(box.mMinMax[MAX_X]<mMinMax[MAX_X])	return false;
		if(box.mMinMax[MAX_Y]<mMinMax[MAX_Y])	return false;
		if(box.mMinMax[MAX_Z]<mMinMax[MAX_Z])	return false;
		return true;
	}

	PX_FORCE_INLINE bool intersects(const IntegerAABB& b) const
	{
		return !(b.mMinMax[MIN_X] > mMinMax[MAX_X] || mMinMax[MIN_X] > b.mMinMax[MAX_X] ||
			     b.mMinMax[MIN_Y] > mMinMax[MAX_Y] || mMinMax[MIN_Y] > b.mMinMax[MAX_Y] ||
			     b.mMinMax[MIN_Z] > mMinMax[MAX_Z] || mMinMax[MIN_Z] > b.mMinMax[MAX_Z]);
	}

	PX_FORCE_INLINE bool intersectsOrTouches(const IntegerAABB& b) const
	{
		return !(b.mMinMax[MIN_X] >= mMinMax[MAX_X] || mMinMax[MIN_X] >= b.mMinMax[MAX_X] ||
			b.mMinMax[MIN_Y] >= mMinMax[MAX_Y] || mMinMax[MIN_Y] >= b.mMinMax[MAX_Y] ||
			b.mMinMax[MIN_Z] >= mMinMax[MAX_Z] || mMinMax[MIN_Z] >= b.mMinMax[MAX_Z]);
	}


	PX_FORCE_INLINE void include(const IntegerAABB& b)
	{
		mMinMax[MIN_X] = PxMin(mMinMax[MIN_X], b.mMinMax[MIN_X]);
		mMinMax[MIN_Y] = PxMin(mMinMax[MIN_Y], b.mMinMax[MIN_Y]);
		mMinMax[MIN_Z] = PxMin(mMinMax[MIN_Z], b.mMinMax[MIN_Z]);
		mMinMax[MAX_X] = PxMax(mMinMax[MAX_X], b.mMinMax[MAX_X]);
		mMinMax[MAX_Y] = PxMax(mMinMax[MAX_Y], b.mMinMax[MAX_Y]);
		mMinMax[MAX_Z] = PxMax(mMinMax[MAX_Z], b.mMinMax[MAX_Z]);
	}

	PX_INLINE void setEmpty()
	{
		mMinMax[MIN_X] = mMinMax[MIN_Y] = mMinMax[MIN_Z] = 0xff7fffff;  //PX_IR(PX_MAX_F32);
		mMinMax[MAX_X] = mMinMax[MAX_Y] = mMinMax[MAX_Z] = 0x00800000;	///PX_IR(0.0f);
	}

	PxcBPValType mMinMax[6];
};

//////////////////////////////////////////////////////////////

struct PxcBroadPhasePair
{
	PxcBroadPhasePair(PxcBpHandle volA, PxcBpHandle volB)
	{
		mVolA=PxMin(volA,volB);
		mVolB=PxMax(volA,volB);
	}
	PxcBroadPhasePair()
		: mVolA(PX_INVALID_BP_HANDLE),
		  mVolB(PX_INVALID_BP_HANDLE)
	{
	}

	PxcBpHandle		mVolA;		// NB: mVolA < mVolB
	PxcBpHandle		mVolB;
};

//////////////////////////////////////////////////////////////

struct PxvBroadPhaseOverlap
{
	void* userdata0;
	void* userdata1;
};

//////////////////////////////////////////////////////////////

class PxcBroadPhaseUpdateData
{
public:

     /**

       \brief A structure detailing the changes to the collection of aabbs, whose overlaps are computed in the broadphase.
       The structure consists of per-object arrays of object bounds and object groups, and three arrays that index
       into the per-object arrays, denoting the bounds which are to be created, updated and removed in the broad phase.

       * each entry in the object arrays represents the same shape or aggregate from frame to frame.
       * each entry in an index array must be less than the capacity of the per-object arrays.
       * no index value may appear in more than one index array, and may not occur more than once in that array.

        An index value is said to be "in use" if it has appeared in a created list in a previous update, and has not
       since occurred in a removed list.

       \param[in] created: an array of indices describing the bounds that must be inserted into the broadphase.
       Each index in the array must not be in use.

       \param[in] updated: an array of indices (referencing the boxBounds and boxGroups arrays) describing the bounds
       that have moved since the last broadphase update. Each index in the array must be in use, and each object
	   whose index is in use and whose AABB has changed must appear in the update list.

       \param[in] removed: an array of indices describing the bounds that must be removed from the broad phase. Each index in
	   the array must be in use.

       \param[in] boxBounds: an array of bounds coordinates for the AABBs to be processed by the broadphase.

       An entry is valid if its values are integer bitwise representations of floating point numbers that satisfy max>min in each dimension,
	   along with a further rule that minima(maxima) must have even(odd) values. 

       Each entry whose index is either in use or appears in the created array must be valid. An entry whose index is either not in use or
       appears in the removed array need not be valid.

       \param[in]  boxGroups: an array of group ids, one for each bound, used for pair filtering.  Bounds with the same group id will not be
       reported as overlap pairs by the broad phase.  Zero is reserved for static bounds.

       Entries in this array are immutable: the only way to change the group of an object is to remove it from the broad phase and reinsert
       it at a different index (recall that each index must appear at most once in the created/updated/removed lists).

       \param[in]  boxesCapacity: the length of the boxBounds and boxGroups arrays.
    */
	PxcBroadPhaseUpdateData(
		const PxcBpHandle* created, const PxU32 createdSize, 
		const PxcBpHandle* updated, const PxU32 updatedSize, 
		const PxcBpHandle* removed, const PxU32 removedSize, 
		const IntegerAABB* boxBounds, const PxcBpHandle* boxGroups, const PxU32 boxesCapacity)
		: mCreated(created),
		  mCreatedSize(createdSize),
		  mUpdated(updated),
		  mUpdatedSize(updatedSize),
		  mRemoved(removed),
		  mRemovedSize(removedSize),
		  mBoxBounds(boxBounds),
		  mBoxGroups(boxGroups),
		  mBoxesCapacity(boxesCapacity)
	{
	}

	const PxcBpHandle* getCreatedHandles() const {return mCreated;}
	PxU32 getNumCreatedHandles() const {return mCreatedSize;}

	const PxcBpHandle* getUpdatedHandles() const {return mUpdated;}
	PxU32 getNumUpdatedHandles() const {return mUpdatedSize;}

	const PxcBpHandle* getRemovedHandles() const {return mRemoved;}
	PxU32 getNumRemovedHandles() const {return mRemovedSize;}

	const IntegerAABB* getAABBs() const {return mBoxBounds;}
	const PxcBpHandle* getGroups() const {return mBoxGroups;}
	PxU32 getCapacity() const {return mBoxesCapacity;}

#ifdef PX_CHECKED
	static bool isValid(const PxcBroadPhaseUpdateData& updateData, const PxvBroadPhase& bp);
#endif

private:

	const PxcBpHandle* mCreated;
	PxU32 mCreatedSize;

	const PxcBpHandle* mUpdated;
	PxU32 mUpdatedSize;

	const PxcBpHandle* mRemoved;
	PxU32 mRemovedSize;

	const IntegerAABB* mBoxBounds;
	const PxcBpHandle* mBoxGroups;
	PxU32 mBoxesCapacity;

#ifdef PX_CHECKED
	bool isValid() const;
#endif
};


} //physx

#endif //PXS_BROADPHASE_COMMON_H

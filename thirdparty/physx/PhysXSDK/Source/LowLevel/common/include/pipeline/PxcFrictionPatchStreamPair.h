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



#ifndef PXC_FRICTIONPATCHPOOL_H
#define PXC_FRICTIONPATCHPOOL_H

#include "PxvConfig.h"
#include "PxSimpleTypes.h"
#include "PsMutex.h"
#include "PxcFrictionPatch.h"
#include "PsArray.h"

// Each narrow phase thread has an input stream of friction patches from the
// previous frame and an output stream of friction patches which will be
// saved for next frame. The patches persist for exactly one frame at which
// point they get thrown away.


// There is a stream pair per thread. A contact callback reserves space
// for its friction patches and gets a cookie in return that can stash
// for next frame. Cookies are valid for one frame only.
//
// note that all friction patches reserved are guaranteed to be contiguous;
// this might turn out to be a bit inefficient if we often have a large
// number of friction patches

#include "PxcNpMemBlockPool.h"

namespace physx
{

class PxcFrictionPatchStreamPair
{
public:
	PxcFrictionPatchStreamPair(PxcNpMemBlockPool& blockPool);

	// reserve can fail and return null. Read should never fail
	template<class FrictionPatch>
	FrictionPatch*		reserve(const PxU32 size);

	template<class FrictionPatch>
	const FrictionPatch* findInputPatches(const PxU8* ptr) const;
	void					reset();

	PxcNpMemBlockPool& getBlockPool() { return mBlockPool;}
private:
	PxcNpMemBlockPool&	mBlockPool;
	PxcNpMemBlock*		mBlock;
	PxU32				mUsed;

	PxcFrictionPatchStreamPair& operator=(const PxcFrictionPatchStreamPair&);
};

// reserve can fail and return null. Read should never fail
template <class FrictionPatch>
FrictionPatch* PxcFrictionPatchStreamPair::reserve(const PxU32 size)
{
	if(size>PxcNpMemBlock::SIZE)
	{
		return ((FrictionPatch*)(-1));
	}

	PX_ASSERT(size <= PxcNpMemBlock::SIZE);

	FrictionPatch* ptr = NULL;

	if(mBlock == NULL || mUsed + size > PxcNpMemBlock::SIZE)
	{
		mBlock = mBlockPool.acquireFrictionBlock();
		mUsed = 0;
	}

	if(mBlock)
	{
		ptr = (FrictionPatch*)(mBlock->data+mUsed);
		mUsed += size;
	}

	return ptr;
}

template <class FrictionPatch>
const FrictionPatch* PxcFrictionPatchStreamPair::findInputPatches(const PxU8* ptr) const
{
	return reinterpret_cast<const FrictionPatch*>(ptr);
}
}

#endif

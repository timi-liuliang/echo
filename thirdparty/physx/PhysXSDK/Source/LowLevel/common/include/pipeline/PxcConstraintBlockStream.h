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



#ifndef PXC_CONSTRAINTBLOCKPOOL_H
#define PXC_CONSTRAINTBLOCKPOOL_H

#include "PxvConfig.h"
#include "PsArray.h"
#include "PsMutex.h"
#include "PxcNpMemBlockPool.h"

namespace physx
{

class PxsConstraintBlockManager
{
public:
	PxsConstraintBlockManager(PxcNpMemBlockPool & blockPool):
		mBlockPool(blockPool)
	{
	}


	PX_FORCE_INLINE	void reset()
	{
		mBlockPool.releaseConstraintBlocks(mTrackingArray);
	}


	PxcNpMemBlockArray			mTrackingArray;
	PxcNpMemBlockPool&			mBlockPool;

private:
	PxsConstraintBlockManager& operator=(const PxsConstraintBlockManager&);
};

class PxcConstraintBlockStream
{
	PX_NOCOPY(PxcConstraintBlockStream)
public:
	PxcConstraintBlockStream(PxcNpMemBlockPool & blockPool):
		mBlockPool(blockPool),
		mBlock(NULL),
		mUsed(0)
	{
	}

	PX_FORCE_INLINE	PxU8* reserve(PxU32 size, PxsConstraintBlockManager& manager)
										{
											size = (size+15)&~15;
											if(size>PxcNpMemBlock::SIZE)
												return mBlockPool.acquireExceptionalConstraintMemory(size);

											if(mBlock == NULL || size+mUsed>PxcNpMemBlock::SIZE)
											{
												mBlock = mBlockPool.acquireConstraintBlock(manager.mTrackingArray);
												PX_ASSERT(0==mBlock || mBlock->data == (PxU8*)mBlock);
												mUsed = size;
												return (PxU8*)mBlock;
											}
											PX_ASSERT(mBlock && mBlock->data == (PxU8*)mBlock);
											PxU8* PX_RESTRICT result = mBlock->data+mUsed;
											mUsed += size;
											return result;
										}

	PX_FORCE_INLINE	void				reset()
										{
											mBlock = NULL;
											mUsed = 0;
										}

	PX_FORCE_INLINE PxcNpMemBlockPool&	getMemBlockPool()
	{
		return mBlockPool;
	}

private:
			PxcNpMemBlockPool&			mBlockPool;
			PxcNpMemBlock*				mBlock;	// current constraint block
			PxU32						mUsed;	// number of bytes used in constraint block
			//Tracking peak allocations
			PxU32						mPeakUsed;
};

class PxcContactBlockStream
{
	PX_NOCOPY(PxcContactBlockStream)
public:
	PxcContactBlockStream(PxcNpMemBlockPool & blockPool):
		mBlockPool(blockPool),
		mBlock(NULL),
		mUsed(0)
	{
	}

	PX_FORCE_INLINE	PxU8* reserve(PxU32 size)
										{
											size = (size+15)&~15;

											if(size>PxcNpMemBlock::SIZE)
												return mBlockPool.acquireExceptionalConstraintMemory(size);

											PX_ASSERT(size <= PxcNpMemBlock::SIZE);

											if(mBlock == NULL || size+mUsed>PxcNpMemBlock::SIZE)
											{
												mBlock = mBlockPool.acquireContactBlock();
												PX_ASSERT(0==mBlock || mBlock->data == (PxU8*)mBlock);
												mUsed = size;
												return (PxU8*)mBlock;
											}
											PX_ASSERT(mBlock && mBlock->data == (PxU8*)mBlock);
											PxU8* PX_RESTRICT result = mBlock->data+mUsed;
											mUsed += size;
											return result;
										}

	PX_FORCE_INLINE	void				reset()
										{
											mBlock = NULL;
											mUsed = 0;
										}

	PX_FORCE_INLINE PxcNpMemBlockPool&	getMemBlockPool()
	{
		return mBlockPool;
	}

private:
			PxcNpMemBlockPool&			mBlockPool;
			PxcNpMemBlock*				mBlock;	// current constraint block
			PxU32						mUsed;	// number of bytes used in constraint block
};

}

#endif

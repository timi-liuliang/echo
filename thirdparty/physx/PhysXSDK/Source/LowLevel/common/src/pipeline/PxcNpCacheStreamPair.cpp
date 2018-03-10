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


#include "PxcNpCacheStreamPair.h"
#include "PsUserAllocated.h"
#include "PxcNpMemBlockPool.h"

using namespace physx;

void PxcNpCacheStreamPair::reset()
{
	mBlock = NULL;
	mUsed = 0;
}

PxcNpCacheStreamPair::PxcNpCacheStreamPair(PxcNpMemBlockPool& blockPool):
  mBlockPool(blockPool), mBlock(NULL), mUsed(0)
{
}

// reserve can fail and return null. Read should never fail
PxU8* PxcNpCacheStreamPair::reserve(PxU32 size)
{
	size = (size+15)&~15;

	if(size>PxcNpMemBlock::SIZE)
	{
		return (PxU8*)(-1);
	}

	if(mBlock == NULL || mUsed + size > PxcNpMemBlock::SIZE)
	{
		mBlock = mBlockPool.acquireNpCacheBlock();
		mUsed = 0;
	}

	PxU8* ptr;
	if(mBlock == NULL)
		ptr = 0;
	else
	{
		ptr = mBlock->data+mUsed;
		mUsed += size;
	}

	return ptr;
}


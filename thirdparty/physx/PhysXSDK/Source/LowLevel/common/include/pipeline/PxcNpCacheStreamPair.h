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



#ifndef PXC_NPCACHESTREAMPAIR_H
#define PXC_NPCACHESTREAMPAIR_H

#include "PxvConfig.h"
#include "PxSimpleTypes.h"
#include "PxcNpMemBlockPool.h"

namespace physx
{

static const PxU32 PXC_NPCACHE_BLOCK_SIZE = 16384;


struct PxcNpCacheStreamPair
{
public:
	PxcNpCacheStreamPair(PxcNpMemBlockPool& blockPool);

	// reserve can fail and return null.
	PxU8*					reserve(PxU32 byteCount);
	void					reset();
private:
	PxcNpMemBlockPool&	mBlockPool;
	PxU16				mBlockIndex;
	PxcNpMemBlock*		mBlock;
	PxU32				mUsed;
private:
	PxcNpCacheStreamPair& operator=(const PxcNpCacheStreamPair&);
};

}

#endif

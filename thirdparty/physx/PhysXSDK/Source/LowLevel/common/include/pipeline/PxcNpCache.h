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

#ifndef PXC_NPCACHE_H
#define PXC_NPCACHE_H

#include "PsIntrinsics.h"
#include "PxcNpCacheStreamPair.h"

#include "PsPool.h"
#ifdef __SPU__
#include "..\..\..\ps3\include\spu\SpuNpMemBlock.h"
#include "..\..\..\LowLevel\ps3\include\CellNarrowPhaseTask.h"
#else
#include "PsFoundation.h"
#endif
#include "GuContactMethodImpl.h"

namespace physx
{
struct PxcNpCache : Gu::Cache
{
	PxU32 pairData;
	PxU8* ptr;
	PxU16 size;

	PxcNpCache() 
	{
	}

	// legacy
	PX_FORCE_INLINE	PxU32	getPairData()			const	{	return pairData;	}
	PX_FORCE_INLINE	void	setPairData(PxU32 p)			{	pairData = p;		}
	PX_FORCE_INLINE	void	invalidate()					
	{
		pairData = 0;
		size = 0;
		ptr = 0;
		
		if(manifold)
		{
			if(isMultiManifold())
			{
				manifold = 1;
			}
			else
			{
				getManifold().clearManifold();
			}
		}
	}
};

#ifdef __SPU__
#if USE_SEPARATE_CACHE_RESOURCE_MANAGER_ON_SPU
extern SpuNpMemBlock gMemBlockCaches;
extern bool gNoNpCacheBlocksAvailable;
extern bool gBigNpCacheAllocation;
#else
extern SpuNpMemBlock gMemBlockFrictionPatches;
#endif
#endif

// the size has to be know on PPU as well, since when we fall back to PPU
// we don't want to write more data to cache than we can later read on SPU
#define SPU_CACHE_BUFFER_SIZE 1024

template <typename T>
void PxcNpCacheWrite(PxcNpCacheStreamPair& streams,
					 PxcNpCache& cache,
					 const T& payload,
					 PxU32 bytes, 
					 const PxU8* data)
{
	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	cache.size = (payloadSize + 4 + bytes + 0xF)&~0xF;

	// AP: we have to do this check both on PPU and SPU
	// because on one frame this object might get assigned to PPU while on the next it may end up on SPU
#ifdef PX_PS3
	if (cache.size >= SPU_CACHE_BUFFER_SIZE)
	{
		// although this function seems always preceded by PxcNpCacheWriteInitiate
		// do the check conservatively nonetheless
#ifdef __SPU__
		gBigNpCacheAllocation = true;
#else
		PX_WARN_ONCE(true,
			"Attempting to cache too much data for PS3 narrow phase. "
			"Either accept a performance penalty or simplify collision geometry.");
#endif
		// no room for this much data, invalidate the cache
		cache.ptr = NULL;
		return;
	}
#endif

#ifdef __SPU__
	uintptr_t ea;
	#if USE_SEPARATE_CACHE_RESOURCE_MANAGER_ON_SPU
	PxU8* ls=gMemBlockCaches.reserve(cache.size,&ea);
	#else
	PxU8* ls=gMemBlockFrictionPatches.reserve(cache.size,&ea);
	#endif
	cache.ptr = (PxU8*)ea;
	// no need to test for -1==ls since 
	// cache.size is already limited by SPU_CACHE_BUFFER_SIZE
	if(ea==NULL)
	{
		gNoNpCacheBlocksAvailable = true;
		return;
	}
#else
	PxU8* ls = streams.reserve(cache.size);
	cache.ptr = ls;
	if(ls==NULL || ((PxU8*)(-1))==ls)
	{
		if(ls==NULL)
		{
			PX_WARN_ONCE(true,
				"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
				"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
			return;
		}
		else
		{
			PX_WARN_ONCE(true,
				"Attempting to allocate more than 16K of contact data for a single contact pair in narrowphase. "
				"Either accept dropped contacts or simplify collision geometry.");
			cache.ptr = NULL;
			ls = NULL;
			return;
		}
	}
#endif

	*reinterpret_cast<T*>(ls) = payload;
	*reinterpret_cast<PxU32*>(ls+payloadSize) = bytes;
	// PT: memCopy already takes care of the cache internally, so no need to do it ourselves
	if(data)
		PxMemCopy(ls+payloadSize+sizeof(PxU32), data, bytes);
}


template <typename T>
PxU8* PxcNpCacheWriteInitiate(PxcNpCacheStreamPair& streams, PxcNpCache& cache, const T& payload, PxU32 bytes)
{
	PX_UNUSED(payload);

	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	cache.size = (payloadSize + 4 + bytes + 0xF)&~0xF;

	// AP: we have to do this check both on PPU and SPU
	// because on one frame this object might get assigned to PPU while on the next it may end up on SPU
#ifdef PX_PS3
	if (cache.size >= SPU_CACHE_BUFFER_SIZE)
	{
#ifdef __SPU__
		gBigNpCacheAllocation = true;
#else
		PX_WARN_ONCE(true,
			"Attempting to cache too much data for PS3 narrow phase. "
			"Either accept a performance penalty or simplify collision geometry.");
#endif
		// no room for this much data, invalidate the cache
		cache.ptr = NULL;
		return NULL;
	}
#endif

#ifdef __SPU__
	uintptr_t ea;
	#if USE_SEPARATE_CACHE_RESOURCE_MANAGER_ON_SPU
	PxU8* ls=gMemBlockCaches.reserve(cache.size,&ea);
	#else
	PxU8* ls=gMemBlockFrictionPatches.reserve(cache.size,&ea);
	#endif
	cache.ptr = (PxU8*)ea;
	// no need to test for -1==ls since 
	// cache.size is already limited by SPU_CACHE_BUFFER_SIZE
	if(ea==NULL)
	{
		gNoNpCacheBlocksAvailable = true;
		return NULL;
	}
#else
	PxU8* ls = streams.reserve(cache.size);
	cache.ptr = ls;
	if(NULL==ls || (PxU8*)(-1)==ls)
	{
		if(NULL==ls)
		{
			PX_WARN_ONCE(true,
				"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
				"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
		}
		else
		{
			PX_WARN_ONCE(true,
				"Attempting to allocate more than 16K of contact data for a single contact pair in narrowphase. "
				"Either accept dropped contacts or simplify collision geometry.");
			cache.ptr = NULL;
			ls = NULL;
		}
	}
#endif
	return ls;
}

template <typename T>
PX_FORCE_INLINE void PxcNpCacheWriteFinalize(PxU8* ls, const T& payload, PxU32 bytes, const PxU8* data)
{
	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	*reinterpret_cast<T*>(ls) = payload;
	*reinterpret_cast<PxU32*>(ls+payloadSize) = bytes;
	// PT: memCopy already takes care of the cache internally, so no need to do it ourselves
	if(data)
		PxMemCopy(ls+payloadSize+sizeof(PxU32), data, bytes);
}


template <typename T>
PX_FORCE_INLINE PxU8* PxcNpCacheRead(PxcNpCache& cache, T*& payload)
{
	PxU8* ls = cache.ptr;
	payload = reinterpret_cast<T*>(ls);
	const PxU32 payloadSize = (sizeof(T)+3)&~3;
	return reinterpret_cast<PxU8*>(ls+payloadSize+sizeof(PxU32));
}

template <typename T>
const PxU8* PxcNpCacheRead2(PxcNpCache& cache, T& payload, PxU32& bytes)
{
	const PxU8* ls = cache.ptr;
	if(ls==NULL)
	{
		bytes = 0;
		return NULL;
	}

	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
#ifdef __SPU__
	// the cached data is now prefetched in the NarrowPhase DMA pipeline
#endif
	payload = *reinterpret_cast<const T*>(ls);
	bytes = *reinterpret_cast<const PxU32*>(ls+payloadSize);
	PX_ASSERT(cache.size == ((payloadSize + 4 + bytes+0xF)&~0xF));
	return reinterpret_cast<const PxU8*>(ls+payloadSize+sizeof(PxU32));
}

}

#endif // #ifndef PXC_NPCACHE_H

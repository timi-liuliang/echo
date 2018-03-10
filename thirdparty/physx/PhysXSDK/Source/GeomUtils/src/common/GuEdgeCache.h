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

#ifndef GU_EDGECACHE_H
#define GU_EDGECACHE_H

#include "CmPhysXCommon.h"
#include "PsIntrinsics.h"
#include "PxMemory.h"
#include "PsHash.h"

namespace physx
{
namespace Gu
{
	class EdgeCache
	{
#define NUM_EDGES_IN_CACHE 64		//must be power of 2.	32 lines result in 10% extra work (due to cache misses), 64 lines in 6% extra work, 128 lines in 4%.
	public:
		EdgeCache()
		{
			PxMemZero(cacheLines, NUM_EDGES_IN_CACHE*sizeof(CacheLine));
		}

		PxU32 hash(PxU32 key)	const
		{
			return (NUM_EDGES_IN_CACHE - 1) & Ps::hash(key);		//Only a 16 bit hash would be needed here.
		}

		bool isInCache(PxU8 vertex0, PxU8 vertex1)
		{
			PX_ASSERT(vertex1 >= vertex0);
			PxU16 key = PxU16((vertex0 << 8) | vertex1);
			PxU32 h = hash(key);
			CacheLine& cl = cacheLines[h];
			if (cl.fullKey == key)
			{
				return true;
			}
			else	//cache the line now as it's about to be processed
			{
				cl.fullKey = key;
				return false;
			}
		}

	private:
		struct CacheLine
		{
			PxU16 fullKey;
		};
		CacheLine cacheLines[NUM_EDGES_IN_CACHE];
#undef NUM_EDGES_IN_CACHE
	};
}

}

#endif


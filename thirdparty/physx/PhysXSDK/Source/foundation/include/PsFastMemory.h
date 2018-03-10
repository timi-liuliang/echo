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


#ifndef PX_FOUNDATION_PSFASTMEMORY_H
#define PX_FOUNDATION_PSFASTMEMORY_H

#include "Ps.h"

namespace physx
{
namespace shdfnd
{
	PX_DEPRECATED PX_INLINE void fastMemzero(void* addr, size_t size)		
	{ 
		memset(addr, 0, size);	
	}

	PX_DEPRECATED PX_INLINE void* fastMemset(void* dest, int c, size_t count)
	{
		return memset(dest,c,count);
	}

	PX_DEPRECATED PX_INLINE void* fastMemcpy(void* dest, const void* src, size_t count)
	{
		return memcpy(dest,src,count);
	}

	PX_DEPRECATED PX_INLINE void* fastMemmove(void* dest, const void* src, size_t count)
	{
		return memmove(dest,src,count);
	}
	
	PX_DEPRECATED PX_INLINE void gatherStrided(const void* src, void* dst, PxU32 nbElem, PxU32 elemSize, PxU32 stride)
	{
		const PxU8* s = (const PxU8*)src;
		PxU8* d = (PxU8*)dst;
		while(nbElem--)
		{
			memcpy(d, s, elemSize);
			d += elemSize;
			s += stride;
		}
	}
} // namespace shdfnd
} // namespace physx

#endif


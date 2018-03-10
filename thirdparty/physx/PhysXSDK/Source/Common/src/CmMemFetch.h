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

#ifndef PX_PHYSICS_COMMON_MEMFETCH_H
#define PX_PHYSICS_COMMON_MEMFETCH_H

#include "PxMemory.h"

#ifdef __SPU__

#include "../ps3/CmPS3MemFetch.h"

#else

namespace physx
{
namespace Cm
{

#define pxPrintf printf

#if !defined(PX_P64)
	typedef unsigned int MemFetchPtr;
#else
	typedef PxU64 MemFetchPtr;
#endif

	struct MemFetchSmallBuffer {};
	struct MemFetchSmallBuffer16 {};
	template <typename T, int nb=1> struct MemFetchBufferA {}; // for aligned EA, pads to 16 bytes
	template <typename T, int nb=1> struct MemFetchBufferU {}; // for unaligned EA, adds 32 bytes of padding

	template <unsigned int T> class MemFetchGatherList
	{
	public:

		PX_FORCE_INLINE void init(){}
		PX_FORCE_INLINE void setSize(const unsigned int index, const unsigned int size){}
		PX_FORCE_INLINE unsigned int getSize(const unsigned int index) const {return 0;}
		PX_FORCE_INLINE void setEA(const unsigned int index, const unsigned int ea){}
		PX_FORCE_INLINE bool isValid(const unsigned int num) const  {return true;}
	};

	inline void memFetchWait(unsigned int) { }
	inline void memFetchWaitMask(unsigned int) { } 

	template<typename T> inline T	memFetch(MemFetchPtr ea, unsigned int /*channel*/)					{ return *(T*)ea; }

	template<typename T> inline T*	memFetchAsync(MemFetchPtr ea, unsigned int, MemFetchSmallBuffer&)	{ return (T*)ea; }
	template<typename T> inline T*	memFetchAsync(MemFetchPtr ea, unsigned int, MemFetchSmallBuffer16&)	{ return (T*)ea; }
	template<typename T> inline T*	memFetchAsync(void*, MemFetchPtr ea, unsigned int, unsigned int)	{ return (T*)ea; }

	template<typename T> inline T*	memFetchAsync(MemFetchPtr ea, unsigned int, MemFetchBufferA<T,1>&)	{ return (T*)ea; }
	template<typename T> inline T*	memFetchAsync(MemFetchPtr ea, unsigned int, MemFetchBufferU<T,1>&)	{ return (T*)ea; }
	template<typename T, int nb>
						 inline T*	memFetchAsync(MemFetchPtr ea, unsigned int, MemFetchBufferA<T,nb>&)	{ return (T*)ea; }
	template<typename T, int nb>
						 inline T*	memFetchAsync(MemFetchPtr ea, unsigned int, MemFetchBufferU<T,nb>&)	{ return (T*)ea; }

	inline void memFetchAlignedAsync(MemFetchPtr target, MemFetchPtr ea, unsigned int size, unsigned int)
	{
		PX_ASSERT((target & 0xF) == 0);
		PX_ASSERT((size & 0xF) == 0);
		PX_ASSERT((ea & 0xF) == 0);
		PxMemCopy((void*)target, (const void*)ea, size);
	}

	template<unsigned int N> inline void memFetchGatherListAsync(
		MemFetchPtr target, const MemFetchGatherList<N>& gatherList, unsigned int gatherListSize, unsigned int channel)
	{
	}

	template<typename T> inline void memStoreAsync(T* ls, MemFetchPtr ea, PxU32 channel, int count = 1)
	{
		PX_ASSERT((MemFetchPtr(ls) & 0xF) == 0);
		PX_ASSERT((sizeof(T) & 0xF) == 0);
		PX_ASSERT((ea & 0xF) == 0);
		PX_UNUSED(count);
		PX_UNUSED(channel);
		*((T*)ea) = *ls;
	}

	inline void memStoreWait(unsigned int channel) { memFetchWait(channel); }
}
} // namespace Cm
#endif

#endif

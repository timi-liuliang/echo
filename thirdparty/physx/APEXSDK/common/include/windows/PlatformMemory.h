/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PLATFORM_MEMORY_H
#define PLATFORM_MEMORY_H

#ifndef PX_WINDOWS
#error This is a windows specific file!
#endif

#define DUMP_MARKER 0

// This file is full of placeholders for functionality only implemented on some platforms

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{


namespace memory
{

template <PxU32 CachePageSize, PxU32 NumCachePages, PxU32 Padding>
class ReadCache
{
public:
	ReadCache(const void*, const void*, const char*)
	{
	}

	void WarmStartCache()
	{
	}

	PX_FORCE_INLINE void* GetMemoryAddress(void* eaAddress)
	{
		return eaAddress;
	}
};




template <PxU32 CachePageSize, PxU32 NumCachePages>
class ReadWriteCache
{
public:
	ReadWriteCache(void*, void*, PxU32, const char*)
	{
	}

	~ReadWriteCache()
	{
	}

	void WarmStartCache()
	{
	}

	PX_FORCE_INLINE void* GetMemoryAddress(void* eaAddress)
	{
		return eaAddress;
	}
};





//A double-buffered read/write buffer - reads in immediately but defers write-back until later
template <PxU32 MaxBufferSize>
class DBReadWriteBuffer
{
public:
	DBReadWriteBuffer(const void*, const void*, const PxU32, const PxU32, const char*)
	{
	}

	PX_FORCE_INLINE bool IsStomped()
	{
		return false;
	}

	PX_FORCE_INLINE void* GetLocalAddress(const void* eaAddress)
	{
		return (void*)eaAddress;
	}

	PX_FORCE_INLINE void WriteBackData()
	{
	}

	PX_FORCE_INLINE void WaitAllComplete()
	{
	}
};



struct MemBaseMarker
{
	MemBaseMarker(const char* /*name*/) {}
};

PX_FORCE_INLINE bool VerifyStack()
{
	return true;
}



template <PxU32 NumPrefetchPages, PxU32 PrefetchSize>
class Prefetcher
{
public:
	PX_FORCE_INLINE Prefetcher(PxU32*, void* /*pStartAddress*/, void* /*pEndAddress*/, const char* /*name*/)
	{
	}

	PX_FORCE_INLINE void* GetLocalAddress(void* eaAddress, const PxU32 /*size*/)
	{
		return eaAddress;
	}
};




template <PxU32 PrefetchSize>
class StreamReaderWriter
{
public:
	PX_FORCE_INLINE StreamReaderWriter(PxU32* /*tags*/, void* /*pStartAddress*/, void* /*pEndAddress*/, const char* /*name*/)
	{
	}

	PX_FORCE_INLINE void* GetLocalAddress(void* eaAddress, const PxU32 /*size*/)
	{
		return eaAddress;
	}
};



PX_FORCE_INLINE void* FetchAlloc(void* eaAddress, const PxU32 /*size*/)
{
	return eaAddress;
}


PX_FORCE_INLINE void* FetchAllocUnaligned(void* eaAddress, const PxU32 /*size*/)
{
	return eaAddress;
}


PX_FORCE_INLINE void* LargeFetchAllocUnaligned(void* eaAddress, const PxU32 /*size*/)
{
	return eaAddress;
}



} // namespace memory


} // namespace apex
} // namespace physx

#endif // PLATFORM_MEMORY_H

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


#ifndef PX_FOUNDATION_TEMP_ALLOCATOR
#define PX_FOUNDATION_TEMP_ALLOCATOR

#include "PsAllocator.h"

namespace physx
{
namespace shdfnd
{
	union TempAllocatorChunk
	{
		TempAllocatorChunk() : mNext(0) {}
		TempAllocatorChunk* mNext; // while chunk is free
		PxU32 mIndex; // while chunk is allocated
		PxU8 mPad[16]; // 16 byte aligned allocations
	};

	class PX_FOUNDATION_API TempAllocator : public Allocator
	{
	public:
		TempAllocator(const char* = 0) {}
		void* allocate(size_t size, const char* file, int line);
		void deallocate(void* ptr);
	};

} // namespace shdfnd
} // namespace physx

#endif

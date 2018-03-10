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


#ifndef PX_FOUNDATION_PSALIGNEDMALLOC_H
#define PX_FOUNDATION_PSALIGNEDMALLOC_H

#include "PsUserAllocated.h"

/*!
Allocate aligned memory.
Alignment must be a power of 2!
-- should be templated by a base allocator
*/

namespace physx
{
namespace shdfnd
{
	/**
	Allocator, which is used to access the global PxAllocatorCallback instance
	(used for dynamic data types template instantiation), which can align memory
	*/

	// SCS: AlignedMalloc with 3 params not found, seems not used on PC either
	// disabled for now to avoid GCC error

	template<PxU32 N, typename BaseAllocator = Allocator >
	class AlignedAllocator : public BaseAllocator
	{
	public:
		AlignedAllocator(const BaseAllocator& base = BaseAllocator()) 
		: BaseAllocator(base) {}

		void* allocate(size_t size, const char* file, int line)
		{
			size_t pad = N - 1 + sizeof(size_t); // store offset for delete.
			PxU8* base = (PxU8*)BaseAllocator::allocate(size+pad, file, line);
			if(!base)
				return NULL;

			PxU8* ptr = (PxU8*)(size_t(base + pad) & ~(size_t(N) - 1)); // aligned pointer, ensuring N is a size_t wide mask
			((size_t*)ptr)[-1] = size_t(ptr - base); // store offset

			return ptr;
		}
		void deallocate(void* ptr)
		{
			if(ptr == NULL)
				return;

			PxU8* base = ((PxU8*)ptr) - ((size_t*)ptr)[-1];
			BaseAllocator::deallocate(base);
		}
	};

} // namespace shdfnd
} // namespace physx

#endif

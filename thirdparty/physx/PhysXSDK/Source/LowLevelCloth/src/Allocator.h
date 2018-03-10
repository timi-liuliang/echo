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

#pragma once

#include "Types.h"
#include "PsArray.h"
#include "PsAllocator.h"
#include "PsAlignedMalloc.h"
#include <stddef.h>

namespace physx
{
namespace cloth
{

void* allocate(size_t);
void deallocate(void*);

/* templated typedefs for convenience */

template <typename T>
struct Vector
{
	typedef physx::shdfnd::Array<T, physx::shdfnd::Allocator> Type;
};

template <typename T, size_t alignment>
struct AlignedVector
{
	typedef physx::shdfnd::Array<T, physx::shdfnd::AlignedAllocator<alignment> > Type;
};

struct UserAllocated
{
	void* operator new(size_t n) { return allocate(n); }
	virtual ~UserAllocated() { deallocate(this); }
private:
	static void operator delete(void*) {}
};

} // namespace cloth

}

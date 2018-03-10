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


#ifndef PX_FOUNDATION_PSUSERALLOCATED_H
#define PX_FOUNDATION_PSUSERALLOCATED_H

#include "PsAllocator.h"
#include "PsAlignedMalloc.h"
#include "PsFoundation.h"

namespace physx
{
namespace shdfnd
{
    /**
    Provides new and delete using a UserAllocator.
    Guarantees that 'delete x;' uses the UserAllocator too.
    */
    class UserAllocated
    {
    public:

// PX_SERIALIZATION
		PX_INLINE void* operator new(size_t, void* address)
		{
			return address;
		}
//~PX_SERIALIZATION
		//Matching operator delete to the above operator new.  Don't ask me
		//how this makes any sense - Nuernberger.
		PX_INLINE void operator delete(void*,void*)
		{
		}

        template <typename Alloc>
        PX_INLINE void* operator new(size_t size, Alloc alloc, const char* fileName, int line)
        {
            return alloc.allocate(size, fileName, line);
        }
        template <typename Alloc>
        PX_INLINE void* operator new[](size_t size, Alloc alloc, const char* fileName, int line)
        {
            return alloc.allocate(size, fileName, line);
        }

        // placement delete
        template <typename Alloc>
        PX_INLINE void operator delete(void* ptr, Alloc alloc, const char* fileName, int line)
        {
			PX_UNUSED(fileName);
			PX_UNUSED(line);
			alloc.deallocate(ptr);
        }
        template <typename Alloc>
        PX_INLINE void operator delete[](void* ptr, Alloc alloc, const char* fileName, int line)
        {
			PX_UNUSED(fileName);
			PX_UNUSED(line);
			alloc.deallocate(ptr);
        }

        PX_INLINE void  operator delete(void* ptr)
        {
			Allocator().deallocate(ptr);
        }
        PX_INLINE void  operator delete[](void* ptr)
        {
			Allocator().deallocate(ptr);
        }
    };
} // namespace shdfnd
} // namespace physx

#endif

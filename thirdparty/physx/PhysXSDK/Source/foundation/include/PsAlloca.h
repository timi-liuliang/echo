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


#ifndef PX_FOUNDATION_PSSCOPEDMEMORY_H
#define PX_FOUNDATION_PSSCOPEDMEMORY_H

#include "PsTempAllocator.h"

namespace physx
{
namespace shdfnd
{
	template<typename T, typename Alloc = TempAllocator>
	class ScopedPointer : private Alloc
	{
	public:

		~ScopedPointer()
		{
			if(mOwned)
				Alloc::deallocate(mPointer);
		}

		operator T*() const 
		{
			return mPointer;
		}

		T* mPointer;
		bool mOwned;
	};

} // namespace shdfnd
} // namespace physx

/*! Stack allocation for \c count instances of \c type. Falling back to temp allocator if using more than 1kB. */
#ifdef __SPU__
#define PX_ALLOCA(var, type, count)	type* var = (type*)PxAlloca(sizeof(type)*(count))
#else
#define PX_ALLOCA(var, type, count)																		\
	physx::shdfnd::ScopedPointer<type> var;															\
	{																									\
		PxU32 size=sizeof(type)*(count);																\
		var.mOwned = size > 1024;																		\
		if(var.mOwned)																					\
			var.mPointer = (type*)physx::shdfnd::TempAllocator().allocate(size, __FILE__, __LINE__);	\
		else																							\
			var.mPointer = (type*)PxAlloca(size);														\
	}
#endif
#endif

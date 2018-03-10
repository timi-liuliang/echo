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


#ifndef PX_FOUNDATION_PSINLINEALLOCATOR_H
#define PX_FOUNDATION_PSINLINEALLOCATOR_H

#include "PsUserAllocated.h"

namespace physx
{
namespace shdfnd
{
	// this is used by the array class to allocate some space for a small number
	// of objects along with the metadata
	template<PxU32 N, typename BaseAllocator>
	class InlineAllocator : private BaseAllocator
	{
	public:
		InlineAllocator(const PxEMPTY& v) : BaseAllocator(v)	{}

		InlineAllocator(const BaseAllocator& alloc = BaseAllocator())
			: BaseAllocator(alloc), mBufferUsed(false)
		{}

		InlineAllocator(const InlineAllocator& aloc)
			: BaseAllocator(aloc), mBufferUsed(false)
		{}

		void* allocate(PxU32 size, const char* filename, int line)
		{
			if(!mBufferUsed && size<=N) 
			{
				mBufferUsed = true;
				return mBuffer;
			}
			return BaseAllocator::allocate(size, filename, line);
		}

		void deallocate(void* ptr)
		{
			if(ptr == mBuffer)
				mBufferUsed = false;
			else
				BaseAllocator::deallocate(ptr);
		}

		PX_FORCE_INLINE	PxU8*	getInlineBuffer()		{ return mBuffer;	  }
		PX_FORCE_INLINE	bool	isBufferUsed()	const	{ return mBufferUsed; }

	protected:
		PxU8 mBuffer[N];
		bool mBufferUsed;
	};
} // namespace shdfnd
} // namespace physx

#endif

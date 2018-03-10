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


#ifndef PX_FOUNDATION_PSINLINEARRAY_H
#define PX_FOUNDATION_PSINLINEARRAY_H

#include "PsArray.h"
#include "PsInlineAllocator.h"

namespace physx
{
namespace shdfnd
{

// needs some work from binary serialization
//#define INHERIT_INLINEARRAY_FROM_ARRAY 

#ifdef INHERIT_INLINEARRAY_FROM_ARRAY
	// array that pre-allocates for N elements
	template <typename T, PxU32 N, typename Alloc = typename AllocatorTraits<T>::Type>
	class InlineArray : public Array<T, Alloc> 
	{
	public:

		InlineArray(const PxEMPTY& v) : Array<T, Alloc>(v) {}

		InlineArray(const Alloc& alloc = Alloc()): 
			Array<T,Alloc>(reinterpret_cast<T*>(mInlineSpace), 0, N, alloc) {}

		PX_INLINE bool isInlined()	const
		{
			return mData == reinterpret_cast<T*>(mInlineSpace);
		}

		template<class Serializer>
		void exportExtraData(Serializer& stream)
		{
			if(!isInlined())
				Array<T, Alloc>::exportArray(stream, false);
		}

		PxU8* importExtraData(PxU8* address)
		{
			if(isInlined())
				this->mData = reinterpret_cast<T*>(mInlineSpace);
			else
				address = Array<T, Alloc>::importArray(address);
			return address;
		}
	protected:
		// T inlineSpace[N] requires T to have a default constructor
		PxU8 mInlineSpace[N*sizeof(T)];
	};

#else
	// array that pre-allocates for N elements
	template <typename T, PxU32 N, typename Alloc = typename AllocatorTraits<T>::Type>
	class InlineArray : public Array<T, InlineAllocator<N * sizeof(T), Alloc> >
	{
		typedef InlineAllocator<N * sizeof(T), Alloc> Allocator;
	public:

		InlineArray(const PxEMPTY& v) : Array<T, Allocator>(v) 
		{
			if(isInlined())
				this->mData = reinterpret_cast<T*>(Array<T, Allocator>::getInlineBuffer());
		}

		PX_INLINE bool isInlined()	const
		{
			return Allocator::isBufferUsed();
		}

		PX_INLINE explicit InlineArray(const Alloc& alloc = Alloc()) 
			: Array<T, Allocator>(alloc)
		{
			this->mData = this->allocate(N);
			this->mCapacity = N; 
		}
	};
#endif

} // namespace shdfnd
} // namespace physx

#endif

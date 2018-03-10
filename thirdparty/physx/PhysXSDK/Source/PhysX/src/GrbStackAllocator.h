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


#ifndef __GRB_STACK_ALLOC_H__
#define __GRB_STACK_ALLOC_H__

#include "PsArray.h"

#define GRB_INTEROP_RIGID_STATIC_DESC	1
#define GRB_INTEROP_RIGID_DYNAMIC_DESC	2

namespace physx
{
	
namespace GrbInterop3
{

class StackAllocator
{
	PX_NOCOPY(StackAllocator)

public:
	StackAllocator(PxU32 pageSize): mPageSize(pageSize)
	{
		grow(0);
	}

	~StackAllocator()
	{
		release();
	}

	void release()
	{
		for(shdfnd::Array<StackAllocatorPage>::Iterator it = mPages.begin(), endit = mPages.end(); it != endit; ++it)
		{
			Ps::getAllocator().deallocate(it->ptr);
		}

		mPages.reset();
	}

	void grow(size_t size)
	{
		size = PxMax<size_t>(size, mPageSize);
		mPages.pushBack(StackAllocatorPage(Ps::getAllocator().allocate(size, "GRB interop page", __FILE__, __LINE__), size));

		mCurrPage = mPages.size() - 1;
		mCurrPageByteSize = 0;
	}

	void * allocate(size_t size)
	{
		if(mCurrPageByteSize + size > mPages[mCurrPage].byteCapacity)
		{
			do
			{
				mCurrPage++; 
			}while(mCurrPage < mPages.size() && size > mPages[mCurrPage].byteCapacity);

			if(mCurrPage < mPages.size())
			{
				mCurrPageByteSize = 0;
			}
			else
			{
				grow(size);
			}
		}

		void *ret = reinterpret_cast<void *>(reinterpret_cast<PxU8 *>(mPages[mCurrPage].ptr) + mCurrPageByteSize); 
		mCurrPageByteSize += size;

		return ret;
	}

	void deallocate(void *)
	{
	// do nothing
	}

	void reset()
	{
		mCurrPageByteSize = 0;
		mCurrPage = 0;
	}

protected:
	struct StackAllocatorPage
	{
		StackAllocatorPage() : ptr(0), byteCapacity(0)
		{
		}

		StackAllocatorPage(void *p, size_t s): ptr(p), byteCapacity(s)
		{
		}
	
		void *ptr;
		size_t byteCapacity;
	};

	shdfnd::Array<StackAllocatorPage> mPages;
	PxU32						mCurrPage;
	size_t						mCurrPageByteSize;
	const PxU32					mPageSize;
};


template<typename T>
struct GrbInteropEvent3Pool 
{
	~GrbInteropEvent3Pool<T>()
	{
		while( eventCache.size() != 0 )
		{
			PX_DELETE_AND_RESET(eventCache.back());
			eventCache.popBack();
		}
	}

	T*	getEvent()
	{
		for( T** i = eventCache.begin(); i < eventCache.end(); ++i )
		{
			T* e = *i;
			if( e->isEmpty() )
			{	// we may use this one
				return e;
			}
		}

		T* e = PX_NEW(T)();
		eventCache.pushBack( e );
		return e;
	}

private:

	shdfnd::Array<T*>	eventCache;
};

}
}

#endif

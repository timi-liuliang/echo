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


#ifndef PXC_SCRATCHALLOCATOR_H
#define PXC_SCRATCHALLOCATOR_H

#include "PxvConfig.h"
#include "PsMutex.h"
#include "PxAssert.h"
#include "PsArray.h"


namespace physx
{
class PxcScratchAllocator
{
	PX_NOCOPY(PxcScratchAllocator)
public:
	PxcScratchAllocator() : mStart(NULL), mSize(0)
	{
		mStack.reserve(64);
		mStack.pushBack(0);
	}

	void setBlock(void* addr, PxU32 size)
	{
		// if the stack is not empty then some scratch memory was not freed on the previous frame. That's 
		// likely indicative of a problem, because when the scratch block is too small the memory will have
		// come from the heap

		PX_ASSERT(mStack.size()==1);
		mStack.popBack();

		mStart = reinterpret_cast<PxU8*>(addr);
		mSize = size;
		mStack.pushBack(mStart + size);
	}

	void* allocAll(PxU32& size)
	{
		Ps::Mutex::ScopedLock lock(mLock);
		PX_ASSERT(mStack.size()>0);
		size = PxU32(mStack.back()-mStart);

		if(size==0)
			return NULL;

		mStack.pushBack(mStart);
		return mStart;
	}


	void* alloc(PxU32 requestedSize, bool fallBackToHeap = false)
	{
		requestedSize = (requestedSize+15)&~15;

		Ps::Mutex::ScopedLock lock(mLock);
		PX_ASSERT(mStack.size()>=1);

		PxU8* top = mStack.back();

		if(top - mStart >= ptrdiff_t(requestedSize))
		{
			PxU8* addr = top - requestedSize;
			mStack.pushBack(addr);
			return addr;
		}

		if(!fallBackToHeap)
			return NULL;

		return PX_ALLOC(requestedSize, "Scratch Block Fallback");
	}

	void free(void* addr)
	{
		PX_ASSERT(addr!=NULL);
		if(!isScratchAddr(addr))
		{
			PX_FREE(addr);
			return;
		}

		Ps::Mutex::ScopedLock lock(mLock);
		PX_ASSERT(mStack.size()>1);

		PxU32 i=mStack.size()-1;		
		while(mStack[i]<addr)
			i--;

		PX_ASSERT(mStack[i]==addr);
		mStack.remove(i);
	}


	bool isScratchAddr(void* addr) const
	{
		PxU8* a = reinterpret_cast<PxU8*>(addr);
		return a>= mStart && a<mStart+mSize;
	}

private:
	Ps::Mutex			mLock;
	Ps::Array<PxU8*>	mStack;
	PxU8*				mStart;
	PxU32				mSize;
};

}

#endif

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


#ifndef PX_PHYSICS_COMMON_QUEUE
#define PX_PHYSICS_COMMON_QUEUE

#include "PsAllocator.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PxAssert.h"

namespace physx
{
namespace Cm
{

	template<class T, class AllocType = Ps::Allocator >
	class Queue: public Ps::UserAllocated
	{
	public:
		Queue(PxU32 maxEntries);
		~Queue();

		T 		popFront();
		T 		front();
		T 		popBack();
		T 		back();
		bool	pushBack(const T& element);
		bool	empty() const;
		PxU32	size() const;

	private:
		T*			mJobQueue;
		PxU32		mNum;
		PxU32		mHead;
		PxU32		mTail;
		PxU32		mMaxEntries;
		AllocType	mAllocator;
	};

	template<class T, class AllocType>
	Queue<T, AllocType>::Queue(PxU32 maxEntries):
		mNum(0),
		mHead(0),
		mTail(0),
		mMaxEntries(maxEntries)
	{
		mJobQueue = reinterpret_cast<T*>(mAllocator.allocate(sizeof(T)*mMaxEntries, __FILE__, __LINE__));
	}

	template<class T, class AllocType>
	Queue<T, AllocType>::~Queue()
	{
		if(mJobQueue)
			mAllocator.deallocate(mJobQueue);
	}

	template<class T, class AllocType>
	T Queue<T, AllocType>::popFront()
	{
		PX_ASSERT(mNum>0);

		mNum--;
		T& element = mJobQueue[mTail];
		mTail = (mTail+1) % (mMaxEntries);
		return element;
	}

	template<class T, class AllocType>
	T Queue<T, AllocType>::front()
	{
		PX_ASSERT(mNum>0);

		return mJobQueue[mTail];
	}

	template<class T, class AllocType>
	T Queue<T, AllocType>::popBack()
	{
		PX_ASSERT(mNum>0);

		mNum--;
		mHead = (mHead-1) % (mMaxEntries);
		return mJobQueue[mHead];
	}

	template<class T, class AllocType>
	T Queue<T, AllocType>::back()
	{
		PX_ASSERT(mNum>0);

		PxU32 headAccess = (mHead-1) % (mMaxEntries);
		return mJobQueue[headAccess];
	}

	template<class T, class AllocType>
	bool Queue<T, AllocType>::pushBack(const T& element)
	{
		if (mNum == mMaxEntries) return false;
		mJobQueue[mHead] = element;

		mNum++;
		mHead = (mHead+1) % (mMaxEntries);

		return true;
	}

	template<class T, class AllocType>
	bool Queue<T, AllocType>::empty() const
	{
		return mNum == 0;
	}

	template<class T, class AllocType>
	PxU32 Queue<T, AllocType>::size() const
	{
		return mNum;
	}


} // namespace Cm

}

#endif

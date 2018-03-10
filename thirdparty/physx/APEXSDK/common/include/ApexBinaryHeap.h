/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_BINARY_HEAP_H
#define APEX_BINARY_HEAP_H

#include "NxApexDefs.h"

namespace physx
{
namespace apex
{

template <class Comparable>
class ApexBinaryHeap
{
public:
	explicit ApexBinaryHeap(int capacity = 100) : mCurrentSize(0)
	{
		if (capacity > 0)
		{
			mArray.reserve((physx::PxU32)capacity + 1);
		}

		mArray.insert();
	}



	bool isEmpty() const
	{
		return mCurrentSize == 0;
	}



	const Comparable& peek() const
	{
		PX_ASSERT(mArray.size() > 1);
		return mArray[1];
	}



	void push(const Comparable& x)
	{
		mArray.insert();
		// Percolate up
		mCurrentSize++;
		physx::PxU32 hole = mCurrentSize;
		while (hole > 1)
		{
			physx::PxU32 parent = hole >> 1;
			if (!(x < mArray[parent]))
			{
				break;
			}
			mArray[hole] = mArray[parent];
			hole = parent;
		}
		mArray[hole] = x;
	}



	void pop()
	{
		if (mArray.size() > 1)
		{
			mArray[1] = mArray[mCurrentSize--];
			percolateDown(1);
			mArray.popBack();
		}
	}



	void pop(Comparable& minItem)
	{
		if (mArray.size() > 1)
		{
			minItem = mArray[1];
			mArray[1] = mArray[mCurrentSize--];
			percolateDown(1);
			mArray.popBack();
		}
	}

private:
	physx::PxU32 mCurrentSize;  // Number of elements in heap
	physx::Array<Comparable> mArray;

	void buildHeap()
	{
		for (physx::PxU32 i = mCurrentSize / 2; i > 0; i--)
		{
			percolateDown(i);
		}
	}



	void percolateDown(physx::PxU32 hole)
	{
		Comparable tmp = mArray[hole];

		while (hole * 2 <= mCurrentSize)
		{
			physx::PxU32 child = hole * 2;
			if (child != mCurrentSize && mArray[child + 1] < mArray[child])
			{
				child++;
			}
			if (mArray[child] < tmp)
			{
				mArray[hole] = mArray[child];
			}
			else
			{
				break;
			}

			hole = child;
		}

		mArray[hole] = tmp;
	}
};

}
} // end namespace physx::apex

#endif // APEX_BINARY_HEAP_H

/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_FIFO_H__
#define __APEX_FIFO_H__

#include "NxApex.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace apex
{

template <typename T>
struct FIFOEntry
{
	T data;
	physx::PxU32 next;
	bool isValidEntry;
};

template<typename T>
class ApexFIFO : public physx::UserAllocated
{
public:
	ApexFIFO() : first((physx::PxU32) - 1), last((physx::PxU32) - 1), count(0) {}

	bool popFront(T& frontElement)
	{
		if (first == (physx::PxU32)-1)
		{
			return false;
		}

		PX_ASSERT(first < list.size());
		frontElement = list[first].data;

		if (first == last)
		{
			list.clear();
			first = (physx::PxU32) - 1;
			last = (physx::PxU32) - 1;
		}
		else
		{
			list[first].isValidEntry = false;

			if (list[last].next == (physx::PxU32)-1)
			{
				list[last].next = first;
			}
			first = list[first].next;
		}

		count--;
		return true;
	}


	void pushBack(const T& newElement)
	{
		if (list.size() == 0 || list[last].next == (physx::PxU32)-1)
		{
			FIFOEntry<T> newEntry;
			newEntry.data = newElement;
			newEntry.next = (physx::PxU32) - 1;
			newEntry.isValidEntry = true;
			list.pushBack(newEntry);

			if (first == (physx::PxU32) - 1)
			{
				PX_ASSERT(last == (physx::PxU32) - 1);
				first = list.size() - 1;
			}
			else
			{
				PX_ASSERT(last != (physx::PxU32) - 1);
				list[last].next = list.size() - 1;
			}

			last = list.size() - 1;
		}
		else
		{
			physx::PxU32 freeIndex = list[last].next;
			PX_ASSERT(freeIndex < list.size());

			FIFOEntry<T>& freeEntry = list[freeIndex];
			freeEntry.data = newElement;
			freeEntry.isValidEntry = true;

			if (freeEntry.next == first)
			{
				freeEntry.next = (physx::PxU32) - 1;
			}

			last = freeIndex;
		}
		count++;
	}

	physx::PxU32 size()
	{
		return count;
	}

	PX_INLINE void reserve(const physx::PxU32 capacity)
	{
		list.reserve(capacity);
	}

	PX_INLINE physx::PxU32 capacity() const
	{
		return list.capacity();
	}

private:
	physx::PxU32 first;
	physx::PxU32 last;
	physx::PxU32 count;
	physx::Array<FIFOEntry<T> > list;
};

}
} // end namespace physx::apex

#endif
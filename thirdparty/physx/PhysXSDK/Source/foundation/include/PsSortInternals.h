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


#ifndef PX_FOUNDATION_PSSORTINTERNALS_H
#define PX_FOUNDATION_PSSORTINTERNALS_H

/** \addtogroup foundation
@{
*/

#include "foundation/PxAssert.h"
#include "foundation/PxMemory.h"
#include "PsUtilities.h"
#include "PsUserAllocated.h"

#if !defined(PX_GNUC) && !defined(PX_GHS)
#pragma warning(push)
#pragma warning(disable:4127) // Disable the conditional is constant warning for this header, it is used intentionally
#endif

namespace physx
{
namespace shdfnd
{
	namespace internal
	{
		template<class T, class Predicate>
		PX_INLINE void median3(T *elements, PxI32 first, PxI32 last, Predicate &compare)
		{
			/*
			This creates sentinels because we know there is an element at the start minimum(or equal) 
			than the pivot and an element at the end greater(or equal) than the pivot. Plus the 
			median of 3 reduces the chance of degenerate behavour.
			*/

			PxI32 mid = (first + last)/2;

			if(compare(elements[mid], elements[first]))
				swap(elements[first], elements[mid]);

			if(compare(elements[last], elements[first]))
				swap(elements[first], elements[last]);

			if(compare(elements[last], elements[mid]))
				swap(elements[mid], elements[last]);

			//keep the pivot at last-1
			swap(elements[mid], elements[last-1]);
		}

		template<class T, class Predicate>
		PX_INLINE PxI32 partition(T *elements, PxI32 first, PxI32 last, Predicate &compare)
		{
			median3(elements, first, last, compare);

			/*
			WARNING: using the line:

			T partValue = elements[last-1];

			and changing the scan loops to:

			while(comparator.greater(partValue, elements[++i]));
			while(comparator.greater(elements[--j], partValue);

			triggers a compiler optimizer bug on xenon where it stores a double to the stack for partValue
			then loads it as a single...:-(
			*/

			PxI32 i = first;		//we know first is less than pivot(but i gets pre incremented) 
			PxI32 j = last - 1;		//pivot is in last-1 (but j gets pre decremented)

			while(true)
			{
				while(compare(elements[++i], elements[last-1]));
				while(compare(elements[last-1], elements[--j]));

				if(i>=j) break;

				PX_ASSERT(i<=last && j>=first);
				swap(elements[i], elements[j]);
			}
			//put the pivot in place

			PX_ASSERT(i<=last && first<=(last-1));
			swap(elements[i], elements[last-1]);

			return i;
		}

		template<class T, class Predicate>
		PX_INLINE void smallSort(T *elements, PxI32 first, PxI32 last, Predicate &compare)
		{
			//selection sort - could reduce to fsel on 360 with floats. 

			for(PxI32 i=first; i<last; i++)
			{
				PxI32 m = i;
				for(PxI32 j=i+1; j<=last; j++)
					if(compare(elements[j], elements[m])) m = j;

				if(m!=i)
					swap(elements[m], elements[i]);
			}
		}

		template<class Allocator>
		class Stack
		{
			Allocator mAllocator;
			PxU32 mSize, mCapacity;
			PxI32 *mMemory;
			bool mRealloc;
		public:
			Stack(PxI32 *memory, PxU32 capacity, const Allocator& inAllocator )
					: mAllocator( inAllocator ), mSize(0), mCapacity(capacity), mMemory(memory), mRealloc(false)
					 {}
			~Stack()
			{
				if(mRealloc) 
					mAllocator.deallocate(mMemory);
			}

			void grow()
			{
				mCapacity *=2;
				PxI32 *newMem = (PxI32 *)mAllocator.allocate(sizeof(PxI32)*mCapacity, __FILE__, __LINE__ );
				PxMemCopy(newMem,mMemory,mSize*sizeof(PxI32));
				if(mRealloc) 
					mAllocator.deallocate(mMemory);
				mRealloc = true;
				mMemory = newMem;
			}

			PX_INLINE void push(PxI32 start, PxI32 end) 
			{ 
				if(mSize >= mCapacity-1)
					grow();
				mMemory[mSize++] = start;
				mMemory[mSize++] = end;
			}

			PX_INLINE void pop(PxI32 &start, PxI32 &end)
			{
				PX_ASSERT(!empty());
				end = mMemory[--mSize];
				start = mMemory[--mSize];
			}

			PX_INLINE bool empty()
			{
				return mSize == 0;
			}
		};
	} // namespace internal

} // namespace shdfnd
} // namespace physx


#if defined(PX_WINDOWS) || defined(PX_XBOXONE)
#pragma warning(pop)
#endif

#endif

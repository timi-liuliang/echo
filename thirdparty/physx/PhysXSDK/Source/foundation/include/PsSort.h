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


#ifndef PX_FOUNDATION_PSSORT_H
#define PX_FOUNDATION_PSSORT_H

/** \addtogroup foundation
@{
*/

#include "PsSortInternals.h"
#include "PsAlloca.h"

#ifdef PX_DEBUG
#define PX_SORT_PARANOIA
#endif

/**
\brief Sorts an array of objects in ascending order, assuming
that the predicate implements the < operator:

\see Less, Greater
*/


#if !defined(PX_GNUC) && !defined(PX_GHS)
#pragma warning(push)
#pragma warning(disable:4706) // disable the warning that we did an assignment within a conditional expression, as this was intentional.
#pragma warning(disable:4127) // disable the conditional expression is constant warning for this header, it is used intentionally 
#endif

namespace physx
{
namespace shdfnd
{
	template<class T, class Predicate, class Allocator>
	void sort(T* elements, PxU32 count, const Predicate& compare, const Allocator& inAllocator, const PxU32 initialStackSize = 32)
	{
		static const PxU32 SMALL_SORT_CUTOFF = 5; // must be >= 3 since we need 3 for median

		PX_ALLOCA(stackMem, PxI32, initialStackSize);
		internal::Stack<Allocator> stack(stackMem, initialStackSize, inAllocator);

		PxI32 first = 0, last = PxI32(count-1);
		if(last > first)
		{
			while(true)
			{
				while(last > first)
				{
					PX_ASSERT(first >= 0 && last < (PxI32)count);
					if(PxU32(last-first) < SMALL_SORT_CUTOFF)
					{
						internal::smallSort(elements, first, last, compare);
						break;
					}
					else
					{
						const PxI32 partIndex = internal::partition(elements, first, last, compare);

						// push smaller sublist to minimize stack usage
						if((partIndex - first) < (last - partIndex)) 
						{
							stack.push(first, partIndex-1);
							first = partIndex + 1;
						}
						else
						{
							stack.push(partIndex+1, last);
							last = partIndex - 1;
						}
					}
				}

				if(stack.empty())
					break;

				stack.pop(first, last);
			}
		}
#ifdef PX_SORT_PARANOIA
		for(PxU32 i=1; i<count; i++)
			PX_ASSERT(!compare(elements[i],elements[i-1]));
#endif

	}

	template<class T, class Predicate>
	void sort(T* elements, PxU32 count, const Predicate& compare)
	{
		sort(elements, count, compare, typename shdfnd::AllocatorTraits<T>::Type());
	}

	template<class T>
	void sort(T* elements, PxU32 count)
	{
		sort(elements, count, shdfnd::Less<T>(),
			typename shdfnd::AllocatorTraits<T>::Type());
	}


} // namespace shdfnd
} // namespace physx

#if !defined(PX_GNUC) && !defined(PX_GHS)
#pragma warning(pop)
#endif

#endif

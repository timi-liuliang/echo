/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_FIND_H
#define APEX_FIND_H

namespace physx
{
namespace apex
{
	// binary search
	template<class Sortable>
	PxI32 ApexFind(const Sortable* buffer, PxU32 numEntries, const Sortable& element, int (*compare)(const void*, const void*))
	{

#if defined PX_CHECKED
		if (numEntries > 0)
		{
			for (PxU32 i = 1; i < numEntries; ++i)
			{
				PX_ASSERT(compare(buffer + i - 1, buffer + i) <= 0);
			}
		}
#endif

		PxI32 curMin = 0;
		PxI32 curMax = (physx::PxI32)numEntries;
		PxI32 testIndex = 0;

		while (curMin < curMax)
		{
			testIndex = (curMin + curMax) / 2;
			PxI32 compResult = compare(&element, buffer+testIndex);
			if (compResult < 0)
			{
				curMax = testIndex;
			}
			else if (compResult > 0)
			{
				curMin = testIndex;
			}
			else
			{
				return testIndex;
			}

		}

		return -1;
	}

} // namespace apex
} // namespace physx

#endif // APEX_FIND_H

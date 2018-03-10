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

#ifndef QUICKSELECT_H
#define QUICKSELECT_H

#include "PxSimpleTypes.h"

// Google "wikipedia QuickSelect" for algorithm explanation
namespace physx { namespace quickSelect {


	#define SWAP32(x, y) { PxU32 tmp = y; y = x; x = tmp; }

	// left is the index of the leftmost element of the subarray
	// right is the index of the rightmost element of the subarray (inclusive)
	// number of elements in subarray = right-left+1
	template<typename LtEq>
	PxU32 partition(PxU32* PX_RESTRICT a, PxU32 left, PxU32 right, PxU32 pivotIndex, const LtEq& cmpLtEq)
	{
		PX_ASSERT(pivotIndex >= left && pivotIndex <= right);
		PxU32 pivotValue = a[pivotIndex];
		SWAP32(a[pivotIndex], a[right]) // Move pivot to end
		PxU32 storeIndex = left;
		for (PxU32 i = left; i < right; i++)  // left <= i < right
			if (cmpLtEq(a[i], pivotValue))
			{
				SWAP32(a[i], a[storeIndex]);
				storeIndex++;
			}
		SWAP32(a[storeIndex], a[right]); // Move pivot to its final place
		for (PxU32 i = left; i < storeIndex; i++)
			PX_ASSERT(cmpLtEq(a[i], a[storeIndex]));
		for (PxU32 i = storeIndex+1; i <= right; i++)
			PX_ASSERT(cmpLtEq(a[storeIndex], a[i]));
		return storeIndex;
	}

	// left is the index of the leftmost element of the subarray
	// right is the index of the rightmost element of the subarray (inclusive)
	// number of elements in subarray = right-left+1
	// recursive version
	template<typename LtEq>
	void quickFindFirstK(PxU32* PX_RESTRICT a, PxU32 left, PxU32 right, PxU32 k, const LtEq& cmpLtEq)
	{
		PX_ASSERT(k <= right-left+1);
		if (right > left)
		{
			// select pivotIndex between left and right
			PxU32 pivotIndex = (left + right) >> 1;
			PxU32 pivotNewIndex = partition(a, left, right, pivotIndex, cmpLtEq);
			// now all elements to the left of pivotNewIndex are < old value of a[pivotIndex] (bottom half values)
			if (pivotNewIndex > left + k) // new condition
				quickFindFirstK(a, left, pivotNewIndex-1, k, cmpLtEq);
			if (pivotNewIndex < left + k)
				quickFindFirstK(a, pivotNewIndex+1, right, k+left-pivotNewIndex-1, cmpLtEq);
		}
	}

	// non-recursive version
	template<typename LtEq>
	void quickSelectFirstK(PxU32* PX_RESTRICT a, PxU32 left, PxU32 right, PxU32 k, const LtEq& cmpLtEq)
	{
		PX_ASSERT(k <= right-left+1);
		for (;;)
		{
			PxU32 pivotIndex = (left+right) >> 1;
			PxU32 pivotNewIndex = partition(a, left, right, pivotIndex, cmpLtEq);
			PxU32 pivotDist = pivotNewIndex - left + 1;
			if (pivotDist == k)
				return;
			else if (k < pivotDist)
			{
				PX_ASSERT(pivotNewIndex > 0);
				right = pivotNewIndex - 1;
			}
			else
			{
				k = k - pivotDist;
				left = pivotNewIndex+1;
			}
		}
	}

} }  // namespace quickSelect, physx

#endif


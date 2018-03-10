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


#ifndef PX_FOUNDATION_PSBITUTILS_H
#define PX_FOUNDATION_PSBITUTILS_H

#include "foundation/PxIntrinsics.h"
#include "PsIntrinsics.h"
#include "foundation/PxAssert.h"
#include "Ps.h"

namespace physx
{
namespace shdfnd
{
	PX_INLINE PxU32 bitCount(PxU32 v)
	{
		// from http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		PxU32 const w = v - ((v >> 1) & 0x55555555);
		PxU32 const x = (w & 0x33333333) + ((w >> 2) & 0x33333333);
		return (((x + (x >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
	}
	
	PX_INLINE bool isPowerOfTwo(PxU32 x)
	{
		return x!=0 && (x & (x-1)) == 0;
	}

	// "Next Largest Power of 2
	// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
	// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
	// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
	// largest power of 2. For a 32-bit value:"
	PX_INLINE PxU32 nextPowerOfTwo(PxU32 x)
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x+1;
	}

	/*!
	Return the index of the highest set bit. Not valid for zero arg.
	*/

	PX_INLINE PxU32 lowestSetBit(PxU32 x)
	{
		PX_ASSERT(x);
		return lowestSetBitUnsafe(x);
	}

	/*!
	Return the index of the highest set bit. Not valid for zero arg.
	*/

	PX_INLINE PxU32 highestSetBit(PxU32 x)
	{
		PX_ASSERT(x);
		return highestSetBitUnsafe(x);
	}


	// Helper function to approximate log2 of an integer value 
	// assumes that the input is actually power of two.
	// todo: replace 2 usages with 'highestSetBit'
	PX_INLINE PxU32 ilog2(PxU32 num)
	{
		for (PxU32 i=0; i<32; i++)
		{
			num >>= 1;
			if (num == 0) return i;
		}

		PX_ASSERT(0);
		return (PxU32)-1;
	}

} // namespace shdfnd
} // namespace physx

#endif

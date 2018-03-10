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


#ifndef PX_FOUNDATION_PSATOMIC_H
#define PX_FOUNDATION_PSATOMIC_H

#include "Ps.h"

namespace physx
{
namespace shdfnd
{
	/* set *dest equal to val. Return the old value of *dest */
	PX_FOUNDATION_API PxI32		atomicExchange(volatile PxI32* dest, PxI32 val);

	/* if *dest == comp, replace with exch. Return original value of *dest */
	PX_FOUNDATION_API PxI32		atomicCompareExchange(volatile PxI32* dest, PxI32 exch, PxI32 comp);

	/* if *dest == comp, replace with exch. Return original value of *dest */
	PX_FOUNDATION_API void * 	atomicCompareExchangePointer(volatile void** dest, void* exch, void* comp);

	/* increment the specified location. Return the incremented value */
	PX_FOUNDATION_API PxI32 	atomicIncrement(volatile PxI32* val);

	/* decrement the specified location. Return the decremented value */
	PX_FOUNDATION_API PxI32 	atomicDecrement(volatile PxI32* val);

	/* add delta to *val. Return the new value */
	PX_FOUNDATION_API PxI32 	atomicAdd(volatile PxI32* val, PxI32 delta);

	/* compute the maximum of dest and val. Return the new value */
	PX_FOUNDATION_API PxI32 	atomicMax(volatile PxI32* val, PxI32 val2);

} // namespace shdfnd
} // namespace physx

#endif

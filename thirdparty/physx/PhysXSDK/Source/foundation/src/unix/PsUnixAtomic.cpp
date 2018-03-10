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
#include "Ps.h"
#include "PsAtomic.h"

#define PAUSE() asm ("nop")

namespace physx
{
namespace shdfnd
{

void *atomicCompareExchangePointer(volatile void **dest,void *exch,void *comp)
{
	return __sync_val_compare_and_swap((void**)dest, comp, exch);
}

PxI32 atomicCompareExchange(volatile PxI32 *dest,PxI32 exch,PxI32 comp)
{
	return __sync_val_compare_and_swap(dest, comp, exch);
}

PxI32 atomicIncrement(volatile PxI32 *val)
{
	return __sync_add_and_fetch(val, 1);
}

PxI32 atomicDecrement(volatile PxI32 *val)
{
	return __sync_sub_and_fetch(val, 1);
}

PxI32 atomicAdd(volatile PxI32 *val, PxI32 delta)
{
	return __sync_add_and_fetch(val, delta);
}

PxI32 atomicMax(volatile PxI32 *val,PxI32 val2)
{
	PxI32 oldVal, newVal;

	do
	{
		PAUSE();
		oldVal = *val;

		if (val2 > oldVal)
			newVal = val2;
		else
	    		newVal = oldVal;

	}
	while (atomicCompareExchange(val, newVal, oldVal) != oldVal);
	
	return *val;
}

PxI32 atomicExchange(volatile PxI32 *val,PxI32 val2)
{
	PxI32 newVal, oldVal;

	do
	{
		PAUSE();
		oldVal = *val;
		newVal = val2;
	}
	while (atomicCompareExchange(val, newVal, oldVal) != oldVal);
	
	return oldVal;
}

} // namespace shdfnd
} // namespace physx


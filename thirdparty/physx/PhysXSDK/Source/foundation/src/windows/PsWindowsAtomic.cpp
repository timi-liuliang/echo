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


#include "windows/PsWindowsInclude.h"
#include "PsAtomic.h"

namespace physx
{
namespace shdfnd
{


PxI32 atomicExchange(volatile PxI32* val,PxI32 val2)
{
	return (PxI32)InterlockedExchange((volatile LONG*)val, (LONG)val2);
}

PxI32 atomicCompareExchange(volatile PxI32* dest, PxI32 exch, PxI32 comp)
{
	return (PxI32)InterlockedCompareExchange((volatile LONG*)dest, exch, comp);
}

void* atomicCompareExchangePointer(volatile void** dest, void* exch, void* comp)
{
	return InterlockedCompareExchangePointer((volatile PVOID*)dest, exch, comp);
}

PxI32 atomicIncrement(volatile PxI32* val)
{
	return  (PxI32)InterlockedIncrement((volatile LONG*)val);
}

PxI32 atomicDecrement(volatile PxI32* val)
{
	return  (PxI32)InterlockedDecrement((volatile LONG*)val);
}

PxI32 atomicAdd(volatile PxI32* val, PxI32 delta)
{
	LONG newValue, oldValue;
	do
	{
		oldValue=*val;
		newValue=oldValue+delta;
	}
	while(InterlockedCompareExchange((volatile LONG*)val,newValue,oldValue)!=oldValue);

	return newValue;
}

PxI32 atomicMax(volatile PxI32* val, PxI32 val2)
{
	//Could do this more efficiently in asm...

	LONG newValue,oldValue;

	do
	{
		oldValue=*val;

		if(val2>oldValue)
			newValue=val2;
		else
			newValue=oldValue;

	} while(InterlockedCompareExchange((volatile LONG*)val,newValue,oldValue)!=oldValue);

	return newValue;
}

} // namespace shdfnd
} // namespace physx

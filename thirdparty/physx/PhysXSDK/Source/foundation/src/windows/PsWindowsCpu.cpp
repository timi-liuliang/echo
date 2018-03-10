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


#include "PsCpu.h"
#pragma warning (push)
//'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning (disable : 4668) 
#ifdef PX_VC10
#pragma warning( disable : 4987 ) // nonstandard extension used: 'throw (...)'
#endif 
#include <intrin.h>
#pragma warning (pop)

namespace physx { namespace shdfnd {

#ifdef PX_ARM
	#define cpuid(reg) reg[0]=reg[1]=reg[2]=reg[3]=0;

	PxU8 Cpu::getCpuId()
	{
		PxU32 cpuInfo[4];	   	
		cpuid(cpuInfo);
		return static_cast<physx::PxU8>(  cpuInfo[1] >> 24 ); // APIC Physical ID
	}
#else
	PxU8 Cpu::getCpuId()
	{
	   int CPUInfo[4];
	   int InfoType = 1;
		__cpuid(CPUInfo, InfoType);
		return static_cast<PxU8>(  CPUInfo[1] >> 24 ); // APIC Physical ID
	}
#endif
}}

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

#if defined(PX_X86)
#define cpuid(op, reg)\
    __asm__ __volatile__("pushl %%ebx      \n\t" /* save %ebx */\
                 "cpuid            \n\t"\
                 "movl %%ebx, %1   \n\t" /* save what cpuid just put in %ebx */\
                 "popl %%ebx       \n\t" /* restore the old %ebx */\
                 : "=a"(reg[0]), "=r"(reg[1]), "=c"(reg[2]), "=d"(reg[3])\
                 : "a"(op)\
                 : "cc")
#else
	#define cpuid(op, reg) reg[0]=reg[1]=reg[2]=reg[3]=0;
#endif

namespace physx { namespace shdfnd {

	physx::PxU8 Cpu::getCpuId()
	{
		PxU32 cpuInfo[4];
		cpuid(1, cpuInfo);
		return static_cast<physx::PxU8>(  cpuInfo[1] >> 24 ); // APIC Physical ID
	}
}}

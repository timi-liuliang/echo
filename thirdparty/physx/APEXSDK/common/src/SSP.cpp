/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "foundation/PxPreprocessor.h"

#if defined PX_PS3 && !defined NDEBUG

#include "foundation/PxAssert.h"

extern "C"
{

// Sony does not include these in libgcc due to problems with license
// so we have to implement them manually

void* __stack_chk_guard = 0;

void __stack_chk_guard_setup()
{
	unsigned char* guard = (unsigned char*)&__stack_chk_guard;
	guard[sizeof(__stack_chk_guard) - 1] = 0xff;
	guard[sizeof(__stack_chk_guard) - 2] = '\n';
	guard[0] = 0;
}

void __attribute__((noreturn)) __stack_chk_fail() {
	PX_ASSERT(0 && "__stack_chk_fail");
	while(1);
}

}

#endif

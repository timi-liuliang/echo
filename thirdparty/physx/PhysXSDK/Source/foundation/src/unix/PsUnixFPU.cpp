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
#include "PsFPU.h"

#if !(defined(__CYGWIN__) || defined(PX_ANDROID) || defined(PX_PS4))
#include <fenv.h>
PX_COMPILE_TIME_ASSERT(8*sizeof(physx::PxU32) >= sizeof(fenv_t));
#endif

#if defined(PX_OSX)
// osx defines SIMD as standard for floating point operations.
#include <xmmintrin.h>
#endif

physx::shdfnd::FPUGuard::FPUGuard()
{
#if defined(__CYGWIN__)
#pragma message "FPUGuard::FPUGuard() is not implemented"
#elif defined(PX_ANDROID)
// not supported unless ARM_HARD_FLOAT is enabled.
#elif defined(PX_PS4)
// not supported
	PX_UNUSED(mControlWords);
#elif defined(PX_OSX)
	mControlWords[0] = _mm_getcsr();
	// set default (disable exceptions: _MM_MASK_MASK) and FTZ (_MM_FLUSH_ZERO_ON), DAZ (_MM_DENORMALS_ZERO_ON: (1<<6))
	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON| (1<<6));
#else
	PX_COMPILE_TIME_ASSERT(sizeof(fenv_t) <= sizeof(mControlWords));
	
	fegetenv(reinterpret_cast<fenv_t*>(mControlWords));
	fesetenv(FE_DFL_ENV);

#if defined(PX_LINUX)
	// need to explicitly disable exceptions because fesetenv does not modify
	// the sse control word on 32bit linux (64bit is fine, but do it here just be sure)
	fedisableexcept(FE_ALL_EXCEPT);
#endif

#endif
}

physx::shdfnd::FPUGuard::~FPUGuard()
{
#if defined(__CYGWIN__) 
#pragma message "FPUGuard::~FPUGuard() is not implemented"
#elif defined(PX_ANDROID)
// not supported unless ARM_HARD_FLOAT is enabled.
#elif defined(PX_PS4)
// not supported
#elif defined(PX_OSX)
	// restore control word and clear exception flags
	// (setting exception state flags cause exceptions on the first following fp operation)
	_mm_setcsr(mControlWords[0] & ~_MM_EXCEPT_MASK);
#else
	fesetenv(reinterpret_cast<fenv_t*>(mControlWords));
#endif
}

PX_FOUNDATION_API void physx::shdfnd::enableFPExceptions()
{
#if defined PX_LINUX
	feclearexcept(FE_ALL_EXCEPT);
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);	
#elif defined PX_OSX
	// clear any pending exceptions
	// (setting exception state flags cause exceptions on the first following fp operation)
	PxU32 control = _mm_getcsr() & ~_MM_EXCEPT_MASK;
	
	// enable all fp exceptions except inexact and underflow (common, benign)
    // note: denorm has to be disabled as well because underflow can create denorms
	_mm_setcsr((control & ~_MM_MASK_MASK) | _MM_MASK_INEXACT | _MM_MASK_UNDERFLOW | _MM_MASK_DENORM);
#endif
}

PX_FOUNDATION_API void physx::shdfnd::disableFPExceptions()
{
#if defined PX_LINUX
	fedisableexcept(FE_ALL_EXCEPT);
#elif defined PX_OSX
	// clear any pending exceptions
	// (setting exception state flags cause exceptions on the first following fp operation)
	PxU32 control = _mm_getcsr() & ~_MM_EXCEPT_MASK;
	_mm_setcsr(control | _MM_MASK_MASK);
#endif
}


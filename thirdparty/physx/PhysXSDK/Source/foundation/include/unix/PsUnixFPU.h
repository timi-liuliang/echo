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


#ifndef PX_FOUNDATION_PSUNIXFPU_H
#define PX_FOUNDATION_PSUNIXFPU_H

#if defined(PX_LINUX) || defined(PX_PS4) || defined(PX_OSX)
#include "PsVecMath.h"

PX_INLINE physx::shdfnd::SIMDGuard::SIMDGuard()
{
	mControlWord = _mm_getcsr();
	// set default (disable exceptions: _MM_MASK_MASK) and FTZ (_MM_FLUSH_ZERO_ON), DAZ (_MM_DENORMALS_ZERO_ON: (1<<6))
	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON| (1<<6));
}

PX_INLINE physx::shdfnd::SIMDGuard::~SIMDGuard()
{
	// restore control word and clear exception flags
	// (setting exception state flags cause exceptions on the first following fp operation)
	_mm_setcsr(mControlWord & ~_MM_EXCEPT_MASK);
}

#endif // defined(PX_LINUX) || defined(PX_PS4) || defined(PX_OSX)

#endif

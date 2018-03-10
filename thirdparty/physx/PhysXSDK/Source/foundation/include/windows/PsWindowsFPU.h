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


#ifndef PX_FOUNDATION_PSWINDOWSFPU_H
#define PX_FOUNDATION_PSWINDOWSFPU_H

PX_INLINE physx::shdfnd::SIMDGuard::SIMDGuard()
{
#if !defined(PX_ARM)
	mControlWord = _mm_getcsr();
	// set default (disable exceptions: _MM_MASK_MASK) and FTZ (_MM_FLUSH_ZERO_ON), DAZ (_MM_DENORMALS_ZERO_ON: (1<<6))
	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON | (1<<6));
#endif
}

PX_INLINE physx::shdfnd::SIMDGuard::~SIMDGuard()
{
#if !defined(PX_ARM)
	// restore control word and clear any exception flags
	// (setting exception state flags cause exceptions on the first following fp operation)
	_mm_setcsr(mControlWord&~_MM_EXCEPT_MASK);
#endif
}

#endif
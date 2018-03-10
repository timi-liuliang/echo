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

#ifndef PS_UNIX_AOS_H
#define PS_UNIX_AOS_H

// no includes here! this file should be included from PxcVecMath.h only!!!

#if !COMPILE_VECTOR_INTRINSICS
#error Vector intrinsics should not be included when using scalar implementation.
#endif

#if defined(PX_X86) || defined(PX_X64)
#	include "sse2/PsUnixSse2AoS.h"
#elif defined(PX_ARM_NEON)
#	include "neon/PsUnixNeonAoS.h"
#else
#	error No SIMD implementation for this unix platform.
#endif


#endif //PS_UNIX_AOS_H

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

#pragma once

#include <math.h>

// ps4 compiler defines _M_X64 without value
#if ( (defined _M_IX86) || (defined _M_X64) ||  (defined __i386__) || defined( __x86_64__))
#define NVMATH_SSE2 1
#else
#define NVMATH_SSE2 0
#endif
#define NVMATH_VMX128 (defined _M_PPC)
#define NVMATH_ALTIVEC (defined __CELLOS_LV2__)
#define NVMATH_NEON (defined _M_ARM || defined __ARM_NEON__)

// which simd types are implemented (one or both are all valid options)
#define NVMATH_SIMD (NVMATH_SSE2 || NVMATH_VMX128 || NVMATH_ALTIVEC || NVMATH_NEON)
#define NVMATH_SCALAR !NVMATH_SIMD
// #define NVMATH_SCALAR 1

// use template expression to fuse multiply-adds into a single instruction
#define NVMATH_FUSE_MULTIPLY_ADD (NVMATH_VMX128 || NVMATH_ALTIVEC || NVMATH_NEON)
// support shift by vector operarations
#define NVMATH_SHIFT_BY_VECTOR (NVMATH_VMX128 || NVMATH_ALTIVEC || NVMATH_NEON)
// Simd4f and Simd4i map to different types
#define NVMATH_DISTINCT_TYPES (NVMATH_SSE2 || NVMATH_ALTIVEC || NVMATH_NEON)
// support inline assembler
#define NVMATH_INLINE_ASSEMBLER !(defined _M_ARM || defined SN_TARGET_PSP2 || defined __arm64__)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// expression template
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

/*! \brief Expression template to fuse and-not. */
template <typename T>
struct ComplementExpr
{
	inline ComplementExpr(T const& v_) : v(v_) {}
	inline operator T() const; 
	const T v;
private:
	ComplementExpr& operator=(const ComplementExpr&); // not implemented
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// helper functions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

template <typename T>
T sqr(const T& x)
{
	return x * x;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// details
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace detail
{
	template <typename T> 
	struct AlignedPointer
	{
		AlignedPointer(const T* p) : ptr(p) {}
		const T* ptr;
	};

	template <typename T> 
	struct OffsetPointer
	{
		OffsetPointer(const T* p, unsigned int off) : ptr(p), offset(off) {}
		const T* ptr;
		unsigned int offset;
	};

	struct FourTuple {};

	// zero and one literals
	template <int i> struct IntType {};
}

// Supress warnings
#if defined(__GNUC__)
#define NVMATH_UNUSED __attribute__((unused))
#else
#define NVMATH_UNUSED
#endif

static detail::IntType<         0> _0    NVMATH_UNUSED;
static detail::IntType<         1> _1    NVMATH_UNUSED;
static detail::IntType<int(0x80000000)> _sign NVMATH_UNUSED;
static detail::IntType<        -1> _true NVMATH_UNUSED;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// platform specific includes
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

#if NVMATH_SSE2
#include "sse2/SimdTypes.h"
#elif NVMATH_VMX128
#include "xbox360/SimdTypes.h"
#elif NVMATH_ALTIVEC
#include "ps3/SimdTypes.h"
#elif NVMATH_NEON
#include "neon/SimdTypes.h"
#else
struct Simd4f;
struct Simd4i;
#endif

#if NVMATH_SCALAR
#include "scalar/SimdTypes.h"
#else
struct Scalar4f;
struct Scalar4i;
#endif

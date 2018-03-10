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


#ifndef PX_FOUNDATION_PSFPU_H
#define PX_FOUNDATION_PSFPU_H

#include "Ps.h"
#include "PsIntrinsics.h"
#include "foundation/PxUnionCast.h"

//unsigned integer representation of a floating-point value.
#ifdef PX_PS3
PX_FORCE_INLINE unsigned int PX_IR(const float x) 
{
	return physx::PxUnionCast<unsigned int, float>(x);
}
#else
#define PX_IR(x)			((PxU32&)(x))
#endif

//signed integer representation of a floating-point value.
#ifdef PX_PS3
PX_FORCE_INLINE int PX_SIR(const float x) 
{
	return physx::PxUnionCast<int, float>(x);
}
#else
#define PX_SIR(x)			((PxI32&)(x))
#endif


//Floating-point representation of a integer value.
#ifdef PX_PS3
PX_FORCE_INLINE float PX_FR(const unsigned int x)
{
	return physx::PxUnionCast<float, unsigned int>(x);
}
#else
#define PX_FR(x)			((PxF32&)(x))
#endif

#ifdef PX_PS3
PX_FORCE_INLINE float* PX_FPTR(unsigned int* x)
{
	return physx::PxUnionCast<float*, unsigned int*>(x);
}

PX_FORCE_INLINE float* PX_FPTR(int* x)
{
	return physx::PxUnionCast<float*, int*>(x);
}
#else
#define PX_FPTR(x)			((PxF32*)(x))
#endif

#define	PX_SIGN_BITMASK		0x80000000

#define PX_FPU_GUARD shdfnd::FPUGuard scopedFpGuard;
#define PX_SIMD_GUARD shdfnd::SIMDGuard scopedFpGuard;

#if defined(PX_WINDOWS) || defined(PX_XBOXONE) || defined(PX_LINUX) || defined(PX_PS4) || defined(PX_OSX)
#define PX_SUPPORT_GUARDS 1
#endif

namespace physx
{
namespace shdfnd
{
	// sets the default SDK state for scalar and SIMD units
	class PX_FOUNDATION_API FPUGuard
	{
	public:
		FPUGuard(); // set fpu control word for PhysX
		~FPUGuard(); // restore fpu control word
	private:
		PxU32 mControlWords[8];
	};

	// sets default SDK state for simd unit only, lighter weight than FPUGuard
	class SIMDGuard
	{
	public:
		PX_INLINE SIMDGuard();  // set simd control word for PhysX
		PX_INLINE ~SIMDGuard(); // restore simd control word
	private:
#ifdef PX_SUPPORT_GUARDS
		PxU32 mControlWord;
#endif
	};

	/**
	\brief Enables floating point exceptions for the scalar and SIMD unit
	*/
	PX_FOUNDATION_API void enableFPExceptions();

	/**
	\brief Disables floating point exceptions for the scalar and SIMD unit
	*/
	PX_FOUNDATION_API void disableFPExceptions();	

} // namespace shdfnd
} // namespace physx

#if defined(PX_WINDOWS) || defined(PX_XBOXONE)
#include "windows/PsWindowsFPU.h"
#elif defined(PX_LINUX) || defined(PX_PS4) || defined(PX_OSX)
#include "unix/PsUnixFPU.h"
#else
PX_INLINE physx::shdfnd::SIMDGuard::SIMDGuard() {}
PX_INLINE physx::shdfnd::SIMDGuard::~SIMDGuard() {}
#endif

#endif

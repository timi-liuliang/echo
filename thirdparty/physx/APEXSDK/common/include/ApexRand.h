/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RAND_H
#define APEX_RAND_H

#include "foundation/PxMath.h"
#include "foundation/PxVec3.h"
#include "PsShare.h"

namespace physx
{
namespace apex
{

// "Quick and Dirty Symmetric Random number generator"	- returns a uniform deviate in [-1.0,1.0)
class QDSRand
{
	physx::PxU32 mSeed;

public:

	PX_CUDA_CALLABLE PX_INLINE QDSRand(physx::PxU32 seed = 0) : mSeed(seed) {}

	PX_CUDA_CALLABLE PX_INLINE void setSeed(physx::PxU32 seed = 0)
	{
		mSeed = seed;
	}

	PX_CUDA_CALLABLE PX_INLINE physx::PxU32 seed() const
	{
		return mSeed;
	}

	PX_CUDA_CALLABLE PX_INLINE physx::PxU32 nextSeed()
	{
		mSeed = mSeed * 1664525L + 1013904223L;
		return mSeed;
	}

	PX_CUDA_CALLABLE PX_INLINE float getNext()
	{
		union NxU32F32
		{
			physx::PxU32   u;
			physx::PxF32   f;
		} r;
		r.u = 0x40000000 | (nextSeed() >> 9);
		return r.f - 3.0f;
	}

	PX_CUDA_CALLABLE PX_INLINE float getScaled(const physx::PxF32 min, const physx::PxF32 max)
	{
		const physx::PxF32 scale = (max - min) / 2.0f;
		return ((getNext() + 1.0f) * scale) + min;
	}

	PX_CUDA_CALLABLE PX_INLINE physx::PxVec3 getScaled(const physx::PxVec3& min, const physx::PxVec3& max)
	{
		return physx::PxVec3(getScaled(min.x, max.x), getScaled(min.y, max.y), getScaled(min.z, max.z));
	}

	PX_CUDA_CALLABLE PX_INLINE float getUnit()
	{
		union NxU32F32
		{
			physx::PxU32   u;
			physx::PxF32   f;
		} r;
		r.u = 0x3F800000 | (nextSeed() >> 9);
		return r.f - 1.0f;
	}

};

// "Quick and Dirty Normal Random number generator"	- returns normally-distributed values
class QDNormRand
{
	QDSRand mBase;

public:

	PX_CUDA_CALLABLE PX_INLINE QDNormRand(physx::PxU32 seed = 0) : mBase(seed) {}

	PX_CUDA_CALLABLE PX_INLINE void setSeed(physx::PxU32 seed = 0)
	{
		mBase.setSeed(seed);
	}

	PX_CUDA_CALLABLE PX_INLINE physx::PxU32 setSeed() const
	{
		return mBase.seed();
	}

	PX_CUDA_CALLABLE PX_INLINE physx::PxU32 nextSeed()
	{
		return mBase.nextSeed();
	}

	PX_CUDA_CALLABLE PX_INLINE float getNext()
	{
		//Using Box-Muller transform (see http://en.wikipedia.org/wiki/Box_Muller_transform)

		physx::PxF32 u, v, s;
		do
		{
			u = mBase.getNext();
			v = mBase.getNext();
			s = u * u + v * v;
		}
		while (s >= 1.0);

		return u * physx::PxSqrt(-2.0f * physx::PxLog(s) / s);
	}

	PX_CUDA_CALLABLE PX_INLINE float getScaled(const physx::PxF32 m, const physx::PxF32 s)
	{
		return m + s * getNext();
	}
};

}
} // end namespace physx::apex

#endif

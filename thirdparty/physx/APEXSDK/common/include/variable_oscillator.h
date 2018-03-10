/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef VARIABLE_OSCILLATOR_H
#define VARIABLE_OSCILLATOR_H

#include "PsShare.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace apex
{

class variableOscillator : public physx::UserAllocated
{
public:
	variableOscillator(physx::PxF32 min, physx::PxF32 max, physx::PxF32 initial, physx::PxF32 period);
	~variableOscillator();
	physx::PxF32 updateVariableOscillator(physx::PxF32 deltaTime);

private:
	physx::PxF32 computeEndVal(physx::PxF32 current, physx::PxF32 max_or_min);

private:
	physx::PxF32	mMin;
	physx::PxF32	mMax;
	physx::PxF32	mPeriod;

	physx::PxF32	mCumTime;
	physx::PxF32	mStartVal;
	physx::PxF32	mEndVal;
	physx::PxF32	mLastVal;
	bool	mGoingUp;
};

}
} // namespace physx::apex

#endif

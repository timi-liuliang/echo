/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "variable_oscillator.h"
#include "foundation/PxMath.h"
#include "PsMathUtils.h"

namespace physx
{
namespace apex
{

variableOscillator::variableOscillator(physx::PxF32 min, physx::PxF32 max, physx::PxF32 initial, physx::PxF32 period) :
	mMin(min),
	mMax(max),
	mPeriod(period),
	mStartVal(initial),
	mLastVal(initial)
{
	mCumTime = 0.0f;
	mGoingUp = true;
	mEndVal = computeEndVal(mMin, mMax);
}


variableOscillator::~variableOscillator()
{
}

physx::PxF32 variableOscillator::computeEndVal(physx::PxF32 current, physx::PxF32 maxOrMin)
{
	physx::PxF32 target;
	physx::PxF32 maxDelta;
	physx::PxF32 quarterVal;

	// compute the max range of the oscillator
	maxDelta = maxOrMin - current;
	// find the 'lower bound' of the oscillator peak
	quarterVal = current + (maxDelta / 4.0f);
	// get a rand between 0 and 1
	target = (physx::PxF32) ::rand() / (physx::PxF32) RAND_MAX;
	// scale the rand to the range we want
	target = target * physx::PxAbs(quarterVal - maxOrMin);
	// add the offset to the scaled random number.
	if (current < maxOrMin)
	{
		target = target + quarterVal;
	}
	else
	{
		target = quarterVal - target;
	}
	return(target);
}

physx::PxF32 variableOscillator::updateVariableOscillator(physx::PxF32 deltaTime)
{
	physx::PxF32 returnVal;
	physx::PxF32 halfRange;

	mCumTime += deltaTime;

	// has the function crossed a max or a min?
	if ((mGoingUp  && (mCumTime > (mPeriod / 2.0f))) ||
	        (!mGoingUp && (mCumTime > mPeriod)))
	{
		mStartVal = mLastVal;
		if (mGoingUp)
		{
			mEndVal = computeEndVal(mStartVal, mMin);
		}
		else
		{
			mEndVal = computeEndVal(mStartVal, mMax);
			mCumTime = mCumTime - mPeriod;
		}
		mGoingUp = !mGoingUp;
	}
	halfRange = 0.5f * physx::PxAbs(mEndVal - mStartVal);
	returnVal = -halfRange * physx::PxCos(mCumTime * PxTwoPi / mPeriod) + halfRange + physx::PxMin(mStartVal, mEndVal);
	mLastVal = returnVal;

	return(returnVal);
}

}
} // namespace physx::apex

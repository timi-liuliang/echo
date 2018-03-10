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


#include "PsTime.h"
#include "windows/PsWindowsInclude.h"

namespace
{
	::physx::PxI64 getTimeTicks()
	{
		LARGE_INTEGER a;
		QueryPerformanceCounter (&a);
		return a.QuadPart;
	}

	double getTickDuration()
	{
		LARGE_INTEGER a;
		QueryPerformanceFrequency (&a);
		return 1.0f / double(a.QuadPart);
	}

	double sTickDuration = getTickDuration();
} // namespace

namespace physx
{
namespace shdfnd
{

static const CounterFrequencyToTensOfNanos gCounterFreq = Time::getCounterFrequency();

const CounterFrequencyToTensOfNanos& Time::getBootCounterFrequency()
{
	return gCounterFreq;
}

CounterFrequencyToTensOfNanos Time::getCounterFrequency()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency (&freq);
	return CounterFrequencyToTensOfNanos( Time::sNumTensOfNanoSecondsInASecond, (PxU64)freq.QuadPart );
}


PxU64 Time::getCurrentCounterValue()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter (&ticks);
	return (PxU64)ticks.QuadPart;
}

Time::Time(): mTickCount(0)
{
	getElapsedSeconds();
}

Time::Second Time::getElapsedSeconds()
{
	PxI64 lastTickCount = mTickCount;
	mTickCount = getTimeTicks();
	return (mTickCount - lastTickCount) * sTickDuration;
}

Time::Second Time::peekElapsedSeconds()
{
	return (getTimeTicks() - mTickCount) * sTickDuration;
}

Time::Second Time::getLastTime() const
{
	return mTickCount * sTickDuration;
}

} // namespace shdfnd
} // namespace physx


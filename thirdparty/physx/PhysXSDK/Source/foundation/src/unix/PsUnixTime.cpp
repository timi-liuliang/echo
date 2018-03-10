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


#include "Ps.h"
#include "PsTime.h"

#include <time.h>
#include <sys/time.h>

#if defined PX_APPLE
#include <mach/mach_time.h>
#endif

// Use real-time high-precision timer.
#ifndef PX_APPLE
#	define CLOCKID CLOCK_REALTIME
#endif

namespace physx
{
namespace shdfnd
{
	
	static const CounterFrequencyToTensOfNanos gCounterFreq = Time::getCounterFrequency();

	const CounterFrequencyToTensOfNanos& Time::getBootCounterFrequency()
	{
		return gCounterFreq;
	}
	
	static Time::Second getTimeSeconds()
	{
		static struct timeval _tv;
		gettimeofday(&_tv, NULL);
		return double(_tv.tv_sec) + double(_tv.tv_usec)*0.000001;
	}
	
	Time::Time()
	{
		mLastTime = getTimeSeconds();
	}
	
	Time::Second Time::getElapsedSeconds()
	{
		Time::Second curTime = getTimeSeconds();
		Time::Second diff = curTime - mLastTime;
		mLastTime = curTime;
		return diff;
	}
		
	Time::Second Time::peekElapsedSeconds()
	{
		Time::Second curTime = getTimeSeconds();
		Time::Second diff = curTime - mLastTime;
		return diff;
	}

	Time::Second Time::getLastTime() const
	{
		return mLastTime;
	}

#ifdef PX_APPLE
	CounterFrequencyToTensOfNanos Time::getCounterFrequency()
	{
		mach_timebase_info_data_t info;
		mach_timebase_info( &info );
		//mach_absolute_time * (info.numer/info.denom) is in units of nano seconds
		return CounterFrequencyToTensOfNanos(info.numer, info.denom * 10);
	}

	PxU64 Time::getCurrentCounterValue()
	{
		return mach_absolute_time();
	}
	
#else

	CounterFrequencyToTensOfNanos Time::getCounterFrequency()
	{
		return CounterFrequencyToTensOfNanos( 1, 10 );
	}

	PxU64 Time::getCurrentCounterValue()
	{
		struct timespec mCurrTimeInt;
		clock_gettime(CLOCKID, &mCurrTimeInt);
		//Convert to nanos as this doesn't cause a large divide here
		return ( static_cast<PxU64>( mCurrTimeInt.tv_sec ) * 1000000000 ) 
			+ ( static_cast<PxU64>( mCurrTimeInt.tv_nsec ) );
	}
#endif

} // namespace shdfnd
} // namespace physx


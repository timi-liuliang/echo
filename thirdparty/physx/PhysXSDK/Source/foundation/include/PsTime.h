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


#ifndef PX_FOUNDATION_PSTIME_H
#define PX_FOUNDATION_PSTIME_H

#include "Ps.h"

#if defined PX_LINUX || defined PX_ANDROID
#include <time.h>
#endif

namespace physx
{
namespace shdfnd
{

	struct CounterFrequencyToTensOfNanos
	{
		PxU64 mNumerator;
		PxU64 mDenominator;
		CounterFrequencyToTensOfNanos( PxU64 inNum, PxU64 inDenom )
			: mNumerator( inNum )
			, mDenominator( inDenom )
		{
		}

		//quite slow.
		PxU64 toTensOfNanos( PxU64 inCounter ) const
		{
			return ( inCounter * mNumerator ) / mDenominator;
		}
	};

	class PX_FOUNDATION_API Time
	{
	public:
		typedef double Second;
		static const PxU64 sNumTensOfNanoSecondsInASecond = 100000000;
		//This is supposedly guaranteed to not change after system boot
		//regardless of processors, speedstep, etc.		
		static const CounterFrequencyToTensOfNanos& getBootCounterFrequency();

		static CounterFrequencyToTensOfNanos getCounterFrequency();

		static PxU64 getCurrentCounterValue();

		//SLOW!!
		//Thar be a 64 bit divide in thar!
		static PxU64 getCurrentTimeInTensOfNanoSeconds()
		{
			PxU64 ticks = getCurrentCounterValue();
			return getBootCounterFrequency().toTensOfNanos( ticks );
		}

		Time();
		Second getElapsedSeconds();
		Second peekElapsedSeconds();
		Second getLastTime() const;

	private:
#if defined(PX_LINUX) || defined(PX_ANDROID) || defined(PX_APPLE) || defined(PX_PSP2) || defined(PX_PS4)
		Second	mLastTime;
#else
		PxI64	mTickCount;
#endif
	};
} // namespace shdfnd
} // namespace physx

#endif

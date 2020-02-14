#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <functional>

#ifdef ECHO_PLATFORM_WINDOWS
	#include <windows.h>
	#include <time.h>
#elif defined(ECHO_PLATFORM_IOS) || defined(ECHO_PLATFORM_MAC)
	#include <mach/mach_time.h>
#endif

namespace Echo
{
	class Time
	{	
	public:
		// Task
		struct Task
		{
			ulong				  m_startTime;
			std::function<void()> m_cb;
		};

	public:
		~Time();
		static Time* instance();

		// tick
		void tick();

	public:
		// reset
		void reset();

		/** Returns milliseconds since initialization or last reset */
		unsigned long getMilliseconds();

		/** Returns microseconds since initialization or last reset */
		unsigned long getMicroseconds();

		/** Returns milliseconds since initialization or last reset, only CPU time measured */	
		unsigned long getMillisecondsCPU();

		/** Returns microseconds since initialization or last reset, only CPU time measured */	
		unsigned long getMicrosecondsCPU();

	public:
		// add event
		void addDelayTask(ulong delayMilliseconds, std::function<void()> cb);

	private:
		Time();

	protected:
		clock_t mZeroClock;

#ifdef ECHO_PLATFORM_WINDOWS
		Echo::ui32 mStartTick;
		LONGLONG mLastTime;
		LARGE_INTEGER mStartTime;
		LARGE_INTEGER mFrequency;

		DWORD_PTR mTimerMask;

#elif defined(ECHO_PLATFORM_IOS) || defined(ECHO_PLATFORM_MAC)
        
        uint64_t m_IosStartTime;
        mach_timebase_info_data_t m_TimeBaseInfo;
        
#elif defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_HTML5)
		struct timespec m_AndroidStartTime;
#else 
		struct timeval	m_startTime;
#endif
		vector<Task>::type		m_tasks;
	};
}

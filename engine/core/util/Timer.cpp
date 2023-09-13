#include "Timer.h"
#include "StringUtil.h"
#include <algorithm>

#ifndef ECHO_PLATFORM_WINDOWS
#include <sys/time.h>
#endif

namespace Echo
{
	Time* Time::instance()
	{
		static Time* inst = EchoNew(Time);
		return inst;
	}

	Time::Time()
#ifdef ECHO_PLATFORM_WINDOWS
		: m_timerMask( 0 )
#endif
	{
		reset();
	}

	Time::~Time()
	{

	}

	void Time::tick()
	{
		if (!m_tasks.empty())
		{
			ulong currentTime = getMilliseconds();
			for (auto it = m_tasks.begin(); it != m_tasks.end();)
			{
				if (currentTime >= it->m_startTime)
				{
					it->m_cb();
					it = m_tasks.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}

	void Time::addDelayTask(ulong delayMilliseconds, std::function<void()> cb)
	{
		Task task;
		task.m_startTime = getMilliseconds() + delayMilliseconds;
		task.m_cb = cb;
		m_tasks.emplace_back(task);
	}

	void Time::reset()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		DWORD_PTR procMask;
		DWORD_PTR sysMask;
		GetProcessAffinityMask(GetCurrentProcess(), &procMask, &sysMask);

		// If procMask is 0, consider there is only one core available
		// (using 0 as procMask will cause an infinite loop below)
		if (procMask == 0)
			procMask = 1;

		// Find the lowest core that this process uses
		if( m_timerMask == 0 )
		{
			m_timerMask = 1;
			while( ( m_timerMask & procMask ) == 0 )
			{
				m_timerMask <<= 1;
			}
		}

		HANDLE thread = GetCurrentThread();

		// Set affinity to the first core
		DWORD_PTR oldMask = SetThreadAffinityMask(thread, m_timerMask);

		// Get the constant frequency
		QueryPerformanceFrequency(&m_frequency);

		// Query the timer
		QueryPerformanceCounter(&m_startTime);
		m_startTick = GetTickCount();

		// Reset affinity
		SetThreadAffinityMask(thread, oldMask);

		m_lastTime = 0;
		m_zeroClock = clock();
#elif defined(ECHO_PLATFORM_IOS) || defined(ECHO_PLATFORM_MAC)
        m_zeroClock = clock();
        m_IosStartTime = mach_absolute_time();
        if(KERN_SUCCESS != mach_timebase_info(&m_TimeBaseInfo))
        {
            return;
        }
#elif defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_HTML5)
        m_zeroClock = clock();
		clock_gettime(CLOCK_MONOTONIC, &m_startTime);
#else
		m_zeroClock = clock();
		gettimeofday(&m_startTime, NULL);
#endif
	}

	unsigned long Time::getMilliseconds()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		LARGE_INTEGER curTime;

		HANDLE thread = GetCurrentThread();

		// Set affinity to the first core
		DWORD_PTR oldMask = SetThreadAffinityMask(thread, m_timerMask);

		// Query the timer
		QueryPerformanceCounter(&curTime);

		// Reset affinity
		SetThreadAffinityMask(thread, oldMask);

		LONGLONG newTime = curTime.QuadPart - m_startTime.QuadPart;

		// scale by 1000 for milliseconds
		unsigned long newTicks = (unsigned long) (1000 * newTime / m_frequency.QuadPart);

		// detect and compensate for performance counter leaps
		// (surprisingly common, see Microsoft KB: Q274323)
		unsigned long check = GetTickCount() - m_startTick;
		signed long msecOff = (signed long)(newTicks - check);
		if (msecOff < -100 || msecOff > 100)
		{
			// We must keep the timer running forward :)
			LONGLONG adjust = std::min<LONGLONG>(msecOff * m_frequency.QuadPart / 1000, newTime - m_lastTime);
			m_startTime.QuadPart += adjust;
			newTime -= adjust;

			// Re-calculate milliseconds
			newTicks = (unsigned long) (1000 * newTime / m_frequency.QuadPart);
		}

		// Record last time for adjust
		m_lastTime = newTime;

		return newTicks;
#elif defined(ECHO_PLATFORM_IOS) || defined(ECHO_PLATFORM_MAC)
        return  (mach_absolute_time() - m_IosStartTime) * m_TimeBaseInfo.numer / m_TimeBaseInfo.denom / 1000000;
#elif defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_HTML5)
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return (now.tv_sec - m_startTime.tv_sec) * 1000 + (now.tv_nsec - m_startTime.tv_nsec) / 1000000;
#else
		struct timeval now;
		gettimeofday(&now, NULL);
		return (now.tv_sec-m_startTime.tv_sec)*1000+(now.tv_usec-m_startTime.tv_usec)/1000;
#endif
	}

	unsigned long Time::getMicroseconds()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		LARGE_INTEGER curTime;

		HANDLE thread = GetCurrentThread();

		// Set affinity to the first core
		DWORD_PTR oldMask = SetThreadAffinityMask(thread, m_timerMask);

		// Query the timer
		QueryPerformanceCounter(&curTime);

		// Reset affinity
		SetThreadAffinityMask(thread, oldMask);

		LONGLONG newTime = curTime.QuadPart - m_startTime.QuadPart;

		// get milliseconds to check against GetTickCount
		unsigned long newTicks = (unsigned long) (1000 * newTime / m_frequency.QuadPart);

		// detect and compensate for performance counter leaps
		// (surprisingly common, see Microsoft KB: Q274323)
		unsigned long check = GetTickCount() - m_startTick;
		signed long msecOff = (signed long)(newTicks - check);
		if (msecOff < -100 || msecOff > 100)
		{
			// We must keep the timer running forward :)
			LONGLONG adjust = (std::min)(msecOff * m_frequency.QuadPart / 1000, newTime - m_lastTime);
			m_startTime.QuadPart += adjust;
			newTime -= adjust;
		}

		// Record last time for adjust
		m_lastTime = newTime;

		// scale by 1000000 for microseconds
		unsigned long newMicro = (unsigned long) (1000000 * newTime / m_frequency.QuadPart);

		return newMicro;
        
#elif defined(ECHO_PLATFORM_IOS) || defined(ECHO_PLATFORM_MAC)
        
        return  (mach_absolute_time() - m_IosStartTime) * m_TimeBaseInfo.numer / m_TimeBaseInfo.denom / 1000;
        
#elif defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_HTML5)
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return (now.tv_sec - m_startTime.tv_sec) * 1000000 + (now.tv_nsec - m_startTime.tv_nsec) / 1000;
#else

		struct timeval now;
		gettimeofday(&now, NULL);
		return (now.tv_sec - m_startTime.tv_sec) * 1000000 + (now.tv_usec - m_startTime.tv_usec);
#endif
	}

	unsigned long Time::getMillisecondsCPU()
	{
		clock_t newClock = clock();
		return (unsigned long)((float)(newClock-m_zeroClock) / ((float)CLOCKS_PER_SEC/1000.0)) ;
	}

	unsigned long Time::getMicrosecondsCPU()
	{
		clock_t newClock = clock();
		return (unsigned long)((float)(newClock-m_zeroClock) / ((float)CLOCKS_PER_SEC/1000000.0)) ;
	}

	bool Time::getTimeOfDay(i32& year, i32& month, i32& day)
	{
		std::time_t now = std::time(0);
		std::tm* localTime = std::localtime(&now);
		if (localTime)
		{
			year = localTime->tm_year + 1900;
			month = localTime->tm_mon + 1;
			day = localTime->tm_mday;

			return true;
		}

		return false;
	}
}

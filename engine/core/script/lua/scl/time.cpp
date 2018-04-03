////////////////////////////////////////////////////////////////////////////////
//	时间相关
//	
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////

#include "scl/time.h"
#include "scl/assert.h"
#include "scl/string.h"
#include "scl/thread.h"


#ifdef SCL_WIN
#include <windows.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
#include <sys/time.h>
#include <unistd.h>
#endif

namespace scl {

#ifdef SCL_LINUX
#pragma GCC diagnostic ignored "-Wnonnull"
#endif

int timezone()
{
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	struct ::timezone tz;
	::gettimeofday(NULL, &tz);
	return tz.tz_minuteswest / 60;
#endif
#ifdef SCL_WIN
	TIME_ZONE_INFORMATION tz;
	::GetTimeZoneInformation(&tz); 
	return tz.Bias / 60;
#endif
}
#ifdef SCL_LINUX
#pragma GCC diagnostic warning "-Wnonnull"
#endif

} //namespace scl


#include <time.h>

namespace scl	{

class thread_time_holder
{
public:
	thread_time_holder						() {}
	~thread_time_holder						() { stop_time_thread(); }
	
	void			start_time_thread		();
	void			stop_time_thread		();
	static void*	_scl_time_thread_func	(void*p, int* signal);

private:
	scl::thread 	_scl_time_thread;
};

volatile	uint64 		_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK 	= 0;
volatile	time_t		_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME	= 0;
volatile	uint		g_time_init_flag 		= 0;
thread_time_holder* 	g_thread_time_holder	= NULL; 
ptr<thread_time_holder>	g_ptrguard_thread_time_holder(NULL);

#ifdef SCL_WIN
volatile	int			_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND = 0;
#endif

uint64 _scl__inner__init_tick()			// get tick which has been written by time thread
{
	if (_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK == 0)
	{
		if (compare_and_swap(&g_time_init_flag, 0, 1)) 
		{
			g_thread_time_holder = new thread_time_holder;
			g_ptrguard_thread_time_holder.p = g_thread_time_holder;
			g_thread_time_holder->start_time_thread();
		}
		else
			// g_time_init_flag  == 1 , so time thread has been stopped. call get_real_timetick
			return get_realtime_tick();

		while (_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK == 0) scl::sleep(1);
	}
	return _DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK;
}

time_t _scl__inner__init_time()			// get time which has been written by time thread
{
	_scl__inner__init_tick();
	return _DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME;
}

#ifdef SCL_WIN
int _scl__inner__init_millisecond()			// get time which has been written by time thread
{
	_scl__inner__init_tick();
	return _DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND;
}
#endif


const time& time::now()
{
	::time(&m_time); 
	do_local();
	return *this;
}

void time::to_string(char* output, const int outputCapacity, bool withYear, bool withMillisecond) const
{
	pstring timeString(output, outputCapacity);
	if (withYear)
		timeString.format("%d-%.2d-%.2d %.2d:%.2d:%.2d", year, month, day, hour, minute, second);
	else
		timeString.format("%.2d-%.2d %.2d:%.2d:%.2d", month, day, hour, minute, second);

	if (withMillisecond)
		timeString.format_append(":%.3d", millisecond);
}

void time::to_filename(char* output, const int outputCapacity, bool withYear, bool withMilliSecond) const
{
	pstring s(output, outputCapacity);
	if (withYear)
		s.format_append("%d_", year);
	s.format_append("%.2d%.2d_%.2d%.2d%.2d", month, day, hour, minute, second);
	if (withMilliSecond)
		s.format_append("_%.4d", millisecond);
}

bool string_to_time(const char* timeStr, int& hour, int& min)
{
	int sep = 0;
	string8 shour;
	string8 smin;
	string64 time(timeStr);
	sep = time.find(':');
	if (sep == -1)
		return false;
	time.substr(0, sep, shour.c_str(), shour.capacity());
	time.substr(sep + 1, time.length() - sep, smin.c_str(), smin.capacity());
	int thour = shour.to_int();
	int tmin = smin.to_int();
	if (thour < 0 || thour >= 24) return false;
	if (tmin < 0 || tmin >= 60) return false;
	hour = thour;
	min = tmin;
	return true;
}

bool string_to_date(const char* dataStr, uint& year, uint& month, uint& day)
{
	int sep1 = 0;
	int sep2 = 0;
	string8 y;
	string8 m;
	string8 d;
	string64 data(dataStr);
	sep1 = data.find('-');
	sep2 = data.find('-', sep1+1);
	
	if (sep1 == -1)
		return false;
	if (sep2 == -1)
		return false;

	data.substr(0, sep1, y.c_str(), y.capacity());
	data.substr(sep1 + 1, sep2, m.c_str(), m.capacity());
	data.substr(sep2 + 1, data.length() - sep2, d.c_str(), d.capacity());
	year = y.to_int();
	month = m.to_int();
	day = d.to_int();
	if (month <= 0 || month > 12) return false;
	if (day <= 0 || day > 31) return false;
	return true;
}

uint64 time::time_stamp() const
{
	tm t;
	t.tm_year = year-1900;
	t.tm_mon = month-1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	t.tm_isdst = daylightSavingTime;
	return ::mktime(&t);
}

void time::do_local(int _millisecond)
{
	tm t;
#ifdef SCL_WIN
	millisecond = _millisecond;
	if (-1 == millisecond)
	{
		SYSTEMTIME win_tm;
		::GetLocalTime(&win_tm);
		millisecond =	win_tm.wMilliseconds;
	}

	::localtime_s(&t, &m_time);
	second		=	t.tm_sec;     
	minute		=	t.tm_min;     
	hour		=	t.tm_hour;    
	day			=	t.tm_mday;
	month		=	t.tm_mon + 1;
	year		=	t.tm_year + 1900;    
	dayOfWeek	=	t.tm_wday;    
	dayOfYear	=	t.tm_yday;
	daylightSavingTime = t.tm_isdst;
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	millisecond = _millisecond;
	if (-1 == static_cast<int>(millisecond))
	{
		timeval timeofday;
		::gettimeofday(&timeofday, NULL);
		millisecond =	timeofday.tv_usec / 1000;
	}

	::localtime_r(&m_time, &t);
	second		=	t.tm_sec;     
	minute		=	t.tm_min;     
	hour		=	t.tm_hour;    
	day			=	t.tm_mday;
	month		=	t.tm_mon + 1;
	year		=	t.tm_year + 1900;    
	dayOfWeek	=	t.tm_wday;    
	dayOfYear	=	t.tm_yday;    
	daylightSavingTime = t.tm_isdst;
#endif
}

//获取系统当前Tick，单位毫秒(ms)
uint64 get_realtime_tick()
{
#ifdef SCL_WIN
	LARGE_INTEGER currentTick;
	memset(&currentTick, 0, sizeof(currentTick));
	if (!::QueryPerformanceCounter(&currentTick))
	{
		assert(0);
		return 0;
	}
	LARGE_INTEGER proc_freq;
	if (!::QueryPerformanceFrequency(&proc_freq) && proc_freq.QuadPart > 0)
	{
		assert(0);
		return 0;
	}
	return currentTick.QuadPart * 1000 / proc_freq.QuadPart;
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000 + time.tv_usec / 1000;
#else
	return 0;
#endif



	//RDTSC, result in eax, edx
	//由于多核下可能存在问题，因此暂不使用
	//_asm
	//{
	//	_emit 0x0F
	//	_emit 0x31
	//}
}

void sleep(uint32 milliseconds)
{
#ifdef SCL_WIN
	::Sleep(milliseconds);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	::usleep(milliseconds * 1000);
#endif
}

void usleep(unsigned int useconds)
{
#ifdef SCL_WIN
	static HANDLE hTimer = NULL;
	if (NULL == hTimer)
	{
		// Create an unnamed waitable timer.
		hTimer = ::CreateWaitableTimer(NULL, TRUE, NULL);
		if (NULL == hTimer)
		{
			return;
		}
	}

	LARGE_INTEGER	liDueTime;
	liDueTime.QuadPart = -10LL * useconds;

    // Set a timer to wait for 10 seconds.
	if (!::SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
    {
        return;
    }

    // Wait for the timer.

	if (::WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
	{
		assertf(false, "WaitForSingleObject failed (%d)\n", ::GetLastError());
	}
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	::usleep(useconds);
#endif
}

//uint64 GetFrequency()
//{
//	LARGE_INTEGER proc_freq;
//	if (!::QueryPerformanceFrequency(&proc_freq) && proc_freq.QuadPart > 0)
//	{
//		assert(0);
//		return 0;
//	}
//	return proc_freq.QuadPart;
//}
//
//uint64 GetCounter()
//{
//	LARGE_INTEGER currentTick;
//	memset(&currentTick, 0, sizeof(currentTick));
//	if (!QueryPerformanceCounter(&currentTick))
//	{
//		assert(0);
//		return 0;
//	}
//	return currentTick.QuadPart;
//}


void timer::start(int interval) //单位：毫秒
{
	m_interval = interval;
	m_nextTick = SCL_TICK + m_interval;
}


void timer::start() //单位：毫秒
{
	start(m_interval);
}

bool timer::ring()
{
	uint64 now = SCL_TICK;

	if (!m_paused)
	{
		if (m_nextTick <= now)
		{
			if (m_update_to_now)
				m_nextTick = now + m_interval;
			else
				m_nextTick += m_interval;

			return true;
		}
	}
	return false;
}

bool timer::is_start()
{
	return m_interval > 0;
}

bool timer::is_paused()
{
	return m_paused;
}

void timer::pause()
{
	if (m_paused)
		return;

	m_paused = true;
	m_lastPauseTick = SCL_TICK;
}

void timer::resume()
{
	if (!m_paused)
		return;

	uint64 now = SCL_TICK;
	assert(now >= m_lastPauseTick);
	m_nextTick += (now - m_lastPauseTick);

	m_paused = false;
}

void timer2::start(const uint64 interval) 	//开启计时器。单位，毫秒; // interval = 0
{
	if (interval == 0)
		return;

	m_interval = interval;
	m_current_tick = 0;
	resume();
}

void timer2::start() 	//开启计时器。单位，毫秒; // interval = 0
{
	if (m_interval == 0)
		return;

	m_current_tick = 0;
	resume();
}

bool timer2::ring(const uint64 diff)
{
	if (m_paused)
	{
		return false;
	}
	m_current_tick += diff;
	if (m_current_tick >= m_interval)
	{
		m_current_tick -= m_interval;
		return true;
	}
	return false;
}


void thread_time_holder::start_time_thread()
{
	_scl_time_thread.start(_scl_time_thread_func, NULL, false);
	_scl_time_thread.set_affinity_mask(0);
	_scl_time_thread.resume();
}

void thread_time_holder::stop_time_thread()
{
	if (_scl_time_thread.is_running())
	{
		_scl_time_thread.send_stop_signal();
		if (!_scl_time_thread.wait(-1, false))
			assert(0);
		_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK = 0;
	}
}

void* thread_time_holder::_scl_time_thread_func(void*p, int* signal)
{
	while (1)
	{
		if (*signal) 
		{
			_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK = 0;
			break;
		}

#ifdef SCL_WIN
		SYSTEMTIME win_tm;
		::GetLocalTime(&win_tm);
		_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND = win_tm.wMilliseconds;
#endif

		::time((time_t*)&_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME);
		_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK = get_realtime_tick();
#if defined(SCL_LINUX) || defined(SCL_APPLE)
	timeval time;
	gettimeofday(&time, NULL);
	::usleep(1000 - (time.tv_usec % 1000));
#endif
#ifdef SCL_WIN
	scl::sleep(1);
#endif

	}
	return NULL;
}

time_span::time_span()
: m_begin(0LL)
, m_end(0LL)
{
}

time_span::time_span(uint64 begin, uint64 end)
: m_begin(begin)
, m_end(end)
{
	assert(end >= begin);
}

void time_span::offset(int sec)
{
	m_begin += sec;
	m_end += sec;
}

int time_span::compare(uint64 t) const
{
	if (t < m_begin)
		return -1;
	if (t > m_end)
		return 1;
	return 0;
}

time_t get_realtime_time()
{
	::time((time_t*)&_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME);
	return _DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME;
}


//
//void timeBuckets::to_string(string64& output) const
//{
//	time begin(m_begin);
//	time end(m_end);
//	string32 beginStr;
//	string32 endStr;
//	begin.to_string(beginStr);
//	end.to_string(endStr);
//	output = beginStr.c_str();
//	output += " ~ ";
//	output += endStr.c_str();
//}
//

}	//namespace scl



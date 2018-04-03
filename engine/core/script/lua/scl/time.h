////////////////////////////////////////////////////////////////////////////////
//	时间相关
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/pstring.h"
#include "scl/string.h"

#include "scl/thread.h"

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <time.h>
#endif

namespace scl {

//获取系统当前Tick，单位毫秒(ms)
uint64 	get_realtime_tick();	// get current real time with system API
time_t	get_realtime_time();

#if defined(SCL_ANDROID) || defined(SCL_APPLE) || defined(SCL_HTML5)
#define SCL_TICK scl::get_realtime_tick()
#define SCL_TIME scl::get_realtime_time() 
#endif

#if defined(SCL_WIN) || defined(SCL_LINUX)
#define SCL_TICK ((scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK == 0) ? (scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK = scl::_scl__inner__init_tick()) : scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK)
#define SCL_TIME ((scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME == 0) ? (scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME = scl::_scl__inner__init_time()) : scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME)
#endif


#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#define SCL_MILLISECOND (int(SCL_TICK % 1000))
#endif
#ifdef SCL_WIN
#define SCL_MILLISECOND ((scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND == 0) ? (scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND = scl::_scl__inner__init_millisecond()) : scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND)
#endif

//休眠
void sleep(uint32 milliseconds);	//单位毫秒(ms)
void usleep(uint32 useconds);		//单位微妙(us)

//获取字符串格式为 12:30 类型的时间
bool string_to_time(const char* timeStr, int& hour, int& min);
//获取字符串格式为 1984-03-21 类型的日期
bool string_to_date(const char* dataStr, uint& year, uint& month, uint& day);

//时间日期类
class time
{
public:
	time() : m_time(0), second(0), minute(0), hour(0), day(0), month(0), year(0), dayOfWeek(0), dayOfYear(0), daylightSavingTime(0), millisecond(0) {}
	time(uint64 t) : m_time(static_cast<time_t>(t)) { do_local(); }
	time(uint64 t, int _millisecond) : m_time(static_cast<time_t>(t)) { do_local(_millisecond);  }

	void			set(uint64 t) { m_time = static_cast<time_t>(t); do_local(); }
	uint64			time_stamp() const;
	const time& 	now();
	void			to_string	(char* output, const int outputCapacity, bool withYear = true, bool withMillisecond = true) const;
	void			to_string	(string32& output, bool withYear = true, bool withMillisecond = true) const { return to_string(output.c_str(), output.capacity(), withYear, withMillisecond); }
	void			to_filename	(char* output, const int outputCapacity, bool withYear = true, bool withMilliSecond = true) const;
	void			do_local	(int _millisecond = -1);
	
	::time_t m_time;

	uint second;				// seconds after the minute - [0,59] 
	uint minute;				// minutes after the hour - [0,59] 
	uint hour;					// hours since midnight - [0,23] 
	uint day;					// day of the month - [1,31]
	uint month;					// months since January - [1,12] 
	uint year;					// years since 0
	uint dayOfWeek;				// days since Sunday - [0,6] 
	uint dayOfYear;				// days since January 1 - [0,365] 
	uint daylightSavingTime;	// daylight savings time flag 
	uint millisecond;			// millisecond
};

////////////////////////////////////
//[计时器1]
//实现：
//		内部保存了“下次tick的时间m_nextTick”，每次ring的时候都会更新m_nextTick
//		更新m_nextTick时候内部会调用scl::getTick()函数
//有点：
//		不像timer2那样，ring函数需要一个参数，因此用起来简单方便一些
//缺点：
//		由于依赖于scl::getTick函数，所以当getTick归零的时候会导致长时间不会触发ring
////////////////////////////////////
class timer
{
public:
	timer() : m_nextTick(0), m_interval(0), m_paused(false), m_lastPauseTick(0), m_update_to_now(false) {}

	void start				(int interval);		//开启计时器。单位，毫秒;
	void start				(); 				//开启计时器。单位，毫秒;
	bool ring				();					//是否响铃
	void pause				();					//暂停计时器
	void resume				();					//恢复被暂停的计时器

	void set_interval		(const int interval) { m_interval = interval; }
	int	 get_interval		() { return m_interval; }
	bool is_start			();					//是否已经开始
	bool is_paused			();					//是否已经暂停
	void set_update_to_now	(bool value) { m_update_to_now = value; } //每次ring的时候，更新时间是否为now + interval，当该值为true的时候，如果两次检查ring的时间大于2次interval，则只会ring1次

private:
	uint64	m_nextTick;
	int		m_interval;

	//pause逻辑相关
	bool	m_paused;
	uint64	m_lastPauseTick;
	bool	m_update_to_now;		//每次ring的时候，更新时间是否为now + interval，当该值为true的时候，如果两次检查ring的时间大于2次interval，则只会ring1次
};

////////////////////////////////////
//[计时器2]
//实现：
//		内部保存了“在一个tick内，当前已经流逝的时间m_currentTick”
//		当流逝时间到了的时候，将流逝时间减去一个tick的长度，然后继续
//优点：
//		不依赖于scl::get_tick函数
//缺点：
//		ring需要传入一个参数diff，表示上次调用ring和这次调用ring的时间间隔
//		这个参数一般可以通过两帧之间的时间间隔来确定
////////////////////////////////////
class timer2
{
public:
	timer2() : m_current_tick(0), m_interval(0) {}

	void	start			(const uint64 interval); 								//开启计时器。单位，毫秒;
	void	start			(); 													//开启计时器。单位，毫秒;
	bool	ring			(const uint64 diff);									//是否响铃;
	void	pause			()						{ m_paused = true;			}	//暂停计时器
	void	resume			()						{ m_paused = false;			}	//恢复被暂停的计时器

	void	set_interval	(const uint64 interval) { m_interval = interval;	}
	uint64	get_interval	() const				{ return m_interval;		}
	uint64	get_current_tick() const				{ return m_current_tick;	}
	bool	is_start		() const				{ return m_interval > 0;	}	//是否已经开始
	bool	is_paused		() const				{ return m_paused;			}	//是否已经暂停

private:
	uint64	m_current_tick;
	uint64	m_interval;
	bool	m_paused;		//是否暂停
};

// 功能类，时间段
class time_span
{
public:
	time_span	();
	time_span	(uint64 begin, uint64 end);
	void	offset		(int sec);											// 时间段整体偏移sec秒
	int		compare		(uint64 t) const;									// 类似strcmp. 测试t是否在时间段内,在的话,返回0,还没达到,返回<0,已经超过返回>0
	bool	contains	(uint64 t) const { return compare(t) == 0; };		//是否包含时间点
	uint64	begin		() const { return m_begin; }
	uint64	end			() const { return m_end; }

private:
	uint64 m_begin;
	uint64 m_end;
};

// get tick which has been written by time thread, if TICK == 0 then start time thread and wait until TICK > 0
uint64	_scl__inner__init_tick();				
time_t	_scl__inner__init_time();				
#ifdef SCL_WIN
int		_scl__inner__init_millisecond();				
#endif

// thread tick, don't use this directly, use SCL_TICK macro instead.
extern volatile uint64 	_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK;  
extern volatile time_t	_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME;  
#ifdef SCL_WIN
extern volatile int		_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND; 
#endif

int timezone(); //返回当前时区-12~+12，为UTC的相反数，例如北京是UTC+8, 这里的值就为-8


} //namespace scl



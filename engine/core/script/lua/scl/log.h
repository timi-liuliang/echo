////////////////////////////////////////////////////////////////////////////////
//	log
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/file.h"
#include "scl/string.h"
#include "scl/log_file.h"
#include "scl/thread_log.h"
#include "scl/log_define.h"
#include "scl/thread.h"
#include "scl/array.h"

namespace scl {

#ifdef SCL_WIN
#define SCL_FUNCTION	__FUNCTION__
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#define SCL_FUNCTION	__PRETTY_FUNCTION__
#endif

class log_stream;
#define log_verbose		scl::log_stream(scl::LOG_LEVEL_VERBOSE,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_debug		scl::log_stream(scl::LOG_LEVEL_DEBUG,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_info		scl::log_stream(scl::LOG_LEVEL_INFO,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_warn		scl::log_stream(scl::LOG_LEVEL_WARN,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_error		scl::log_stream(scl::LOG_LEVEL_ERROR,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_fatal		scl::log_stream(scl::LOG_LEVEL_FATAL,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_user1		scl::log_stream(scl::LOG_LEVEL_USER1,	-1, __FILE__, SCL_FUNCTION, __LINE__)
#define log_user(l)		scl::log_stream(l,						-1, __FILE__, SCL_FUNCTION, __LINE__)



//#define LOG_VERBOSE(s)	do{log_verbose	<< s << scl::endl;} while(0)
//#define LOG_DEBUG(s)	do{log_debug	<< s << scl::endl;} while(0)
//#define LOG_INFO(s)		do{log_info		<< s << scl::endl;} while(0)
//#define LOG_WARN(s)		do{log_warn		<< s << scl::endl;} while(0)
//#define LOG_ERROR(s)	do{log_error	<< s << scl::endl;} while(0)


//注意，log_direct是线程不安全的，不保证不会发生日志打印错误或死锁，所有的log_direct都是error日志
//当只有主线程打印日志的时候，是不会开启多线程模式的，所以尽量不要使用log_direct
#ifdef SCL_WIN
#define log_verbose_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_VERBOSE,    -1, __FILE__, __FUNCTION__, __LINE__, s, __VA_ARGS__)
#define log_debug_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_DEBUG,    -1, __FILE__, __FUNCTION__, __LINE__, s, __VA_ARGS__)
#define log_info_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_INFO,    -1, __FILE__, __FUNCTION__, __LINE__, s, __VA_ARGS__)
#define log_warn_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_WARN,    -1, __FILE__, __FUNCTION__, __LINE__, s, __VA_ARGS__)
#define log_error_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_ERROR,    -1, __FILE__, __FUNCTION__, __LINE__, s, __VA_ARGS__)
#define log_fatal_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_FATAL,    -1, __FILE__, __FUNCTION__, __LINE__, s, __VA_ARGS__)
#define log_direct(s, ...)	scl::log::direct_out(scl::LOG_LEVEL_DEBUG,	-1, __FILE__, SCL_FUNCTION, __LINE__, s, __VA_ARGS__)
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined (SCL_ANDROID)
#define log_verbose_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_VERBOSE,    -1, __FILE__, __FUNCTION__, __LINE__, s, ##__VA_ARGS__)
#define log_debug_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_DEBUG,    -1, __FILE__, __FUNCTION__, __LINE__, s, ##__VA_ARGS__)
#define log_info_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_INFO,    -1, __FILE__, __FUNCTION__, __LINE__, s, ##__VA_ARGS__)
#define log_warn_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_WARN,    -1, __FILE__, __FUNCTION__, __LINE__, s, ##__VA_ARGS__)
#define log_error_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_ERROR,    -1, __FILE__, __FUNCTION__, __LINE__, s, ##__VA_ARGS__)
#define log_fatal_unsafe(s, ...)    scl::log::out(scl::LOG_LEVEL_FATAL,    -1, __FILE__, __FUNCTION__, __LINE__, s, ##__VA_ARGS__)
#define log_direct(s, ...)	scl::log::direct_out(scl::LOG_LEVEL_DEBUG,	-1, __FILE__, SCL_FUNCTION, __LINE__, s, ##__VA_ARGS__)
#endif


class ini_file;
class log
{
	friend class log_stream;
public:
	typedef void (*handler_t)(int level, const char* const msg);

	static void out					(LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line, const char* format_string, ...);
	static void direct_out			(LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line, const char* format_string, ...);

	//开启/关闭某个级别的日志
	static void enable				(LOG_LEVEL level);
	static void disable				(LOG_LEVEL level);

	//设置日志输出。 //举例：log::set_log_output(LOG_LEVEL_DEBUG, LOG_OUTPUT_CONSOLE | LOG_OUTPUT_FILE);
	static void set_log_output		(LOG_LEVEL level, byte output		);
	static void set_log_level_info	(LOG_LEVEL level, log_level& info	);
	static void hide_console		(LOG_LEVEL level);
	static void hide_console_all	();
	static void show_console		(LOG_LEVEL level);
	static void show_console_all	();
	static void set_file_quick_flush(bool value) { log::inst().m_globalfile.set_quick_flush(value); }
	static void prevent_frequent	(LOG_LEVEL level, bool value);
	static void	enable_thread_mode	();

	//add external cust handler.
	//example : 
	//			
	//	#include "scl/log.h"
	//	void scl_log_handler(int level, const char* const msg)
	//	{
	//		switch (level)
	//		{
	//		case scl::LOG_LEVEL_VERBOSE	: mylog_verbose	(msg);	break;
	//		case scl::LOG_LEVEL_DEBUG	: mylog_debug	(msg);	break;
	//		case scl::LOG_LEVEL_INFO	: mylog_info	(msg);	break;
	//		case scl::LOG_LEVEL_WARN	: mylog_warn	(msg);	break;
	//		case scl::LOG_LEVEL_ERROR	: mylog_error	(msg);	break;
	//		case scl::LOG_LEVEL_FATAL	: mylog_fatal	(msg);	break;
	//		default: { LordLogInfo(msg); assert(false);	}
	//		};
	//	}
	//
	//	int main()
	//	{
	//		scl::log::add_handler(scl_log_handler);
	//		log_debug("test");
	//		return 0;
	//	}
	static void add_handler			(handler_t h);
	static void remove_handler		(handler_t h);

	//如果想设置某个线程的日志缓冲区的大小，必须在线程刚刚开始执行的时候调用init_in_thread指定日志缓冲区大小
	static void	init_in_thread		(const int log_buffer_size) { log::inst()._find_thread_log(log_buffer_size); }
	
public:
	log();
	~log();

	static log&		inst				();
	static void		release				();

private:
	void			_init				();
	void			_init_level			(ini_file& cfg, LOG_LEVEL level, const char* const name);

	static void		_log_handler		(const log_header& header, const char* const s, int thread_id);
	static void*	_flush_thread_func	(void* p, int* s);
	static void		_prepare_log_header	(log_header& header, LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line);
	static void		_out_string			(log_header& header, const char* const slog);
	//void			_sendlog			(log_header& header, const char* format_string, va_list args);
	void			_sendlog			(log_header& header, const char* log_string);
	int				_find_thread_log	(const int log_buffer_size = MAX_THREAD_LOG_BUFFER_SIZE);
	void			_flush_all			();

	// log file
	static void		_open_log_file		(log_file& file, const char* const filename, bool split);
	static void		_try_open_index_file(log_file& file, const char* const exename, int start_index);
	static void		_insert_date_dir	(stringPath& filename, uint64 _time);
    static const char* const _get_cfg_filename();

private:
	array<log_level, LOG_LEVEL_COUNT>			m_levels;
	log_file									m_globalfile;		//全局日志文件
	log_file									m_files[LOG_LEVEL_COUNT];
	array<thread_log, MAX_THREAD_LOG_COUNT>		m_thread_logs;
	thread										m_flush_thread;
	volatile int								m_using_thread_mode;
	array<handler_t, MAX_LOG_HANDLER_COUNT>		m_handlers;
};

class log_stream
{
public:
	log_stream(LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line);

	class end_of_log{};

	log_stream& operator<<(void* i)	{ m_tempLog.format_append("%x", i);	return *this; }
	log_stream& operator<<(int i)	{ m_tempLog.format_append("%d", i);				return *this; }
	log_stream& operator<<(uint i)	{ m_tempLog.format_append("%u", i);				return *this; }
	log_stream& operator<<(int8 i)	{ m_tempLog.format_append("%d", i);				return *this; }
	log_stream& operator<<(uint8 i)	{ m_tempLog.format_append("%u", i);				return *this; }
	log_stream& operator<<(int16 i)	{ m_tempLog.format_append("%d", i);				return *this; }
	log_stream& operator<<(uint16 i) { m_tempLog.format_append("%u", i);				return *this; }
	log_stream& operator<<(int64 i)	{ m_tempLog.format_append("%lld", i);			return *this; }
	log_stream& operator<<(uint64 i)	{ m_tempLog.format_append("%llu", i);			return *this; }
	log_stream& operator<<(bool i)	{ m_tempLog.format_append("%d", static_cast<int>(i)); return *this; }
	log_stream& operator<<(float i)	{ m_tempLog.format_append("%f", i);				return *this; }
	log_stream& operator<<(double i) { m_tempLog.format_append("%f", i);				return *this; }
	log_stream& operator<<(const char* const s) { m_tempLog += s;					return *this; }
	log_stream& operator<<(const scl::time t) { m_tempLog.format_append("%d-%d-%d %d-%d-%d", 
		t.year, t.month, t.day, t.hour, t.minute, t.second);  return *this;	}

	template <int N>
	log_stream& operator<<(const scl::string<N>& s) { m_tempLog += s.c_str(); return *this; }
	log_stream& operator<<(end_of_log&)
	{
		log::_out_string(m_tempHeader, m_tempLog.c_str());
		m_tempLog.clear();
		return *this;
	}

private:
	log_header										m_tempHeader;
	scl::string<MAX_LOG_STRING_LENGTH>				m_tempLog;
};

extern log_stream::end_of_log endl;

//紧急日志
//某些情况下，例如assert或者日志系统down掉的时候，需要使用紧急日志写入一个文件
//紧急文件名为urgency.log
//如果该文件快速增长，说明进程已经进入非常严重的错误状态，需要终止
void urgency_log(const char* const s, const char* const filename="error.log");


} //namespace scl


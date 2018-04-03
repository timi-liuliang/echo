#pragma once

#include "scl/string.h"

namespace scl {

//线程日志在线程消失后，等待THREAD_LOG_CLOSE_DELAY毫秒后关闭，
//原因是buffer中可能还有未能写入的日志，等所有日志写完后，再关闭thread_log
const uint THREAD_LOG_CLOSE_DELAY		= 3 * 1000;		//单位：毫秒
const int  THREAD_LOG_CHECK_INTERVAL	= 5 * 1000;		//单位：毫秒

//一条日志的最大字符串长度
const int MAX_LOG_STRING_LENGTH			= 1024 * 8;

//日志缓冲区的大小。
//每当“需要打印到文件的日志字符串的长度”超过这个缓冲区大小时，就会向文件写入日志
const int LOG_CACHE_BUFFER_SIZE			= 1024 * 32;

const int FREQUENT_LOG_MAX_COUNT		= 1024;	//缓存频繁日志的个数上限
const int FREQUENT_LOG_TIMES_LIMIT		= 16;	//单条日志刷新次数上限，超过这个上线，视为频繁日志，将被过滤掉
const int FREQUENT_LOG_CLEAR_INTERVAL	= 1 * 60 * 1000; //频繁日志清空间隔，单位：毫秒

const int MAX_LOG_FILE_SIZE				= 512 * 1024 * 1024; //日志文件大小上限

//一个线程的日志缓冲区大小，如果超过这个缓冲区大小，日志系统就会报错
//#ifdef SCL_WIN
const int MAX_THREAD_LOG_BUFFER_SIZE	= 32 * 1024;
//#endif
//#ifdef SCL_LINUX
//const int MAX_THREAD_LOG_BUFFER_SIZE	= 128 * 1024 * 1024;
//#endif

//日志线程最大数量，如果需要打印日志的线程数大于这个数量，日志系统就会报错
#ifdef SCL_WIN
const int MAX_THREAD_LOG_COUNT			= 32;
#endif
#ifdef SCL_LINUX
const int MAX_THREAD_LOG_COUNT			= 256;
#endif
#ifdef SCL_APPLE
const int MAX_THREAD_LOG_COUNT			= 64;
#endif
#ifdef SCL_ANDROID
const int MAX_THREAD_LOG_COUNT			= 64;
#endif
#ifdef SCL_HTML5
const int MAX_THREAD_LOG_COUNT			= 32;
#endif

const int MAX_LOG_HANDLER_COUNT			= 256;

//并不严格的buffer检查
const byte	BEGIN_MARK	= 0xFE;
const byte	END_MARK	= 0xFF;

struct log_header
{
	byte		begin_mark;
	byte		format;
	byte		level;
	int			len;
	int			line;
	uint64		time;
	int			millisecond;
	string256	filename;
	string256	function;
	log_header() : begin_mark(0), format(0), level(0), len(0), line(0), time(0), millisecond(0) {}
};

//日志等级，枚举值越大情况越严重
enum LOG_LEVEL
{
	//从上到下严重程度依次增高
	LOG_LEVEL_INVALID = -1,	//

	LOG_LEVEL_VERBOSE,	//日志等级：冗余信息
	LOG_LEVEL_DEBUG,	//日志等级：调试
	LOG_LEVEL_INFO,		//日志等级：提示
	LOG_LEVEL_WARN,		//日志等级：警告
	LOG_LEVEL_ERROR,	//日志等级：错误
	LOG_LEVEL_FATAL,	//日志等级：致命
	//LOG_LEVEL_NET,		//日志等级: net.

	LOG_LEVEL_USER1 = 11,//日志等级：用户
	LOG_LEVEL_USER2,	//日志等级：用户
	LOG_LEVEL_USER3,	//日志等级：用户
	LOG_LEVEL_USER4,	//日志等级：用户
	LOG_LEVEL_USER5,	//日志等级：用户
	LOG_LEVEL_USER6,	//日志等级：用户
	LOG_LEVEL_USER7,	//日志等级：用户
	LOG_LEVEL_USER8,	//日志等级：用户

	LOG_LEVEL_COUNT,		//日志种类数量
};

//日志输出方式
enum LOG_OUTPUT
{
	LOG_OUTPUT_CONSOLE	= 0x01,	//打印在屏幕上
	LOG_OUTPUT_FILE 	= 0x02,	//打印到本地文件中
	//LOG_OUTPUT_REMOTE	= 0x04,	//打印到远端socket
};

//日志格式
enum LOG_FORMAT
{
	LOG_FORMAT_NONE			= 0,

	LOG_FORMAT_TIME			= 0x01,
	LOG_FORMAT_LINE_FEED	= 0x02,
	LOG_FORMAT_LEVEL		= 0x04,
	LOG_FORMAT_FUNCTION		= 0x08,
	LOG_FORMAT_FILENAME		= 0x10,
	LOG_FORMAT_MILLISECOND	= 0x20,

	LOG_FORMAT_ALL			= 0xFF,
};

struct log_level
{
	byte		output;
	byte		format;
	bool		enable;
	bool		prevent_frequent;
	bool		quick_flush;
	string8		show;
	//string32	remote;
	//int			remote_socket;
	string64	filename;
	bool		date_dir;
	bool		split;

	log_level() : output(0), format(0), enable(false), prevent_frequent(true), quick_flush(false), date_dir(false), split(false) {}

	void clear	() { output = 0; format = 0; enable = false; show = "";  }
	void set	(
		const char* show, 
		byte		output, 
		byte		format, 
		bool		prevent_frequent	= true,
		bool		quick_flush			= false,
		bool		enable				= true,
		bool		date_dir			= false,
		const char* filename			= NULL,
		bool		split				= true) 
	{ 
		this->output				= output;
		this->format				= format; 
		this->enable				= enable;
		this->prevent_frequent	= prevent_frequent;
		this->quick_flush		= quick_flush;
		this->show				= show; 
		this->date_dir			= date_dir;
		if (NULL != filename)
			this->filename		= filename;
		this->split				= split;
	}
};

//过于频繁的日志
struct frequent_log
{
	string256	filename;
	int			line;
	int			times;
	
	frequent_log() { clear(); }

	void clear()
	{
		filename.clear();
		line		= 0;
		times	= 0;
	}
};



} //namespace scl

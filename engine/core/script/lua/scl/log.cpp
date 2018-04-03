////////////////////////////////////////////////////////////////////////////////
//	log.cpp
//	日志类
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#include <stdarg.h>

#include "scl/log.h"
#include "scl/file.h"
#include "scl/time.h"
#include "scl/assert.h"
#include "scl/string.h"
#include "scl/wstring.h"
#include "scl/thread.h"
#include "scl/ini_file.h"
#include "scl/backtrace.h"
#include "scl/directory.h"
#include "scl/frame_lock.h"
#include "scl/path.h"

#include <stdio.h>

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <wchar.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#endif

#ifdef SCL_WIN
#include <Windows.h> // for OutputDebugStringA
#endif

#ifdef SCL_ANDROID
#include "scl/android.h"
#include <android/log.h>
#endif

namespace scl {

log_stream::end_of_log endl;

//ptr<log>				g_pinst(NULL);
log*					g_pinst = NULL;
volatile	uint		g_init_started = 0;
//#define log_inst() ((g_pinst.p == NULL) ? log::inst() : *(g_pinst.p))

inline log& log_inst()
{
	return ((g_pinst == NULL) ? log::inst() : *(g_pinst));
}

log::log() : m_using_thread_mode(0)
{
}


log::~log()
{
			
}

void log::_prepare_log_header(log_header& header, LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line)
{
	//文件名（去掉路径）
	stringPath fullfilename = filename;
	extract_filename(fullfilename.pstring());

	if (log_format == -1)
		log_format = log_inst().m_levels[level].format;

	header.format	= log_format;
	header.level	= level;
	header.line		= line;
	header.time		= SCL_TIME;
	header.millisecond = SCL_MILLISECOND;
	header.filename = fullfilename.c_str();
	header.function	= function;
}

void log::out(LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line, const char* format_string, ...)
{
	va_list arg;
	va_start(arg, format_string);
	log_header header;
	_prepare_log_header(header, level, log_format, filename, function, line);

	scl::string<MAX_LOG_STRING_LENGTH> strlog;
	strlog.format_arg(format_string, arg);
	_out_string(header, strlog.c_str());
	va_end(arg);
}

void log::direct_out(LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line, const char* format_string, ...)
{
	va_list arg;
	va_start(arg, format_string);
	scl::string<MAX_LOG_STRING_LENGTH> strlog;
	strlog.format_arg(format_string, arg);
	va_end(arg);

	log_header header;
	_prepare_log_header(header, level, log_format, filename, function, line);
	_log_handler(header, strlog.c_str(), thread::self());
}

//void log::_sendlog(log_header& header, const char* format, va_list args)
//{
//	log_level& info = m_levels[header.level];
//	if (!info.enable)
//		return;
//
//	int thread_log_index = _find_thread_log();
//	if (-1 == thread_log_index)
//	{
//		assert(false); //所有日志都被占用了
//		return;
//	}
//
//	scl::string<MAX_LOG_STRING_LENGTH> strlog;
//	strlog.format_arg(format, args);
//	m_thread_logs[thread_log_index].write(header, strlog.c_str());
//}


void log::_sendlog(log_header& header, const char* log_string)
{
	log_level& info = m_levels[header.level];
	if (!info.enable)
		return;

	int thread_log_index = _find_thread_log();
	if (-1 == thread_log_index)
	{
		assert(false); //所有日志都被占用了
		return;
	}

	m_thread_logs[thread_log_index].write(header, log_string);
}


void log::set_log_output(LOG_LEVEL level, byte output)
{
	log_inst().m_levels[level].output = output;
}

void log::hide_console(LOG_LEVEL level)
{
	byte& output = log_inst().m_levels[level].output;
	output = output & ~LOG_OUTPUT_CONSOLE;
}

void log::hide_console_all()
{
	for (int i = 0; i < LOG_LEVEL_COUNT; ++i)
	{
		hide_console(static_cast<LOG_LEVEL>(i));
	}
}

void log::show_console(LOG_LEVEL level)
{
	byte& output = log_inst().m_levels[level].output;
	output = output | LOG_OUTPUT_CONSOLE;
}

void log::show_console_all()
{
	for (int i = 0; i < LOG_LEVEL_COUNT; ++i)
	{
		show_console(static_cast<LOG_LEVEL>(i));
	}
}


void log::set_log_level_info(LOG_LEVEL level, log_level& info)
{
	log_inst().m_levels[level] = info;
}


void log::enable(LOG_LEVEL level)
{	
	log_inst().m_levels[level].enable = true;
}

void log::disable(LOG_LEVEL level)
{	
	log_inst().m_levels[level].enable = false;
}

void log::_open_log_file(log_file& _file, const char* const logname, bool split)
{
	//如果文件所在路径发生变化，关闭原有文件
	stringPath currentPath = _file.filename();
	extract_path(currentPath.pstring());
	stringPath newPath = logname;
	extract_path(newPath.pstring());
	if (currentPath != newPath)
	{
		_file.close();
	}

	if (!_file.is_open()) //第一次打开文件，需要顺序查找所有的文件
	{	
		_try_open_index_file(_file, logname, -1);
	}
	else  //文件已经打开了
	{
		//如果无需拆分，则跳过拆分逻辑，直接返回即可
		if (!split)
			return;
		//文件大小还没超过MAX_LOG_FILE_SIZE，可以继续写入，所以返回即可
		if (_file.size() < MAX_LOG_FILE_SIZE)
			return;

		stringPath filename = _file.filename();

		//从当前文件名中截取index
		int start_index = static_cast<int>(::strlen(logname));
		int end_index	= filename.length() - sizeof(".log") + 1;
		string8 str_index;
		filename.substr(start_index, end_index - start_index, str_index.c_str(), str_index.capacity());

		//计算新的index
		int new_index = str_index.empty() ? 1 : (str_index.to_int() + 1);

		_try_open_index_file(_file, logname, new_index);
	}
}

int log::_find_thread_log(const int log_buffer_size)
{
	assert(log_buffer_size > static_cast<int>(sizeof(log_header)));
	const int thread_id = thread::self();
	for (int i = 0; i < MAX_THREAD_LOG_COUNT; ++i)
	{
		if (m_thread_logs[i].is_thread(thread_id))
			return i;
	}

	//如果没有对应的日志，找个空日志
	for (int i = 0; i < MAX_THREAD_LOG_COUNT; ++i)
	{
		thread_log& l = m_thread_logs[i];
		if (l.ready_to_work())
		{
			if (!l.has_init())
			{
				l.init(&log::_log_handler, m_levels.c_array(), m_levels.size(), log_buffer_size);
			}
			l.set_thread_id(thread_id);
			return i;	//找到了空日志,写入
		}
	}
	return -1;
}

void log::_insert_date_dir(stringPath& _name, uint64 _time)
{
	//根据日期确定日志的打印目录
	scl::time logtime(_time);
	string256 timedir;
	timedir.format("%d%02d%02d", logtime.year, logtime.month, logtime.day);

	//需要将filename的路径拆开，然后将time string作为中间目录插入路径
	stringPath path = _name.c_str();
	extract_path(path.pstring());
			
	//无路径的文件名
	stringPath filename = _name.c_str();
	extract_filename(filename.pstring());

	//检查日期目录是否存在
	_name.format("%s/%s", path.c_str(), timedir.c_str());
	if (!file::exists(_name.c_str()))
	{
		directory::create_dir(_name.c_str());
	}

	//完整文件名
	_name += "/";
	_name += filename.c_str();
}

void log::_log_handler(const log_header& header, const char* const s, int thread_id)
{
	log_level& info = log_inst().m_levels[header.level];
	if (!info.enable)
		return;

	scl::string<MAX_LOG_STRING_LENGTH> strlog;
	strlog.clear();
	if (header.format & LOG_FORMAT_TIME)
	{
		bool withMillisecond = ((header.format & LOG_FORMAT_MILLISECOND) != 0);
		scl::time t(header.time, header.millisecond);
		string32 strtime;
		t.to_string(strtime, true, withMillisecond);
		strlog += strtime.c_str();
	}
	if (header.format & LOG_FORMAT_LEVEL)
	{
		strlog.format_append(" [%s]", info.show.c_str());
	}
	if (header.format & LOG_FORMAT_FILENAME)
	{
		strlog.format_append(" [%s:%d]", header.filename.c_str(), header.line);
	}
	if (header.format & LOG_FORMAT_FUNCTION)
	{
		strlog.format_append(" [%s]", header.function.c_str());
	}
	
	if (!strlog.empty())
		strlog += " ";

	//log本体
	strlog += s;

	if (header.format & LOG_FORMAT_LINE_FEED)
	{
		strlog += "\n";
	}

	////////////////////////////////////
	//开始输出
	////////////////////////////////////
	if (info.output & LOG_OUTPUT_CONSOLE)
	{
#ifdef SCL_WIN
		OutputDebugStringA(strlog.c_str());
#endif

#ifdef SCL_ANDROID
		string32 android_log_level;
		android_log_level.format("scl.%s", info.show.c_str());
		__android_log_print(ANDROID_LOG_INFO, android_log_level.c_str(), "%s", strlog.c_str());
#endif

		printf("%s", strlog.c_str());

#if defined(SCL_LINUX) || defined(SCL_APPLE)
		fflush(stdout);
#endif
	}


	if (info.output & LOG_OUTPUT_FILE)
	{
		//__android_log_print(ANDROID_LOG_INFO, "scl", "scl lord game _log_handler output file being");
		if (info.filename.empty()) //全局文件
		{
			stringPath exename;
            
#ifdef SCL_APPLE
            get_home_path(exename.c_str(), exename.capacity());
            exename += "/Documents/log";
#elif defined(SCL_ANDROID)
		//__android_log_print(ANDROID_LOG_INFO, "scl", "scl lord game _log_handler get exe name begin");
			get_android_external_files_dir(exename.c_str(), exename.capacity());
		//__android_log_print(ANDROID_LOG_INFO, "scl", "scl lord game _log_handler exename1 = %s", exename.c_str());
			if (exename.empty())
				get_android_files_dir(exename.c_str(), exename.capacity());
			if (exename.empty())
				exename = "/sdcard";
			exename += "/log";
			//__android_log_print(ANDROID_LOG_INFO, "scl.info : scl log filename=[%s] quick_flush = %d",  exename.c_str(), info.quick_flush);
#else
			get_exe_name(exename.pstring());
#endif

			//是否写入日期目录
			if (info.date_dir)
				_insert_date_dir(exename, header.time);

			//打开文件并写入
			log_inst()._open_log_file(log_inst().m_globalfile, exename.c_str(), info.split);
			log_inst().m_globalfile.write(info.quick_flush, strlog.c_str());
		}
		else
		{
			log_file& _file = log_inst().m_files[header.level];

			stringPath filename;
#ifdef SCL_APPLE
            get_home_path(filename.c_str(), filename.capacity());
            filename += "/Documents/";
			filename += info.filename.c_str(); 
#elif defined(SCL_ANDROID)
			get_android_external_files_dir(filename.c_str(), filename.capacity());
			filename += "/";
			filename += info.filename.c_str(); 
#else
			filename = info.filename.c_str();
#endif

			//是否写入日期目录
			if (info.date_dir)
				_insert_date_dir(filename, header.time);

			//打开日志文件并写入
			log_inst()._open_log_file(_file, filename.c_str(), info.split);
			_file.write(info.quick_flush, strlog.c_str());
		}
	}

	for (int i = 0; i < log_inst().m_handlers.size(); ++i)
	{
		handler_t f = log_inst().m_handlers[i];
		if (NULL == f)
			return;
		f(header.level, s);
	}
	//if (info.output & LOG_OUTPUT_REMOTE)
	//{
	//	//TODO 如果strlog中有
	//	if (info.remote_socket != -1)
	//	{
	//		string128 hostname;
	//		gethostname(hostname.c_str(), hostname.capacity());

	//		scl::string<MAX_LOG_STRING_LENGTH> remotelog;
	//		remotelog.format("<182>%s  %s", hostname.c_str(), strlog.c_str());
	//		int sendbytes = send(info.remote_socket, remotelog.c_str(), remotelog.length(), 0); 
	//		if (sendbytes <= 0)
	//		{
	//			urgency_log("log output remote error!");
	//		}
	//	}
	//}
}

void* log::_flush_thread_func(void* p, int* s)
{
	assert(s);
	frame_lock fl;
	while (1)
	{
SCL_ASSERT_TRY
{
		fl.start();
		log_inst()._flush_all();
		if (*s == thread::SIGNAL_STOP)
		{
			log_inst()._flush_all();
			break;
		}
		fl.wait();
#ifdef SCL_FORCE_SLEEP
		scl::sleep(1);
#endif
		scl::sleep(0);

}
SCL_ASSERT_CATCH
{
	scl::sleep(1);
	continue;
}

	}
	return NULL;
}

void log::_init()
{
	m_thread_logs.resize(MAX_THREAD_LOG_COUNT);
	m_levels.resize(LOG_LEVEL_COUNT);

	byte output = LOG_OUTPUT_CONSOLE | LOG_OUTPUT_FILE;
	byte format = LOG_FORMAT_LINE_FEED | LOG_FORMAT_TIME | LOG_FORMAT_MILLISECOND | LOG_FORMAT_LEVEL;

#if defined(SCL_APPLE)
	output = LOG_OUTPUT_CONSOLE;
	format = LOG_FORMAT_LINE_FEED | LOG_FORMAT_TIME | LOG_FORMAT_MILLISECOND | LOG_FORMAT_LEVEL;
#endif
#if defined(SCL_ANDROID)
	output = LOG_OUTPUT_CONSOLE;
	format = LOG_FORMAT_TIME | LOG_FORMAT_MILLISECOND;
#endif

	//初始化默认值
	m_levels[LOG_LEVEL_VERBOSE	].set("verbose",	output, format	);
	m_levels[LOG_LEVEL_DEBUG	].set("debug",		output, format	);
	m_levels[LOG_LEVEL_INFO		].set("info ",		output, format	);
	m_levels[LOG_LEVEL_WARN		].set("warn ",		output, format	);
	m_levels[LOG_LEVEL_ERROR	].set("error",		output, format	);
	m_levels[LOG_LEVEL_FATAL	].set("fatal",		output, format	);
	//m_levels[LOG_LEVEL_NET		].set("net",		output, LOG_FORMAT_LINE_FEED | LOG_FORMAT_TIME | LOG_FORMAT_LEVEL);

	//user日志初始化
	for (int i = LOG_LEVEL_USER1; i < LOG_LEVEL_COUNT; ++i)
	{
		string16 username;
		username.format("user%d", i - LOG_LEVEL_USER1 + 1);
		m_levels[i].set(username.c_str(), LOG_OUTPUT_FILE, LOG_FORMAT_LINE_FEED | LOG_FORMAT_LEVEL);
	}

	
	m_globalfile.init(LOG_CACHE_BUFFER_SIZE);

	//m_flush_thread.start(_flush_thread_func);

	ini_file cfg;
    const char* const filename = _get_cfg_filename();

#ifdef SCL_ANDROID
	scl::android_file af;
	if (af.open(filename))
		cfg.open_buffer(static_cast<const char*>(af.get_buffer()), af.size());
	else
		__android_log_print(ANDROID_LOG_INFO, "scl", "scl log::_init cfg open file failed. filename = %s", filename);
#else
	if (file::exists(filename))
		cfg.open(filename, "rb");
#endif

	if (cfg.is_open())
	{
		_init_level(cfg, LOG_LEVEL_VERBOSE,	"verbose");
		_init_level(cfg, LOG_LEVEL_DEBUG,	"debug");
		_init_level(cfg, LOG_LEVEL_INFO,	"info");
		_init_level(cfg, LOG_LEVEL_WARN,	"warn");
		_init_level(cfg, LOG_LEVEL_ERROR,	"error");
		_init_level(cfg, LOG_LEVEL_FATAL,	"fatal");
		//_init_level(cfg, LOG_LEVEL_NET,		"net");

		for (int i = LOG_LEVEL_USER1; i < LOG_LEVEL_COUNT; ++i)
		{
			string16 username;
			username.format("user%d", i - LOG_LEVEL_USER1 + 1);
			_init_level(cfg, static_cast<LOG_LEVEL>(i), username.c_str());
		}
	}
}

void log::_init_level(ini_file& cfg, LOG_LEVEL level, const char* const name)
{
	bool prevent_frequent_log	= cfg.get_int(name, "prevent_frequent", 1)	!= 0;
	bool enable					= cfg.get_int(name, "enable",			1)	!= 0;
	bool quick_flush			= cfg.get_int(name, "quick_flush",		0)	!= 0;
	bool date_dir				= cfg.get_int(name, "date_dir",			0)	!= 0;
	bool split					= cfg.get_int(name, "split",			1)	!= 0;

	byte output = 0;
	if (cfg.get_int(name, "output_console", 1))
		output |= LOG_OUTPUT_CONSOLE;
	if (cfg.get_int(name, "output_file", 1))
		output |= LOG_OUTPUT_FILE;
	//if (cfg.get_int(name, "output_remote", 1))
	//	output |= LOG_OUTPUT_REMOTE;

	byte format = 0;
	if (cfg.get_int(name, "format_time", 1))
		format |= LOG_FORMAT_TIME;
	if (cfg.get_int(name, "format_millisecond", 1))
		format |= LOG_FORMAT_MILLISECOND;
	if (cfg.get_int(name, "format_line_feed", 1))
		format |= LOG_FORMAT_LINE_FEED;
	if (cfg.get_int(name, "format_level", 1))
		format |= LOG_FORMAT_LEVEL;
	if (cfg.get_int(name, "format_function", 0))
		format |= LOG_FORMAT_FUNCTION;
	if (cfg.get_int(name, "format_filename", 0))
		format |= LOG_FORMAT_FILENAME;		

	//string32 remote;
	//cfg.get_string(name, "remote", remote.c_str(), remote.capacity());

	string64 filename;
	cfg.get_string(name, "filename", filename.c_str(), filename.capacity());

	//TODO 多个level使用同名文件怎么办？
	if (!filename.empty())
	{
		m_files[level].init	(LOG_CACHE_BUFFER_SIZE);
	}
	m_levels[level].set(name, output, format, prevent_frequent_log, quick_flush, enable, date_dir, filename.c_str(), split);

//	if (remote.empty())
//		return;
//
//	//创建远程socket
//	char* tokcontext = NULL;
//	string32 ip			= scl_strtok(remote.c_str(), ":", &tokcontext);
//	string16 strport	= scl_strtok(NULL, ":", &tokcontext);
//	int port = strport.to_int();
//#ifdef SCL_WIN
//	WSAData wsa;
//	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
//	{
//		assertf(false, "WSAStartup failed!");
//		return;
//	}
//#endif
//	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (-1 == sock)
//	{
//		urgency_log("log remote server socket failed!");
//		return;
//	}
//	sockaddr_in addr;
//	addr.sin_family			= AF_INET;
//	addr.sin_port			= htons(port);
//#ifdef SCL_WIN
//#pragma warning(disable:4996) 
//#endif
//	addr.sin_addr.s_addr	= inet_addr(ip.c_str());
//#ifdef SCL_WIN
//#pragma warning(default:4996) 
//#endif
//	int result = connect(sock, (sockaddr*)(&addr), sizeof(addr));
//	if (result != 0)
//	{
//		urgency_log("log remote server socket failed!");
//		return;
//	}
//	m_levels[level].remote_socket = sock;
}

log& log::inst()
{
	if (NULL == g_pinst)
	{
		if (compare_and_swap(&g_init_started, 0, 1))
		{
			g_pinst = new log;
			g_pinst->_init();
			//g_pinst.p = p;
		}
		//等待日志初始化成功
		while (g_pinst == NULL)
			sleep(1);
	}
	if (NULL == g_pinst)
	{
		assert(0);
		throw 1;
	}
	return *(g_pinst);
}

void log::prevent_frequent(LOG_LEVEL level, bool value)
{
	log_inst().m_levels[level].prevent_frequent = value;
}

void log::_try_open_index_file(log_file& _file, const char* const exename, int start_index)
{
	stringPath filename;
	if (start_index == -1)
	{
		start_index = 0;
		filename.format("%s.log", exename);
	}
	else
		filename.format("%s%d.log", exename, start_index);

	if (_file.is_open())
		_file.close();

	while (1)
	{
		//文件不存在，直接创建即可
		if (!file::exists(filename.c_str()))
		{
			_file.open(filename.c_str());
			break;
		}

		//文件存在，但size小于MAX_LOG_FILE_SIZE，也可以直接打开
		_file.open(filename.c_str());
		if (_file.size() < MAX_LOG_FILE_SIZE)
			break;

		//文件存在，但是大小超过了MAX_LOG_FILE_SIZE，尝试打开新的文件
		_file.close();
		++start_index;
		filename.format("%s%d.log", exename, start_index);
	}
}

void log::release()
{
	if (NULL == g_pinst)
		return;
	if (g_pinst->m_flush_thread.is_running())
	{
		g_pinst->m_flush_thread.send_stop_signal();
		if (!g_pinst->m_flush_thread.wait())
		{
			assert(0);
		}
	}
	delete g_pinst;
	g_pinst = NULL;
	g_init_started = 0;
}

void log::enable_thread_mode()
{
	if (compare_and_swap(&log_inst().m_using_thread_mode, 0, 1)) //进入多线程模式
		log_inst().m_flush_thread.start(_flush_thread_func);	
}

void log::_flush_all()
{
	for (int i = 0; i < MAX_THREAD_LOG_COUNT; ++i)
	{
		if (log_inst().m_thread_logs[i].has_data())
			log_inst().m_thread_logs[i].flush();
	}
}

void log::_out_string(log_header& header, const char* const slog)
{
	if (log_inst().m_using_thread_mode == 0)
	{
		if (thread::main_thread_id() == thread::self())
			_log_handler(header, slog, thread::self());
		else
		{
			enable_thread_mode();
			log_inst()._sendlog(header, slog);
		}
	}
	else
		log_inst()._sendlog(header, slog);
}

const char* const log::_get_cfg_filename()
{
    static stringPath filename;
    if (filename.empty())
    {
#ifdef SCL_APPLE
        get_application_path(filename.c_str(), filename.max_size());
        filename += "log.ini";
#else
        filename = "log.ini";
#endif
    }
    return filename.c_str();
}

void log::add_handler(log::handler_t f)
{
	if (NULL == f)
		return;
	log_inst().m_handlers.push_back(f);
}

void log::remove_handler(handler_t h)
{
	log_inst().m_handlers.erase_element(h);
}

log_stream::log_stream(LOG_LEVEL level, int log_format, const char* const filename, const char* const function, const int line)
{
 	assert(level > LOG_LEVEL_INVALID && level < LOG_LEVEL_COUNT);
 	assert(m_tempLog.empty());

	log::_prepare_log_header(m_tempHeader, level, log_format, filename, function, line);
}

// LogStream::~LogStream()
// {
// 	if (!m_tempLog.empty())
// 		log::_out_string(m_tempHeader, m_tempLog.c_str());
// 
// 	m_tempLog.clear();
// }

void urgency_log(const char* const s, const char* const filename)
{
	__inner_error_log_mutex.lock();

#ifdef SCL_WIN
#pragma warning(disable: 4996)
#endif
	FILE* pf = fopen(filename, "ab");
#ifdef SCL_WIN
#pragma warning(default: 4996)
#endif
	if (NULL != pf)
	{
		scl::time t;
		t.now();
		string32 strtime;
		t.to_string(strtime);
		fprintf(pf, "===================[urgency]=================\n%s\n", strtime.c_str());
		fprintf(pf, "%s\n", s);
		fclose(pf);
	}

	__inner_error_log_mutex.unlock();
}


}  //namespace scl


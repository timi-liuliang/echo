////////////////////////////////////////////////////////////////////////////////
//	log.cpp
//	»’÷æ¿‡
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////

#include "scl/log_file.h"
#include "scl/log.h" //include MAX_LOG_STRING_LENGTH
#include "scl/assert.h"

namespace scl {

log_file::log_file(const int maxSize)
{
	m_cache.init(new char[maxSize], maxSize, "");
}

log_file::~log_file()
{
	flush(true);
	close();
	m_cache.free();
	m_cache_len = 0;
}


void log_file::open(const char* const filename)
{
	m_filename = filename;

	if (m_file.is_open())
		return;

	if (!m_file.open(filename, "a+b"))
	{
		urgency_log("log_file::open error! fopen failed!");
		return;
	}
}

void log_file::write(const char* format, ...)
{
	if (m_filename.empty())
	{
		assert(false);
		return;
	}

	static scl::string<MAX_LOG_STRING_LENGTH> strlog;

	va_list arg;
	va_start(arg, format);
	strlog.format_arg(format, arg);
	va_end(arg);
	int strlog_len = strlog.length();

	if (m_cache_len + strlog_len >= m_cache.max_size())
		flush(m_quickflush);

#ifdef SCL_WIN
#pragma warning(push)
#pragma warning(disable:4996)
#endif

	strcat(m_cache.c_str(), strlog.c_str());

#ifdef SCL_WIN
#pragma warning(pop)
#endif

	m_cache_len += strlog_len;

	if (m_quickflush)
		flush(true);
}


void log_file::write(bool quick_flush, const char* format, ...)
{
	if (m_filename.empty())
	{
		assert(false);
		return;
	}

	static scl::string<MAX_LOG_STRING_LENGTH> strlog;

	va_list arg;
	va_start(arg, format);
	strlog.format_arg(format, arg);
	va_end(arg);

	int strlog_len = strlog.length();

	if (m_cache_len + strlog_len >= m_cache.max_size())
		flush(quick_flush);

#ifdef SCL_WIN
#pragma warning(push)
#pragma warning(disable:4996)
#endif

	strcat(m_cache.c_str(), strlog.c_str());

#ifdef SCL_WIN
#pragma warning(pop)
#endif

	m_cache_len += strlog_len;

	if (quick_flush)
		flush(true);
}

void log_file::flush(bool quick_flush)
{
	do 
	{
		if (m_cache.length() <= 0)
		{
			break;
		}
		if (m_filename.empty())
		{
			urgency_log("log_file::flush failed! file name is empty!");
			break;
		}
		if (!m_file.is_open())
		{
			if (!m_file.open(m_filename.c_str(), "a+b"))
			{
				urgency_log("log_file::flush failed! fopen failed!");
				break;
			}
		}
		m_file.write_string(m_cache.c_str(), m_cache.length());
	}
	while (0);

	m_cache.clear();
	m_cache_len = 0;
	if (quick_flush)
		m_file.flush();
	//m_file.close();
}

void log_file::close()
{
	flush(true);
	m_filename.clear();
	m_cache.clear();
	m_cache_len		= 0;
	m_quickflush	= false;
	if (m_file.is_open())
		m_file.close();
}

void log_file::init(const int cacheBufferSize)
{
	assert(m_cache.c_str() == NULL);
	m_cache.init(new char[cacheBufferSize], cacheBufferSize, "");
}

}  //namespace scl


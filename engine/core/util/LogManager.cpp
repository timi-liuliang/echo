#include "StringUtil.h"
#include "LogManager.h"
#include "AssertX.h"
#include <stdarg.h>

namespace Echo
{
	LogManager* LogManager::s_instance = NULL;
	// 获取单一实例
	LogManager* LogManager::instance()
	{
		if (NULL == s_instance)
		{
			s_instance = EchoNew(LogManager);
		}
		return s_instance;
	}

	// 替换实例
	void LogManager::replaceInstance(LogManager* inst)
	{
		EchoAssert(!s_instance);
		s_instance = inst;
	}

	// 构造函数
	LogManager::LogManager()
		: m_logLevel( Log::LL_INVALID)
	{

	}

	// 析构函数
	LogManager::~LogManager()
	{
	}

	// 添加日志
	bool LogManager::addLog( Log* pLog)
	{
		for ( size_t i = 0; i < m_logArray.size(); i++ )
		{
			if ( m_logArray[i] == pLog )
				return false;
		}

		m_logArray.push_back(pLog);

		return true;
	}

	// 根据名称获取日志
	Log* LogManager::getLog(const String& name) const
							{
		for( size_t i=0; i<m_logArray.size(); i++)
				{
			if( m_logArray[i]->getName() == name)
				return m_logArray[i];
				}

			return NULL;
	}

	// 移除日志
	void LogManager::removeLog( const String &name)
	{
		for( LogArray::iterator it=m_logArray.begin(); it!=m_logArray.end(); it++)
	{
			if( (*it)->getName() == name)
		{
				m_logArray.erase( it);
				return;
		}
	}
	}

	// 移除日志
	void LogManager::removeLog( Log* pLog)
	{
		for( LogArray::iterator it=m_logArray.begin(); it!=m_logArray.end(); it++)
		{
			if( (*it) == pLog)
			{
				m_logArray.erase( it);
				return;
			}
		}
	}

	// 输出日志
	void LogManager::logMessage(Log::LogLevel level, const char* formats, ...)
	{
        if ( Log::LL_INVALID != m_logLevel )
		{
            if ( level < m_logLevel )
                return;
		}

		if (!m_logArray.empty())
		{
			char szBuffer[4096];
			int bufferLength = sizeof(szBuffer);
			va_list args;
			va_start(args, formats);
			vsnprintf(szBuffer, bufferLength, formats, args);
			va_end(args);

			szBuffer[bufferLength - 1] = 0;

			for (LogArray::iterator it = m_logArray.begin(); it != m_logArray.end(); it++)
			{
				(*it)->logMessage(level, szBuffer);
			}
		}
	}

	void LogManager::logEvent(Log::LogLevel level, const std::wstring& message)
	{
		logMessage(level, "%s", StringUtil::WCS2MBS(message).c_str());
	}

	// 错误消息
	void LogManager::error(const char* msg)
	{
		logMessage(Log::LL_ERROR, msg);
	}

	// 警告
	void LogManager::warning(const char* msg)
	{
		logMessage(Log::LL_WARNING, msg);
	}

	// 信息
	void LogManager::info(const char* msg)
	{
		logMessage(Log::LL_INFO, msg);
	}
}

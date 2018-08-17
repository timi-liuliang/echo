#include "engine/core/util/StringUtil.h"
#include "Log.h"
#include "engine/core/util/AssertX.h"
#include <stdarg.h>

namespace Echo
{
	LogManager* LogManager::instance()
	{
		static LogManager* inst = EchoNew(LogManager);
		return inst;
	}

	LogManager::LogManager()
		: m_logLevel( LogOutput::LL_INVALID)
	{

	}

	LogManager::~LogManager()
	{
	}

	void LogManager::bindMethods()
	{
		//CLASS_BIND_METHOD(LogManager, error, "error");
		//CLASS_BIND_METHOD(LogManager, warning, "warning");
		//CLASS_BIND_METHOD(LogManager, info, "info");
	}

	bool LogManager::addLog( LogOutput* pLog)
	{
		for ( size_t i = 0; i < m_logArray.size(); i++ )
		{
			if ( m_logArray[i] == pLog )
				return false;
		}

		m_logArray.push_back(pLog);

		return true;
	}

	LogOutput* LogManager::getLog(const String& name) const
							{
		for( size_t i=0; i<m_logArray.size(); i++)
				{
			if( m_logArray[i]->getName() == name)
				return m_logArray[i];
				}

			return NULL;
	}

	void LogManager::removeLog( const String &name)
	{
		for(OutputArray::iterator it=m_logArray.begin(); it!=m_logArray.end(); it++)
	{
			if( (*it)->getName() == name)
		{
				m_logArray.erase( it);
				return;
		}
	}
	}

	void LogManager::removeLog( LogOutput* pLog)
	{
		for(OutputArray::iterator it=m_logArray.begin(); it!=m_logArray.end(); it++)
		{
			if( (*it) == pLog)
			{
				m_logArray.erase( it);
				return;
			}
		}
	}

	void LogManager::logMessage(LogOutput::LogLevel level, const char* formats, ...)
	{
        if ( LogOutput::LL_INVALID != m_logLevel )
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

			for (OutputArray::iterator it = m_logArray.begin(); it != m_logArray.end(); it++)
			{
				(*it)->logMessage(level, szBuffer);
			}
		}
	}

	void LogManager::logMessage(LogOutput::LogLevel level, const std::wstring& message)
	{
		logMessage(level, "%s", StringUtil::WCS2MBS(message).c_str());
	}

	void LogManager::error(const char* msg)
	{
		logMessage(LogOutput::LL_ERROR, msg);
	}

	void LogManager::warning(const char* msg)
	{
		logMessage(LogOutput::LL_WARNING, msg);
	}

	void LogManager::info(const char* msg)
	{
		logMessage(LogOutput::LL_INFO, msg);
	}
}

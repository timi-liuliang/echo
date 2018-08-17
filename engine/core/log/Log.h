#pragma once

#include "engine/core/base/object.h"
#include "LogOutput.h"

namespace Echo
{
	class LogManager : public Object
	{
		ECHO_SINGLETON_CLASS(LogManager, Object);

		typedef vector<LogOutput*>::type	OutputArray;

	public:
		~LogManager();

		// instance
		static LogManager* instance();

		// add log
		bool addLog(LogOutput* pLog);

		// get log by name
		LogOutput* getLog(const String& name) const;

		// remove log
		void removeLog( const String& name);
		void removeLog(LogOutput* pLog);
		void removeAllLog() { m_logArray.clear(); }

		// log level
		void setLogLeve(LogOutput::LogLevel level) { m_logLevel = level; }
		LogOutput::LogLevel getLogLevel() { return m_logLevel; }

		// output message
		void logMessage(LogOutput::LogLevel level, const char* formats, ...);
		void logMessage(LogOutput::LogLevel level, const std::wstring& message);

	public:
		// lua
		void error(const char* msg);
		void warning(const char* msg);
		void info(const char* msg);

	private:
		LogManager();

	protected:
		LogOutput::LogLevel	m_logLevel;		// 日志级别
		OutputArray			m_logArray;		// A list of all the logs the manager can access
	};
}

#define EchoLogDebug(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::LogOutput::LL_DEBUG, formats, ##__VA_ARGS__);
#define EchoLogInfo(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::LogOutput::LL_INFO, formats, ##__VA_ARGS__);
#define EchoLogWarning(formats, ...)	Echo::LogManager::instance()->logMessage(Echo::LogOutput::LL_WARNING, formats, ##__VA_ARGS__);
#define EchoLogError(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::LogOutput::LL_ERROR, formats, ##__VA_ARGS__);
#define EchoLogFatal(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::LogOutput::LL_FATAL, formats, ##__VA_ARGS__);

#pragma once

#include "engine/core/base/object.h"
#include "LogOutput.h"

namespace Echo
{
	class Log : public Object
	{
		ECHO_SINGLETON_CLASS(Log, Object);

		typedef vector<LogOutput*>::type	OutputArray;

	public:
		~Log();

		// instance
		static Log* instance();

		// add log
		bool addOutput(LogOutput* pLog);

		// get log by name
		LogOutput* getOutput(const String& name) const;

		// remove log
		void removeOutput( const String& name);
		void removeOutput(LogOutput* pLog);
		void removeAllOutput() { m_logArray.clear(); }

		// log level
		void setOutputLeve(LogOutput::Level level) { m_logLevel = level; }
		LogOutput::Level getOutputLevel() { return m_logLevel; }

		// output message
		void logMessage(LogOutput::Level level, const char* msg);
		void logMessage(LogOutput::Level level, const std::wstring& message);
		void logMessageExt(LogOutput::Level level, const char* formats, ...);

	public:
		// lua
		void error(const char* msg);
		void warning(const char* msg);
		void info(const char* msg);

	private:
		Log();

	protected:
		LogOutput::Level	m_logLevel;		// 日志级别
		OutputArray			m_logArray;		// A list of all the logs the manager can access
	};
}

#define EchoLogDebug(formats, ...)		Echo::Log::instance()->logMessageExt(Echo::LogOutput::LL_DEBUG, formats, ##__VA_ARGS__);
#define EchoLogInfo(formats, ...)		Echo::Log::instance()->logMessageExt(Echo::LogOutput::LL_INFO, formats, ##__VA_ARGS__);
#define EchoLogWarning(formats, ...)	Echo::Log::instance()->logMessageExt(Echo::LogOutput::LL_WARNING, formats, ##__VA_ARGS__);
#define EchoLogError(formats, ...)		Echo::Log::instance()->logMessageExt(Echo::LogOutput::LL_ERROR, formats, ##__VA_ARGS__);
#define EchoLogFatal(formats, ...)		Echo::Log::instance()->logMessageExt(Echo::LogOutput::LL_FATAL, formats, ##__VA_ARGS__);

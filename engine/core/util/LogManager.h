#pragma once

#include "Log.h"

namespace Echo
{
	/** 
	 * 日志系统 
	 */
	class LogManager
	{
		typedef vector<Log*>::type	LogArray;

	public:
		LogManager();
		~LogManager();

		// 获取单一实例
		static LogManager* instance();

		// 替换实例
		static void replaceInstance(LogManager* inst);

		// 添加日志
		bool addLog( Log* pLog);

		// 根据名称获取日志
		Log* getLog(const String& name) const;

		// 根据名称移除日志
		void removeLog( const String& name);

		// 根据日志指针移除日志
		void removeLog( Log* pLog);

		// 移除所有日志
		void removeAllLog() { m_logArray.clear(); }

		// 设置日志级别
		void setLogLeve( Log::LogLevel level) { m_logLevel = level; }

		// 获取日志级别
		Log::LogLevel getLogLevel() { return m_logLevel; }

		// 输出日志
		void logMessage(Log::LogLevel level, const char* formats, ...);

		// 输出事件
		void logEvent(Log::LogLevel level, const std::wstring& message);

	public:
		// lua
		void error(const char* msg);
		void warning(const char* msg);
		void info(const char* msg);

		static LogManager* s_instance;

	protected:
		Log::LogLevel	m_logLevel;		// 日志级别
		LogArray		m_logArray;		// A list of all the logs the manager can access
	};
}

#define EchoLogDebug(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::Log::LL_DEBUG, formats, ##__VA_ARGS__);
#define EchoLogInfo(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::Log::LL_INFO, formats, ##__VA_ARGS__);
#define EchoLogWarning(formats, ...)	Echo::LogManager::instance()->logMessage(Echo::Log::LL_WARNING, formats, ##__VA_ARGS__);
#define EchoLogError(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::Log::LL_ERROR, formats, ##__VA_ARGS__);
#define EchoLogFatal(formats, ...)		Echo::LogManager::instance()->logMessage(Echo::Log::LL_FATAL, formats, ##__VA_ARGS__);

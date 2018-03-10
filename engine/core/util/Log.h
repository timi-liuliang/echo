#pragma once

#include "engine/core/Memory/MemManager.h"
#include <fstream>

namespace Echo
{
	/**
	 * Log 2014-6-13
	 */
	class Log
	{
	public:
		enum LogLevel
		{
            LL_INVALID = -1,
			LL_DEBUG, 
			LL_INFO, 
			LL_WARNING, 
			LL_ERROR, 
			LL_FATAL, 
		};

		/** 设置日志名称 */
		virtual void setLogName(const String &logName)=0;

		/** 获取目标名称 */
		virtual const String& getName() const=0;

		/** 设置日志过滤级别 */
		virtual void setLogLevel(LogLevel level)=0;

		/** 日志输出 */
		virtual void logMessage(LogLevel level, const String &msg)=0;
        
		/** 获取日志级别 */
        virtual LogLevel getLogLevel() const=0;
	};

	/**
	 * default log implementation
	 */
	class LogDefault : public Log
	{
	public:
		struct LogConfig
		{
			String			logName;
			String			logFilename;
			bool			bConsoleOutput;
			bool			bVSOutput;
			bool			bFileOutput;
			LogLevel		logLevel;
			bool			bTimeStamp;
			bool			bLogAppend;
			String			path;

			LogConfig()
			{
				bConsoleOutput = true;
				bVSOutput = false;
				bFileOutput = false;
				logLevel   = LL_INVALID;
				bTimeStamp = true;
				bLogAppend = false;
			}
		};

	public:
		LogDefault();
		LogDefault(const String& name);
		LogDefault(const LogConfig& config);
		virtual ~LogDefault();

	public:

		const String&		getName() const;
		const String&		getFilename() const;
		void				enableConsoleOutput(bool bEnable);
		void				enableVSConsoleOutput(bool bEnable);
		void				enableTimeStamp(bool bEnable);
		void				setLogName(const String &logName);
		void				setLogLevel(LogLevel level);
		LogLevel			getLogLevel() const;
		String				getLogLevelDesc(LogLevel level) const;
		bool				isIgnore(LogLevel level) const;

		void				logMessage(LogLevel level, const String &msg);
		void				forceLogMessage(LogLevel level, const String &msg);
		bool 				writelogtosdcard(const char* formats);
        
	protected:
		String				m_logName;
		String				m_logFilename;
		OFStream			m_logStream;
		LogLevel			m_loglevel;
		bool				m_bConsoleOutput;
		bool				m_bVSOutput;
		bool				m_bFileOutput;
		bool				m_bTimeStamp;
		String				m_path;
	};
}

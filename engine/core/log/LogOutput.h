#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <fstream>

namespace Echo
{
	class LogOutput
	{
	public:
		enum Level
		{
            LL_INVALID = -1,
			LL_DEBUG, 
			LL_INFO, 
			LL_WARNING, 
			LL_ERROR, 
			LL_FATAL, 
		};

		// log message
		virtual void logMessage(Level level, const String &msg) = 0;

	public:
		LogOutput(const String& name) : m_name(name) {}

		// get log name
		virtual const String& getName() const { return m_name; }

	private:
		String				m_name;
	};

	/**
	 * default log implementation
	 */
	class LogDefault : public LogOutput
	{
	public:
		struct LogConfig
		{
			String			logName;
			String			logFilename;
			bool			bConsoleOutput;
			bool			bVSOutput;
			bool			bFileOutput;
			Level			logLevel;
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
		LogDefault(const String& name);
		LogDefault(const LogConfig& config);
		virtual ~LogDefault();

	public:
		const String&		getFilename() const;
		void				enableConsoleOutput(bool bEnable);
		void				enableVSConsoleOutput(bool bEnable);
		void				enableTimeStamp(bool bEnable);
		String				getLogLevelDesc(Level level) const;
		bool				isIgnore(Level level) const;

		void				logMessage(Level level, const String &msg);
		void				forceLogMessage(Level level, const String &msg);
		bool 				writelogtosdcard(const char* formats);
        
	protected:
		String				m_logFilename;
		OFStream			m_logStream;
		bool				m_bConsoleOutput;
		bool				m_bVSOutput;
		bool				m_bFileOutput;
		bool				m_bTimeStamp;
		String				m_path;
	};
}

#include "engine/core/util/StringUtil.h"
#include "LogOutput.h"
#include <time.h>

#ifdef ECHO_PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	define _CRT_SECURE_NO_WARNINGS
#	include <Windows.h>
#elif defined(ECHO_PLATFORM_ANDROID)
#	include <android/log.h>
#   include "platform/android/JniHelper.h"
#endif

//////////////////////////////////////////////////////////////////////////

namespace Echo
{
	LogDefault::LogDefault(const String& name)
		: LogOutput(name)
		, m_bConsoleOutput(true)
		, m_bVSOutput(true)
		, m_bFileOutput(false)
		, m_bTimeStamp(true)
	{
		m_logFilename = name + ".log";

		if(m_bFileOutput)
		{
			m_path = "./";
			//std::locale loc = std::locale::global(std::locale(""));
			m_logStream.open(m_logFilename.c_str());
			//std::locale::global(loc);

			String msg;
			msg += "----------------------------------------------------------------------------------------------------\n";

			struct tm *pTime;
			time_t ctTime;
			time(&ctTime);
			pTime = localtime(&ctTime);
			msg += StringUtil::Format(	"LOG DATE: %02d-%02d-%04d %02d:%02d:%02d \n", 
				pTime->tm_mon + 1, 
				pTime->tm_mday, 
				pTime->tm_year + 1900, 
				pTime->tm_hour, 
				pTime->tm_min, 
				pTime->tm_sec);

			msg += "----------------------------------------------------------------------------------------------------\n";
			m_logStream << msg;
			m_logStream.flush();
		}
		
	}

	LogDefault::LogDefault(const LogConfig& config)
		: LogOutput(config.logName)
		, m_logFilename(config.logFilename)
		, m_bConsoleOutput(config.bConsoleOutput)
		, m_bVSOutput(config.bVSOutput)
		, m_bFileOutput(config.bFileOutput)
		, m_bTimeStamp(config.bTimeStamp)
		, m_path(config.path)
	{
		StringUtil::LowerCase(m_logFilename);

		if(m_bFileOutput)
		{
			if(config.bLogAppend)
			{
				//std::locale loc = std::locale::global(std::locale(""));
				if (m_path != "")
					m_logFilename = m_path + "/" + m_logFilename;
				m_logStream.open(m_logFilename.c_str(), std::ios::app);
				//std::locale::global(loc);

				m_logStream<<std::endl;
				m_logStream.flush();
			}
			else
			{
				//std::locale loc = std::locale::global(std::locale(""));
				if (m_path != "")
					m_logFilename = m_path + "/" + m_logFilename;
				m_logStream.open(m_logFilename.c_str());
				//std::locale::global(loc);
#ifdef ECHO_PLATFORM_ANDROID
				FILE* fp = NULL;
        		String strDocPath;
        		if (!strDocPath.empty()) 
				{
            		String strFile = strDocPath + "../" + m_logFilename;
            		fp = fopen(strFile.c_str(),"w+");
					fclose(fp);
				}
#elif defined(ECHO_PLATFORM_MAC_IOS)
				String strDocPath = m_path;
				if ( !strDocPath.empty( ) ) 
				{
					String strFile = strDocPath + "/" + m_logFilename;
					FILE *fp = fopen( strFile.c_str( ), "w+" );
					if ( fp ) 
					{
						fclose( fp );
					}
				}
#endif
			}

			String msg;
			msg += "----------------------------------------------------------------------------------------------------\n";

			struct tm *pTime;
			time_t ctTime;
			time(&ctTime);
			pTime = localtime(&ctTime);
			msg += StringUtil::Format(	"LOG DATE: %02d-%02d-%04d %02d:%02d:%02d \n", 
				pTime->tm_mon + 1, 
				pTime->tm_mday, 
				pTime->tm_year + 1900, 
				pTime->tm_hour, 
				pTime->tm_min, 
				pTime->tm_sec);

			msg += "----------------------------------------------------------------------------------------------------\n";
			m_logStream << msg;
			m_logStream.flush();
		}
	}

	LogDefault::~LogDefault()
	{
		if(m_bFileOutput)
		{
			m_logStream.close();
		}
	}

	const String& LogDefault::getFilename() const
	{
		return m_logFilename;
	}

	void LogDefault::enableConsoleOutput(bool bEnable)
	{
		m_bConsoleOutput = bEnable;
	}

	void LogDefault::enableVSConsoleOutput(bool bEnable)
	{
		m_bVSOutput = bEnable;
	}

	void LogDefault::enableTimeStamp(bool bEnable)
	{
		m_bTimeStamp = bEnable;
	}

	String LogDefault::getLogLevelDesc(Level level) const
	{
		switch(level)
		{
		case LL_DEBUG:		return "DEBUG";
		case LL_INFO:		return "INFO";
		case LL_WARNING:	return "WARNING";
		case LL_ERROR:		return "ERROR";
		case LL_FATAL:		return "FATAL";
		default:			return "INFO";
		}
	}

	bool LogDefault::isIgnore(Level level) const
	{
		return false;
	}

	void LogDefault::logMessage(Level level, const String& msg)
	{
		if(isIgnore(level))
			return ;

		forceLogMessage(level, msg);
	} 

	bool LogDefault::writelogtosdcard(const char* formats)
	{
#ifdef ECHO_PLATFORM_ANDROID
		FILE* fp = NULL;
		String strDocPath;
        if (!strDocPath.empty()) {
            String strFile = strDocPath + "../" + m_logFilename;
            fp = fopen(strFile.c_str(),"a+");
		if(!fp)
		{
				__android_log_print(ANDROID_LOG_ERROR, m_logName.c_str(), strFile.c_str());
			Echo::LogDefault("Can't find SdCard");
			return false;
		}
		//Echo::Log("find SdCard");
		Echo::String str_ = formats;
		str_.append("\n");
		fwrite(str_.c_str(),str_.length(),1,fp);
		fclose(fp);
        }

		ICEFIRE::JniMethodInfo t;
		if (ICEFIRE::JniHelper::getStaticMethodInfo(t,
			LOG_FOR_LEDOCRASHSDK_CLASS_NAME,
			LOG_FOR_LEDOCRASHSDK_FUNCTION_NAME,
			LOG_FOR_LEDOCRASHSDK_SIGNATURE))
		{
			Echo::String strlog = formats;
			strlog.append("\n");
			char arrChLog[4096];
			strncpy(arrChLog, strlog.c_str(), sizeof(arrChLog));
			jstring stringArg1 = t.env->NewStringUTF(arrChLog);

			t.env->CallStaticVoidMethod(t.classID, t.methodID, stringArg1);
			t.env->DeleteLocalRef(t.classID);
			t.env->DeleteLocalRef(stringArg1);
		}

#endif

		return true;
	}
    
	void LogDefault::forceLogMessage(Level level, const String& msg)
	{
		String msgStr;
		// Write log level
		String logLevelDesc = getLogLevelDesc(level);

		if (m_bTimeStamp)
		{
			struct tm* pTime;
			time_t ctTime;
			time(&ctTime);
			pTime = localtime(&ctTime);
			msgStr += StringUtil::Format("%02d:%02d:%02d ", pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
		}
		msgStr += "[" + logLevelDesc + "]: " + msg;

		msgStr = "(" + getName() + ") " + msgStr + "\n";

		// Write time to console
		if(m_bConsoleOutput)
		{
#ifdef ECHO_PLATFORM_ANDROID
			android_LogPriority priority;
			switch(level)
			{
			case LL_DEBUG:		priority = ANDROID_LOG_DEBUG;	break;
			case LL_INFO:		priority = ANDROID_LOG_INFO;	break;
			case LL_WARNING:	priority = ANDROID_LOG_WARN;	break;
			case LL_ERROR:		priority = ANDROID_LOG_ERROR;	break;
			case LL_FATAL:		priority = ANDROID_LOG_FATAL;	break;
			default :			priority = ANDROID_LOG_INFO;	break;
			}

			__android_log_print(priority, m_logName.c_str(), msgStr.c_str());
			writelogtosdcard(msgStr.c_str());
#else
			printf("%s", msgStr.c_str());
#endif
		}

#if defined(_WIN32) || defined(WIN32)
		// Write time to vs console
		if(m_bVSOutput)
		{
			OutputDebugStringA(msgStr.c_str());
		}


		// 错误消息强制弹窗(Release && Windows only)
#ifndef ECHO_EDITOR_MODE
		if (level >= LL_ERROR)
		{
			//::MessageBox(NULL, msg.c_str(), "Fatal Error", MB_ICONERROR);
		}
#endif

#endif

		if (m_bFileOutput)
		{
			m_logStream << msgStr.c_str() ;
			m_logStream.flush();
		}
	}

}


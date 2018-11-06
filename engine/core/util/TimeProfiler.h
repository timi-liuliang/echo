#pragma once

#include <functional>
#include <string>
#include "engine/core/base/echo_def.h"

namespace Echo
{
	class TimeProfiler
	{
	public:
		// 跟踪数据
		struct Trace
		{
			int			m_id;			// 标识
			int			m_tab;			// 缩进
			String		m_tag;			// 标识(显示)
			ulong		m_startTime;	// 开始时间
			ulong		m_totalTime;
		};

	public:
		TimeProfiler();
		~TimeProfiler();

		// 获取全局实例
		static TimeProfiler& instance();

		// 重置
		void reset();

		// 开始记录
		void beginSection(std::size_t section, const String& tag, const char* func);

		// 结束记录
		void endSection(std::size_t section);

		// 输出
		void output();

		// 通过id获取trace
		bool getTraceByID( Trace& trace, int id);

		static size_t lordStrHash(const char *str);

	private:
		int							m_tab;					// 当前缩进(进栈出栈)
		map<std::size_t, Trace>::type	m_traces;
		Dword						m_totalStartTime;		// 总分析开始时间
	};
}
//vs2010 only takes a subset of the string's characters to determine the hash value.so we have to implement it ourselves

#define ECHO_STR_EXPAND(token) #token
#define ECHO_STR(token) ECHO_STR_EXPAND(token)

#define ECHO_TIME_PROFILE_LINENAME_CAT(name, line) ECHO_STR(name+line)
#define ECHO_TIME_PROFILE_LINENAME(name, line) ECHO_TIME_PROFILE_LINENAME_CAT(name, line)
#define ECHO_STR_IDENTIFY ECHO_TIME_PROFILE_LINENAME(__FILE__, __LINE__)
// FIXME: hash collision !!!
#define ECHO_IDENTIFY (Echo::TimeProfiler::lordStrHash(ECHO_STR_IDENTIFY) + __LINE__)

#define TIME_PROFILE_ON		0
#if TIME_PROFILE_ON 
	#define TIME_PROFILE_RESET				Echo::TimeProfiler::instance().reset();
#define TIME_PROFILE(codeModule)		Echo::TimeProfiler::instance().beginSection(ECHO_IDENTIFY, ECHO_STR_IDENTIFY, __FUNCTION__); codeModule Echo::TimeProfiler::instance().endSection(ECHO_IDENTIFY);
	#define TIME_PROFILE_OUTPUT				Echo::TimeProfiler::instance().output();
#else
	#define TIME_PROFILE_RESET
	#define TIME_PROFILE(codeModule) codeModule
	#define TIME_PROFILE_OUTPUT
#endif


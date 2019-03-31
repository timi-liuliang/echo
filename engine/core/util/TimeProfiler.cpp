#include "StringUtil.h"
#include "AssertX.h"
#include "Timer.h"
#include "TimeProfiler.h"
#include "engine/core/log/Log.h"

#ifdef ECHO_PLATFORM_ANDROID
#include <android/log.h>
#endif

namespace Echo
{
	static TimeProfiler	g_timeProfiler;

	TimeProfiler::TimeProfiler()
		: m_tab( 0)
	{
	}

	TimeProfiler::~TimeProfiler()
	{

	}

	TimeProfiler& TimeProfiler::instance()
	{
		return g_timeProfiler;
	}

	void TimeProfiler::reset()
	{
		m_traces.clear();
		m_totalStartTime = Time::instance()->getMilliseconds();
	}

	void TimeProfiler::beginSection(std::size_t section, const String& tag, const char* func)
	{
		auto it = m_traces.find( section);
		if( it == m_traces.end())
		{
			auto identify = tag;
			auto slash = identify.rfind('/');
			if (slash == String::npos)
			{
				slash = identify.rfind('\\');
			}
			if (slash != String::npos)
			{
				identify = identify.substr(slash + 1);
			}
			identify = func + String("+") + identify;
			Trace trace;
			trace.m_id		  = static_cast<int>( m_traces.size());
			trace.m_tab		  = m_tab;
			trace.m_tag		  = identify;
			trace.m_startTime = Time::instance()->getMilliseconds();
			trace.m_totalTime = 0;
			m_traces[section] = trace;
		}
		else
		{
			it->second.m_startTime = Time::instance()->getMilliseconds();
		}

		m_tab++;
	}

	void TimeProfiler::endSection(std::size_t section)
	{
		auto it = m_traces.find( section);
		EchoAssert( it!=m_traces.end());

		ulong endSectionTime = Time::instance()->getMilliseconds();
		ulong timeInterval   = endSectionTime - it->second.m_startTime;
		it->second.m_totalTime += timeInterval;

		m_tab--;
	}

	bool TimeProfiler::getTraceByID( Trace& trace, int id)
	{
		for (auto it=m_traces.begin(); it!=m_traces.end(); it++)
		{
			if( it->second.m_id == id)
			{
				trace = it->second;
				return true;
			}
		}

		EchoAssert( false);
		return false;
	}

	void TimeProfiler::output()
	{
		Dword totalEndTime = Time::instance()->getMilliseconds();
		ulong totalTime    = totalEndTime - m_totalStartTime;

		String result = StringUtil::Format( "TimeProfiler Total:%d\n", totalTime);

		Trace trace;
		for( int i=0; i<static_cast<int>(m_traces.size()); i++)
		{
			if( getTraceByID( trace, i))
			{
				String tag;
				for( int i=0; i<trace.m_tab; i++)
					tag += "    ";
				tag += trace.m_tag;

				result += StringUtil::Format( "TimeProfiler %-60s%9d, %1.1f%%\n", tag.c_str(), trace.m_totalTime, (float)(trace.m_totalTime) / (float)(totalTime)*100.0f);
			}
			else
			{
				EchoAssert( false);
			}
		}

		EchoLogInfo("%s", result.c_str());
	}

	size_t TimeProfiler::lordStrHash(const char *str)
	{
		size_t hash = 5381;
		if (NULL != str)
		{
			int c = 0;
			while ((c = *str++))
			{
				hash = ((hash << 5) + hash) + c;
			}
		}
		return hash;
	}
}

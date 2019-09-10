#include <windows.h>
#include "Log.h"
#include <engine/core/util/StringUtil.h>

namespace Echo
{
	AppLog::AppLog(const Echo::String& name)
		: LogOutput(name)
	{

	}

	void AppLog::logMessage(Level level, const Echo::String &msg)
	{
		// out put to vs
#ifdef ECHO_PLATFORM_WINDOWS
		OutputDebugStringA((msg + "\n").c_str());
#endif
	}
}
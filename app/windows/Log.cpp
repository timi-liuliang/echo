#include "Log.h"
#include <engine/core/util/StringUtil.h>

namespace App
{
	Log::Log(const Echo::String& name)
		: LogOutput(name)
	{

	}

	void Log::logMessage(Level level, const Echo::String &msg)
	{
		// out put to vs
#ifdef ECHO_PLATFORM_WINDOWS
		OutputDebugStringA((msg + "\n").c_str());
#endif
	}
}
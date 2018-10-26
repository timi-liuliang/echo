#pragma once

#include <engine/core/log/Log.h>

namespace Echo
{
	class AppLog : public Echo::LogOutput
	{
	public:
		AppLog(const Echo::String& name);

		/** out message */
		virtual void logMessage(Level level, const Echo::String &msg);
	};
}
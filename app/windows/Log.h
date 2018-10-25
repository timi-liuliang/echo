#pragma once

#include <engine/core/log/Log.h>

namespace App
{
	class Log : public Echo::LogOutput
	{
	public:
		Log(const Echo::String& name);

		/** out message */
		virtual void logMessage(Level level, const Echo::String &msg);
	};
}
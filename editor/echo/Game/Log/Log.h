#pragma once

#include <engine/core/log/Log.h>

namespace Game
{
	class GameLog : public Echo::LogOutput
	{
	public:
		GameLog(const Echo::String& name);

		/** »’÷æ ‰≥ˆ */
		virtual void logMessage(Level level, const Echo::String &msg);
	};
}
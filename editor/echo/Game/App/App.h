#pragma once

#include <engine/core/main/Engine.h>
#include "Log.h"

namespace Game
{
	class App
	{
	public:
		App();
		~App();

		// init
		void init(size_t hwnd, const Echo::String& echoProject);

		// tick
		void tick(float elapsedTime);

		// onSize
		void onSize(Echo::ui32 width, Echo::ui32 height);

	private:
		GameLog*			m_log;
	};
}

#pragma once

#include <Engine/core/main/Root.h>

namespace Game
{
	class App
	{
	public:
		App();
		~App();

		// init
		void init(HWND hwnd, const Echo::String& echoProject);

		// tick
		void tick(Echo::ui32 elapsedTime);
	};
}
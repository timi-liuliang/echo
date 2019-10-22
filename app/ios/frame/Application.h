#pragma once

#include <engine/core/util/StringUtil.h>
#include "Log/Log.h"

namespace Echo
{
	class Application
	{
	public:
		Application();
		~Application();

        // instance
        static Application* instance();

        // init
        void init(int width, int height, const String& rootPath);

        // tick ms
        void tick(float elapsedTime);

    private:
        // check screen size
        void checkScreenSize();

    private:
        GameLog*    m_log = nullptr;
	};
}

#pragma once

#include <engine/core/util/StringUtil.h>

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
	};
}

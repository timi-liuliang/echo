#pragma once

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
        void init(int width, int height);
	};
}

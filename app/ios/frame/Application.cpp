#include "Application.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>

namespace Echo
{
	Application::Application()
	{
	}

	Application::~Application()
	{

	}
    
    Application* Application::instance()
    {
        static Application* inst = EchoNew(Application);
        return inst;
    }
    
    void Application::init(int width, int height, const String& rootPath)
    {
        Engine::instance();
        
        //m_log = EchoNew(GameLog("Game"));
        //Echo::LogManager::instance()->addLog(m_log);
        
        Engine::Config rootcfg;
        rootcfg.m_projectFile = rootPath + "/app.echo";
        rootcfg.m_isEnableProfiler = false;
        Engine::instance()->initialize(rootcfg);
    }
    
    // tick ms
    void Application::tick(float elapsedTime)
    {
        Engine::instance()->tick(elapsedTime);
    }
}

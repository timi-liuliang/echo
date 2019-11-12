#include "Application.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>
#include <engine/core/render/interface/Renderer.h>

extern float iOSGetScreenWidth();
extern float iOSGetScreenHeight();

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

    void Application::init(int width, int height, const String& rootPath, const String& userPath)
    {
        m_log = EchoNew(GameLog("Game"));
        Echo::Log::instance()->addOutput(m_log);

        Echo::initRender(0);

        Engine::Config rootcfg;
        rootcfg.m_projectFile = rootPath + "/data/app.echo";
        rootcfg.m_userPath = userPath;
        Engine::instance()->initialize(rootcfg);
    }

    // tick ms
    void Application::tick(float elapsedTime)
    {
        checkScreenSize();

        Engine::instance()->tick(elapsedTime);
    }

    // check screen size
    void Application::checkScreenSize()
    {
        static float width = 0;
        static float height = 0;
        if(width!=iOSGetScreenWidth() || height!=iOSGetScreenHeight())
        {
            width = iOSGetScreenWidth();
            height = iOSGetScreenHeight();
            Engine::instance()->onSize( width, height);
        }
    }
}

#include "App.h"
#include "Log.h"
#include <engine/core/main/Engine.h>
#include <engine/core/render/gles/GLES.h>
#include <engine/core/util/Timer.h>

namespace Echo
{
    App* App::instance()
    {
        static App app;
        return &app;
    }

    void App::initRes(const String& resDir, const String& userDir)
    {
        m_resDir = resDir;
        m_userDir = userDir;
    }

    void App::initEngine(i32 width, i32 height)
    {
        // Log
        Echo::Log::instance()->addOutput(EchoNew(GameLog("Game")));

        // Initialize
        Echo::initRender(0, width, height);

        Engine::Config rootcfg;
        rootcfg.m_projectFile = m_resDir + "app.echo";
        rootcfg.m_userPath = m_userDir;
        rootcfg.m_isGame = true;
        Engine::instance()->initialize(rootcfg);
    }

    void App::tick()
    {
        static Echo::ulong lastTime = Echo::Time::instance()->getMilliseconds();

        // calc delta Time
        ulong curTime = Echo::Time::instance()->getMilliseconds();
        ulong elapsedTime = curTime - lastTime;
        lastTime = curTime;

        Echo::Engine::instance()->tick(elapsedTime * 0.01f);
    }
}
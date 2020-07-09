#include "App.h"
#include "Log.h"
#include <engine/core/render/gles/GLES.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/HashGenerator.h>

namespace Game
{
	App::App()
		: m_log(nullptr)
	{

	}

	App::~App()
	{

	}

	void App::init(size_t hwnd, const Echo::String& echoProject)
	{
		m_log = EchoNew(GameLog("Game"));
		Echo::Log::instance()->addOutput(m_log);

        Echo::initRender(hwnd);

		Echo::Engine::Config rootcfg;
        rootcfg.m_projectFile = echoProject;
        rootcfg.m_isGame = true;
        rootcfg.m_userPath = Echo::PathUtil::GetCurrentDir() + "/user/" + Echo::StringUtil::Format("u%d/", Echo::BKDRHash(echoProject.c_str()));
		Echo::PathUtil::FormatPath(rootcfg.m_userPath);
		Echo::Engine::instance()->initialize(rootcfg);
	}

	void App::tick(float elapsedTime)
	{
		Echo::Engine::instance()->tick(elapsedTime);
	}

	void App::onSize(Echo::ui32 width, Echo::ui32 height)
	{
		Echo::Engine::instance()->onSize(width, height);
	}
}

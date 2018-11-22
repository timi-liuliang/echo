#include "App.h"
#include "Log.h"
#include <engine/core/render/gles/GLES.h>

namespace Game
{
	App::App()
		: m_root(nullptr)
		, m_log(nullptr)
	{

	}

	App::~App()
	{

	}

	// init
	void App::init(HWND hwnd, const Echo::String& echoProject)
	{
		m_root = Echo::Engine::instance();

		m_log = EchoNew(GameLog("Game"));
		Echo::Log::instance()->addOutput(m_log);

		Echo::Engine::Config rootcfg;
		rootcfg.m_projectFile = echoProject;
		rootcfg.m_windowHandle = (size_t)hwnd;
		m_root->initialize(rootcfg);
	}

	void App::tick(float elapsedTime)
	{
		m_root->tick(elapsedTime);
	}

	// onSize
	void App::onSize(Echo::ui32 width, Echo::ui32 height)
	{
		m_root->onSize(width, height);
	}
}
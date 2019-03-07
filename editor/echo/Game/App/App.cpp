#include "App.h"
#include "Log.h"
#include <engine/core/render/gles/GLES.h>

namespace Game
{
	App::App()
		: m_log(nullptr)
	{

	}

	App::~App()
	{

	}

	// init
	void App::init(size_t hwnd, const Echo::String& echoProject)
	{
		m_log = EchoNew(GameLog("Game"));
		Echo::Log::instance()->addOutput(m_log);

        Echo::initRender(hwnd);
        Echo::initEngine( echoProject, true);
	}

	void App::tick(float elapsedTime)
	{
		Echo::Engine::instance()->tick(elapsedTime);
	}

	// onSize
	void App::onSize(Echo::ui32 width, Echo::ui32 height)
	{
		Echo::Engine::instance()->onSize(width, height);
	}
}

#include "App.h"
#include <engine/core/render/gles/GLES2.h>

namespace Game
{
	App::App()
	{

	}

	App::~App()
	{

	}

	// init
	void App::init(HWND hwnd, const Echo::String& echoProject)
	{
		Echo::Root::RootCfg rootcfg;
		rootcfg.projectFile = echoProject;
		rootcfg.m_isEnableProfiler = true;
		EchoRoot->initialize(rootcfg);

		Echo::Renderer* renderer = nullptr;
		Echo::LoadGLESRenderer(renderer);

		Echo::Renderer::RenderCfg renderCfg;
		renderCfg.enableThreadedRendering = false;
		renderCfg.windowHandle = (unsigned int)hwnd;
		renderCfg.enableThreadedRendering = false;
		EchoRoot->initRenderer(renderer, renderCfg);

		Echo::Renderer::BGCOLOR = Echo::Color(0.298f, 0.298f, 0.322f);
	}

	void App::tick(Echo::ui32 elapsedTime)
	{
		EchoRoot->tick(elapsedTime);
	}
}
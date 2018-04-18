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
		Echo::Root::Config rootcfg;
		rootcfg.projectFile = echoProject;
		rootcfg.m_isEnableProfiler = true;
		rootcfg.m_windowHandle = (unsigned int)hwnd;
		EchoRoot->initialize(rootcfg);
	}

	void App::tick(Echo::ui32 elapsedTime)
	{
		EchoRoot->tick(elapsedTime);
	}

	// onSize
	void App::onSize(Echo::ui32 width, Echo::ui32 height)
	{
		EchoRoot->onSize(width, height);
	}
}
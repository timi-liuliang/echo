#include "Application.h"
#include "Log.h"
#include <engine/core/render/gles/gles.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/hash_generator.h>

namespace Echo
{
	DECLARE_MODULE(Application, __FILE__)

	Application::Application()
		: m_log(nullptr)
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

	void Application::bindMethods()
	{
		CLASS_BIND_METHOD(Application, quit);
	}

	void Application::init(QWidget* mainWindow, size_t hwnd, const Echo::String& echoProject)
	{
		auto calcEngineRootPath = []()
		{
			Echo::String appPath = Echo::PathUtil::GetCurrentDir();
			Echo::PathUtil::FormatPath(appPath, false);

			// calculate root path
#ifdef ECHO_PLATFORM_WINDOWS
			Echo::String rootPath = appPath + "../../../../";
#elif defined ECHO_PLATFORM_MAC
			Echo::String rootPath = appPath + "../echo/";
#else
			Echo::String rootPath = appPath + "../../../../";
#endif
			Echo::PathUtil::FormatPath(rootPath, false);
			Echo::PathUtil::FormatPathAbsolut(rootPath, false);

			return rootPath;
		};

		m_mainWindow = mainWindow;

		m_log = EchoNew(Game::GameLog("Game"));
		Echo::Log::instance()->addOutput(m_log);

        Echo::initRender(hwnd);

		Echo::Engine::Config rootcfg;
        rootcfg.m_projectFile = echoProject;
        rootcfg.m_isGame = true;
        rootcfg.m_userPath = Echo::PathUtil::GetCurrentDir() + "/user/" + Echo::StringUtil::Format("u%d/", Echo::BKDRHash(echoProject.c_str()));
		rootcfg.m_rootPath = calcEngineRootPath();
		Echo::PathUtil::FormatPath(rootcfg.m_userPath);
		Echo::Engine::instance()->initialize(rootcfg);
	}

	void Application::tick(float elapsedTime)
	{
		Echo::Engine::instance()->tick(elapsedTime);
	}

	void Application::onSize(Echo::ui32 width, Echo::ui32 height)
	{
		Echo::Engine::instance()->onSize(width, height);
	}

	void Application::quit()
	{
		if (m_mainWindow)
		{
			m_mainWindow->close();
		}
	}
}

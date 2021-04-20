#pragma once

#include <engine/core/main/Engine.h>
#include <engine/core/main/module.h>
#include <QWidget>
#include "Log.h"

namespace Echo
{
	class Application : public Module
	{
		ECHO_SINGLETON_CLASS(Application, Module)

	public:
		Application();
		~Application();

		// instance
		static Application* instance();

		// init
		void init(QWidget* mainWindow, size_t hwnd, const Echo::String& echoProject);

		// tick
		void tick(float elapsedTime);

		// onSize
		void onSize(Echo::ui32 width, Echo::ui32 height);

		// quit
		void quit();

	private:
		Game::GameLog*			m_log;
		QWidget*				m_mainWindow = nullptr;
	};
}

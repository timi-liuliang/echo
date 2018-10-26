#pragma once

#include "Log.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	class App
	{
	public:
		App();
		~App();

		// start
		void start();

		// tick
		void tick(float elapsedTime);

		// message handle
		LRESULT CALLBACK wndProc(HWND hWnd, Echo::ui32 msg, WPARAM wParam, LPARAM lParam);

		// destroy
		void destroy();

	protected:
		// init window
		bool initWindow(int screenWidth, int screenHeight, bool bFullscreen = false);

		// init engine
		void initEngine(HWND hwnd, const Echo::String& echoProject);

		// destroy window
		void destroyWindow();

		// switch to full screen
		BOOL setFullScreen();

	private:
		Echo::String	m_appName;
		HINSTANCE		m_hInstance;
		HWND			m_hWnd;
		Echo::i32		m_Style;
		bool			m_bFullscreen;
		Echo::i32		m_screenWidth;
		Echo::i32		m_screenHeight;
		Echo::String	m_projectFile;
		Echo::Engine*	m_engine;
		App::AppLog*	m_log;
	};
}
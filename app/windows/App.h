#pragma once

#include "Log.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	class App
	{
	public:
		App( const Echo::String& project);
		virtual ~App();

		virtual bool		setup();
		virtual void		run();
		virtual bool		onInit();
		virtual void		onDestroy();
		virtual void		tick(float elapse);

		LRESULT CALLBACK messageHandler(HWND hWnd, Echo::ui32 msg, WPARAM wParam, LPARAM lParam);

	public:
		bool	onInitEngine();

	protected:
		bool	initWindow(int screenWidth, int screenHeight, bool bFullscreen = false);
		void	destroyWindow();
		BOOL	setFullScreen();
		BOOL	resetWindow();

	private:
		Echo::String	m_appName;
		Echo::String	m_rootPath;
		HINSTANCE		m_hInstance;
		HWND			m_hWnd;
		Echo::i32		m_Style;
		RECT			m_WndRect;

		bool			m_bFullscreen;
		Echo::i32		m_screenWidth;
		Echo::i32		m_screenHeight;
		bool			m_bPaused;
		bool			m_bRendererInited;
		Echo::String	m_projectFile;
		Echo::Engine*	m_engine;
		App::AppLog*	m_log;
	};
}
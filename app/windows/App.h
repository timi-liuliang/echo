#pragma once

#include "Log.h"
#include <engine/core/main/Engine.h>

namespace App
{
	class App
	{
	public:
		App( const Echo::String& project);
		virtual ~App();

		virtual bool		setup();
		virtual void		run();
		HINSTANCE			getInstHandle();
		HWND				getWindowHandle();

		virtual bool		onInit();
		virtual void		onDestroy();
		virtual void		onTouchBegine(float x, float y);
		virtual void		onTouchMove(float x, float y);
		virtual void		onTouchEnd(float x, float y);
		virtual void		tick(float elapse);

		void				onKeyboardMsg(Echo::ui32 AnMsg, Echo::i32 AnWParam, Echo::i32 AnLParam);

		LRESULT CALLBACK	messageHandler(HWND hWnd, Echo::ui32 msg, WPARAM wParam, LPARAM lParam);

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
		bool			m_isMove;
		Echo::String	m_projectFile;
		Echo::Engine*	m_root;
		App::Log*		m_log;
	};

	extern App*	g_pApp;
}
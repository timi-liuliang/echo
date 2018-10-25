#pragma once

#include "LuaModule.h"
#include <Foundation/Util/LogManager.h>

namespace App
{
	class App
	{
	public:
		App( const String& project, const String& scene);
		virtual ~App();

		const String		getRootPath() const { return m_rootPath; }
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

		void				onKeyboardMsg(uint AnMsg, i32 AnWParam, i32 AnLParam);

		LRESULT CALLBACK	messageHandler(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);

	public:
		bool	onInitEngine();

	protected:
		bool	initWindow(int screenWidth, int screenHeight, bool bFullscreen = false);
		void	destroyWindow();
		BOOL	setFullScreen();
		BOOL	resetWindow();

		// 切换场景
		void ChangeScene(const char* sceneName);

	private:
		String			m_appName;
		String			m_rootPath;
		HINSTANCE		m_hInstance;
		HWND			m_hWnd;
		i32				m_Style;
		RECT			m_WndRect;

		bool			m_bFullscreen;
		i32				m_screenWidth;
		i32				m_screenHeight;
		bool			m_bPaused;
		bool			m_bRendererInited;
		bool			m_isMove;

		String			m_projectFile;
		String			m_sceneName;

	private:
		LORD::LogDefault*			m_log;				// 日志
		LORD::SimpleLuaModule*		m_pLuaModule;		// lua模块
	};

	extern App*	g_pApp;
}
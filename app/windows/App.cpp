#include "App.h"

#include <Engine/Object/Root.h>
#include <Engine/Scene/Scene.h>
#include <GLES2Render/GLES2.h>
#include <Foundation/Util/Timer.h>
#include <Foundation/pfs/pfs2.h>
#include <Foundation/platform/utils/PlatformStringUtil.h>

#include "Extension/Actor/ActorManager.h"

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_I 0x49
#define VK_U 0x55
#define VK_O 0x4F
#define VK_H 0x48

namespace App
{
	bool gbKeyUp = false;
	bool gbKeyDown = false;
	bool gbKeyLeft = false;
	bool gbKeyRight = false;

	App* g_pApp = NULL;

	void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
	{
		// 创建Dump文件
		HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		// Dump信息
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = pException;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;

		// 写入Dump文件内容
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

		CloseHandle(hDumpFile);
	}

	// 作为except块中表达式的函数
	LONG CrashHandler(EXCEPTION_POINTERS *pException)
	{
		// 这里以弹出一个对话框为例子
		MessageBox(NULL, _T("崩溃日志将要保存为  Dump.dmp, 请发送给编译这个版本代码的程序员。"), _T("Crash"), MB_OK);

		// 创建Dump文件
		String strRoot = g_pApp->getRootPath();
		String strDump = strRoot + "\\Dump.dmp";
		CreateDumpFile(strDump.c_str(), pException);

		return EXCEPTION_EXECUTE_HANDLER;
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return g_pApp->messageHandler(hWnd, msg, wParam, lParam);
	}

	App::App(const String& project, const String& scene)
		: m_log(NULL)
		, m_pLuaModule(NULL)
		, m_projectFile(project)
		, m_sceneName(scene)
	{
		g_pApp = this;

		char szRootPath[MAX_PATH];
		memset(szRootPath, 0, sizeof(szRootPath));
		GetCurrentDirectory(MAX_PATH, szRootPath);
		m_rootPath = szRootPath;
		m_bFullscreen = false;
		m_screenWidth = 1280;
		m_screenHeight = 720;
		m_bPaused = false;
		m_bRendererInited = false;
		m_isMove = false;
	}

	App::~App()
	{
	}

	bool App::setup()
	{
		if ( !initWindow(m_screenWidth, m_screenHeight, m_bFullscreen) )
			return false;

		if ( !onInit() )
		{
			return false;
		}
		m_bRendererInited = true;

		return true;
	}

	void App::run()
	{
#ifndef _DEBUG
		__try
#endif
		{
			if ( !setup() )
				return;

			__int64 cntsPerSec = 0;
			QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
			float secsPerCnt = 1.0f / (float)cntsPerSec;

			__int64 prevTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

			MSG msg;
			ZeroMemory(&msg, sizeof(MSG));
			// Loop until there is a quit message from the window or the user.
			bool done = false;
			while ( WM_QUIT != msg.message )
			{
				// Handle the windows messages.
				if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					if ( m_bPaused )
					{
						Sleep(20);
						continue;
					}

					//Sleep( 15 );
					__int64 currTimeStamp = 0;
					QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
					float deltaTime = (currTimeStamp - prevTimeStamp) * 1000.f / cntsPerSec;

					tick(deltaTime);

					prevTimeStamp = currTimeStamp;
				}
			}
			onDestroy();
		}
#ifndef _DEBUG
		__except ( CrashHandler(GetExceptionInformation()) )
		{

		}
#endif
	}

	HINSTANCE App::getInstHandle()
	{
		return m_hInstance;
	}

	HWND App::getWindowHandle()
	{
		return m_hWnd;
	}

	bool App::onInit()
	{
		if ( !onInitEngine() )
		{
			return false;
		}

		m_pLuaModule = new LORD::SimpleLuaModule;

		return true;
	}

	bool App::onInitEngine()
	{
		// PFS Mount
		std::wstring rootPath = s2ws(LORD::PathUtil::GetCurrentDir());
		PFS::CEnv::Mount(L"/root", rootPath, PFS::FST_NATIVE);

		LordNew(LORD::Root);
		LordRoot->initLogSystem();
		// 添加默认日志处理
		LORD::LogDefault::LogConfig logConfig;
		logConfig.logName = "lord.log";
		logConfig.logLevel = LORD::Log::LL_DEBUG;
		logConfig.bVSOutput = true;
		logConfig.bFileOutput = true;
		logConfig.path = "./";
		logConfig.logFilename = "GameShell.log";

		LordLogManager->setLogLeve(logConfig.logLevel);
		m_log = LordNew(LORD::LogDefault(logConfig));
		if ( m_log )
		{
			LordLogManager->addLog(m_log);
		}

		LORD::ActorManager* pActorManager = LordNew(LORD::ActorManager);

		// 获取当前路径
		LORD::String currentDir = LORD::PathUtil::GetCurrentDir();
		LORD::Root::RootCfg rootcfg;
		rootcfg.m_actorManager = pActorManager;

#ifdef LORD_PLATFORM_WINDOWS
		rootcfg.projectFile = m_projectFile;
#elif defined LORD_PLATFORM_MAC_IOS
		rootcfg.projectFile = currentDir + "/media/shell.project";
#endif

		LordRoot->initialize(rootcfg);

		LordRoot->setEnableDistortion(true);

		LORD::Renderer::RenderCfg renderCfg;

#ifdef LORD_PLATFORM_WINDOWS
		renderCfg.windowHandle = (uint)m_hWnd;
#endif
		renderCfg.screenWidth = m_screenWidth;
		renderCfg.screenHeight = m_screenHeight;

		// 添加默认资源路径
		LordResourceManager->addArchive(LORD::PathUtil::GetCurrentDir() + "/assets", "filesystem", true);

		LORD::Renderer* render = NULL;
		LORD::LoadGLESRenderer(render);
		if ( render )
		{
			LordRoot->initRenderer(render, renderCfg);
		}
		else
		{
			LordLogError("GameShell Init GLESRender Failed!");
			return false;
		}

		// 背景色
		LORD::Renderer::BGCOLOR = LORD::Color::GRAY;

		// 加载场景
		ChangeScene(m_sceneName.c_str());

		return true;
	}

	// 切换场景
	void App::ChangeScene(const char* sceneName)
	{
		LordSceneManager->closeScene();
		LordSceneManager->loadScene(sceneName, Vector3::ZERO);
	}

	void App::onDestroy()
	{
		if (m_pLuaModule)
		{
			delete m_pLuaModule;
			m_pLuaModule = NULL;
		}

		LORD::Root* pRoot = LordRoot;
		pRoot->destroy();
		LordSafeDelete(pRoot, Root);

		LordSafeDelete(m_log, LogDefault);

		destroyWindow();
	}

	LRESULT CALLBACK App::messageHandler(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam)
	{
		// Is the application in a minimized or maximized state?
		static bool minOrMaxed = false;

		switch ( msg )
		{
			// WM_SIZE is sent when the user resizes the window.  
			case WM_SIZE:
			{
				if ( m_bRendererInited )
				{
					uint width = (uint)LOWORD(lParam);
					uint height = (uint)HIWORD(lParam);

					LordRoot->onSize(width, height);
				}
			}
			break;
			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
			case WM_EXITSIZEMOVE:
			{
				RECT clientRect = { 0, 0, 0, 0 };
				GetClientRect(m_hWnd, &clientRect);
			} 
			break;
			// WM_CLOSE is sent when the user presses the 'X' button in the
			// caption bar menu.
			case WM_CLOSE:
			{
				DestroyWindow(m_hWnd);
			} 
			break;
			// WM_DESTROY is sent when the window is being destroyed.
			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			break;
			case WM_LBUTTONDOWN:
			{
				uint x = (uint)LOWORD(lParam);
				uint y = (uint)HIWORD(lParam);
				m_isMove = true;
				onTouchBegine((float)x, (float)y);
			}
			break;
			case WM_LBUTTONUP:
			{
				uint x = (uint)LOWORD(lParam);
				uint y = (uint)HIWORD(lParam);
				m_isMove = false;
				onTouchEnd((float)x, (float)y);
			}
			break;
			case WM_MOUSEMOVE:
			{
				uint x = (uint)LOWORD(lParam);
				uint y = (uint)HIWORD(lParam);
				if ( m_isMove == true )
				{
					onTouchMove((float)x, (float)y);
				}
			}
			break;
			case WM_KEYDOWN:
			{
				if ( wParam == VK_W )
				{
					if ( !gbKeyUp )
					{
						onKeyboardMsg(kKeyEvent_Down, BUTTON_W, lParam);
						gbKeyUp = true;
					}
				}
				else if ( VK_S == wParam )
				{
					if ( !gbKeyDown )
					{
						onKeyboardMsg(kKeyEvent_Down, BUTTON_A, lParam);
						gbKeyDown = true;
					}
				}
				else if ( VK_A == wParam )
				{
					if ( !gbKeyLeft )
					{
						onKeyboardMsg(kKeyEvent_Down, BUTTON_S, lParam);
						gbKeyLeft = true;
					}
				}
				else if ( VK_D == wParam )
				{
					if ( !gbKeyRight )
					{
						onKeyboardMsg(kKeyEvent_Down, BUTTON_D, lParam);
						gbKeyRight = true;
					}
				}
				else if ( VK_NUMPAD0 == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_NUM0, lParam);
				else if ( VK_NUMPAD1 == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_NUM1, lParam);
				else if ( VK_NUMPAD3 == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_NUM3, lParam);
				else if ( VK_NUMPAD4 == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_NUM4, lParam);
				else if ( VK_NUMPAD6 == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_L1, lParam);
				else if ( VK_NUMPAD7 == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_R1, lParam);
				else if ( VK_DIVIDE == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_THUMBR, lParam);
				else if ( VK_MULTIPLY == wParam )
					onKeyboardMsg(kKeyEvent_Down, BUTTON_THUMBL, lParam);
				else if ( 'Q' == wParam )
					onKeyboardMsg(kKeyEvent_Down, KEYCODE_HOME, lParam);
				else if ( 'R' == wParam )
					onKeyboardMsg(kKeyEvent_Down, KEYCODE_MENU, lParam);
				else if ( VK_HOME == wParam )
					onKeyboardMsg(kKeyEvent_Down, KEYCODE_MUTE, lParam);
				else if ( VK_MBUTTON == wParam )
					onKeyboardMsg(kKeyEvent_Down, MAX_CONTROLLERS, lParam);
			}
			break;
			case WM_KEYUP:
			{
				if ( wParam == VK_W )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_W, lParam);
					if ( gbKeyUp )
					{
						gbKeyUp = false;
					}
				}
				else if ( VK_S == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_S, lParam);
					if ( gbKeyDown )
					{
						gbKeyDown = false;
					}
				}
				else if ( VK_A == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_A, lParam);
					if ( gbKeyLeft )
					{
						gbKeyLeft = false;
					}
				}
				else if ( VK_D == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_D, lParam);
					if ( gbKeyRight )
					{
						gbKeyRight = false;
					}
				}
				else if ( VK_H == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_H, lParam);
				}
				else if ( VK_J == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_J, lParam);
				}
				else if ( VK_K == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_K, lParam);
				}
				else if ( VK_L == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_L, lParam);
				}
				else if ( VK_I == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_I, lParam);
				}
				else if ( VK_U == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_U, lParam);
				}
				else if ( VK_O == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_O, lParam);
				}
				else if ( VK_NUMPAD0 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM0, lParam);
				}
				else if ( VK_NUMPAD1 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM1, lParam);
				}
				else if ( VK_NUMPAD2 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM2, lParam);
				}
				else if ( VK_NUMPAD3 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM3, lParam);
				}
				else if ( VK_NUMPAD4 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM4, lParam);
				}
				else if ( VK_NUMPAD5 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM5, lParam);
				}
				else if ( VK_NUMPAD6 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, BUTTON_NUM6, lParam);
				}
				else if ( VK_DIVIDE == wParam )
					onKeyboardMsg(kKeyEvent_Up, BUTTON_THUMBR, lParam);
				else if ( VK_MULTIPLY == wParam )
					onKeyboardMsg(kKeyEvent_Up, BUTTON_THUMBL, lParam);
				else if ( 'Q' == wParam )
					onKeyboardMsg(kKeyEvent_Up, KEYCODE_HOME, lParam);
				else if ( 'R' == wParam )
					onKeyboardMsg(kKeyEvent_Up, KEYCODE_MENU, lParam);
				else if ( VK_HOME == wParam )
					onKeyboardMsg(kKeyEvent_Up, KEYCODE_MUTE, lParam);
				else if ( VK_MBUTTON == wParam )
					onKeyboardMsg(kKeyEvent_Up, MAX_CONTROLLERS, lParam);
				else if ( VK_F11 == wParam )
				{
					onKeyboardMsg(kKeyEvent_Up, VK_F11, lParam);
				}
				else if ( VK_F9 == wParam )
				{
					if ( m_bFullscreen )
					{
						resetWindow();
					}
					else
					{
						setFullScreen();
					}
				}
			}
			break;
			case WM_CHAR:
			{
			}
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	BOOL App::setFullScreen()
	{
		//获得并储存当前窗口信息，用于以后恢复
		GetWindowRect(m_hWnd, &m_WndRect); //获得当前窗口RECT
		LONG style = GetWindowLong(m_hWnd, GWL_STYLE); //获得当前窗口的类型
		m_Style = style;
		//m_Style = style; //储存当前的窗口类型
		//设置窗口显示类型
		style &= ~WS_CAPTION;
		SetWindowLong(m_hWnd, GWL_STYLE, style);
		//获得屏幕大小
		int ScreenX = GetSystemMetrics(SM_CXSCREEN);
		int ScreenY = GetSystemMetrics(SM_CYSCREEN);
		if ( !MoveWindow(m_hWnd, 0, 0, ScreenX, ScreenY, TRUE) )
			return FALSE;

		m_bFullscreen = true;

		return TRUE;
	}

	BOOL App::resetWindow()
	{
		SetWindowLong(m_hWnd, GWL_STYLE, m_Style);
		if ( !MoveWindow(m_hWnd, m_WndRect.left, m_WndRect.top,
			m_WndRect.right - m_WndRect.left, m_WndRect.bottom - m_WndRect.top, TRUE) )
			return FALSE;

		m_bFullscreen = false;

		return TRUE;
	}

	bool App::initWindow(int screenWidth, int screenHeight, bool bFullscreen)
	{
		WNDCLASSEX wc;
		DEVMODE dmScreenSettings;
		int posX, posY;
		// Get the instance of this application.
		m_hInstance = GetModuleHandle(NULL);
		// Give the application a name.
		m_appName = m_projectFile.c_str();

		// Setup the windows class with default settings.
		wc.style = CS_HREDRAW | CS_VREDRAW /*| CS_OWNDC*/;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = m_appName.c_str();
		wc.cbSize = sizeof(WNDCLASSEX);
		// Register the window class.
		RegisterClassEx(&wc);

		// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
		if ( bFullscreen )
		{
			// Determine the resolution of the clients desktop screen.
			screenWidth = GetSystemMetrics(SM_CXSCREEN);
			screenHeight = GetSystemMetrics(SM_CYSCREEN);

			// If full screen set the screen to maximum size of the users desktop and 32bit.
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = (uint)screenWidth;
			dmScreenSettings.dmPelsHeight = (uint)screenHeight;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			// Change the display settings to full screen.
			ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
			// Set the position of the window to the top left corner.
			posX = posY = 0;
		}
		else
		{
			// Place the window in the middle of the screen.
			posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
			posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
		}

		RECT loRt;
		loRt.left = 0;
		loRt.top = 0;
		loRt.right = m_screenWidth;
		loRt.bottom = m_screenHeight;
		AdjustWindowRect(&loRt, WS_CAPTION | WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_POPUP, FALSE);

		// Create the window with the screen settings and get the handle to it.
		m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, m_appName.c_str(), m_appName.c_str(),
			WS_OVERLAPPEDWINDOW /*WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP*/,
			posX, posY, /*screenWidth*/loRt.right - loRt.left, /*screenHeight*/loRt.bottom - loRt.top,
			NULL, NULL, m_hInstance, NULL);
		// Bring the window up on the screen and set it as main focus.
		ShowWindow(m_hWnd, SW_SHOW);
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);

		if ( m_bFullscreen )
		{
			setFullScreen();
		}

		return true;
	}

	void App::destroyWindow()
	{
		// Fix the display settings if leaving full screen mode.
		if ( m_bFullscreen )
		{
			ChangeDisplaySettings(NULL, 0);
		}

		// Remove the window.
		m_hWnd = NULL;

		// Remove the application instance.
		UnregisterClass(m_appName.c_str(), m_hInstance);
		m_hInstance = NULL;

		// Release the pointer to this class.
		g_pApp = NULL;
	}

	// 游戏逻辑
	void App::tick(float elapse)
	{
		LORD::Scene* scene = LordSceneManager->getCurrentScene();
		if (scene)
		{
			scene->setLoadCentrePoint(LORD::Vector3(100.f, 0.f, 100.f));
		}

		// 从场景内运行
		LordSceneManager->setMainPosition(LORD::Vector3(100.f, 0.f, 100.f));

		// 更新
		LordRoot->frameMove(static_cast<LORD::i32>(elapse));

		// 渲染当前场景
		LordRoot->renderScene();

		// 显示到屏幕
		LordRender->present();
	}

	// 鼠标按下
	void App::onTouchBegine(float x, float y)
	{
		
	}

	// 鼠标按下后移动
	void App::onTouchMove(float x, float y)
	{
		
	}

	// 鼠标抬起
	void App::onTouchEnd(float x, float y)
	{
		
	}

	void App::onKeyboardMsg(ui32 AnMsg, i32 AnWParam, i32 AnLParam)
	{
		
	}
}


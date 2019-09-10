#include "App.h"
#include "Log.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/GameSettings.h>
#include <engine/core/render/interface/Renderer.h>

namespace Echo
{
	App* g_pApp = NULL;

	void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
	{
	}

	LONG CrashHandler(EXCEPTION_POINTERS *pException)
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return g_pApp->wndProc(hWnd, msg, wParam, lParam);
	}

	App::App(const Echo::String& rootPath)
		: m_log(nullptr)
	{
		m_bFullscreen = false;
		m_projectFile = rootPath + "data/app.echo";
		Echo::PathUtil::FormatPath(m_projectFile, false);

		g_pApp = this;
	}

	App::~App()
	{
	}

	void App::start()
	{
#ifndef _DEBUG
		__try
#endif
		{
			if (initWindow( 800, 600, m_bFullscreen))
			{
				initEngine(m_hWnd, m_projectFile);

				__int64 cntsPerSec = 0;
				QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
				float secsPerCnt = 1.0f / (float)cntsPerSec;

				__int64 prevTimeStamp = 0;
				QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

				MSG msg;
				ZeroMemory(&msg, sizeof(MSG));

				// Loop until there is a quit message from the window or the user.
				while (WM_QUIT != msg.message)
				{
					// Handle the windows messages.
					if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else
					{
						__int64 currTimeStamp = 0;
						QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
						float deltaTime = (currTimeStamp - prevTimeStamp) * secsPerCnt;

						tick(deltaTime);

						prevTimeStamp = currTimeStamp;
					}
				}
			}

			destroy();
		}
#ifndef _DEBUG
		__except ( CrashHandler(GetExceptionInformation()) )
		{

		}
#endif
	}

	void App::initEngine(HWND hwnd, const Echo::String& echoProject)
	{
		m_log = EchoNew(Echo::AppLog("Game"));
		Echo::Log::instance()->addOutput(m_log);

        Echo::initRender((size_t)hwnd);
        Echo::initEngine(echoProject, true);

		// default window size
		resizeWindow( m_hWnd, Echo::GameSettings::instance()->getWindowWidth(), Echo::GameSettings::instance()->getWindowHeight());
	}

	void App::resizeWindow(HWND hwnd, int width, int height)
	{
		RECT rcClient, rcWindow;
		POINT ptDiff;

		GetClientRect(hwnd, &rcClient);
		GetWindowRect(hwnd, &rcWindow);

		ptDiff.x = rcWindow.right - rcWindow.left - rcClient.right;
		ptDiff.y = rcWindow.bottom - rcWindow.top - rcClient.bottom;

		int realWidth = width + ptDiff.x;
		int realHeight = height + ptDiff.y;

		Echo::i32 posX = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
		Echo::i32 posY = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;
	
		MoveWindow(hwnd, posX, posY, realWidth, realHeight, TRUE);
	}

	void App::destroy()
	{
        Echo::Engine* engine = Echo::Engine::instance();
        EchoSafeDelete(engine, Engine);

		destroyWindow();
	}

	LRESULT CALLBACK App::wndProc(HWND hWnd, Echo::ui32 msg, WPARAM wParam, LPARAM lParam)
	{
		switch ( msg )
		{
			case WM_SIZE:
			{
				Echo::ui32 width = (Echo::ui32)LOWORD(lParam);
				Echo::ui32 height = (Echo::ui32)HIWORD(lParam);
				Echo::Engine::instance()->onSize(width, height);
			}
			break;
			case WM_EXITSIZEMOVE:
			{
				RECT clientRect = { 0, 0, 0, 0 };
				GetClientRect(m_hWnd, &clientRect);
			} 
			break;
			case WM_CLOSE:
			{
				DestroyWindow(m_hWnd);
			} 
			break;
			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			break;
			case WM_LBUTTONDOWN:
			{
				Echo::ui32 x = (Echo::ui32)LOWORD(lParam);
				Echo::ui32 y = (Echo::ui32)HIWORD(lParam);
			}
			break;
			case WM_LBUTTONUP:
			{
				Echo::ui32 x = (Echo::ui32)LOWORD(lParam);
				Echo::ui32 y = (Echo::ui32)HIWORD(lParam);
			}
			break;
			case WM_MOUSEMOVE:
			{
				Echo::ui32 x = (Echo::ui32)LOWORD(lParam);
				Echo::ui32 y = (Echo::ui32)HIWORD(lParam);
			}
			break;
			case WM_KEYDOWN:
			{
			}
			break;
			case WM_KEYUP:
			{
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
		RECT wndRect;
		GetWindowRect(m_hWnd, &wndRect);
		LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
		m_Style = style;
		//m_Style = style; 

		style &= ~WS_CAPTION;
		SetWindowLong(m_hWnd, GWL_STYLE, style);

		// get screen size
		int ScreenX = GetSystemMetrics(SM_CXSCREEN);
		int ScreenY = GetSystemMetrics(SM_CYSCREEN);
		if ( !MoveWindow(m_hWnd, 0, 0, ScreenX, ScreenY, TRUE) )
			return FALSE;

		m_bFullscreen = true;

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
		m_appName = Echo::PathUtil::GetPureFilename(m_projectFile, false);

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
			dmScreenSettings.dmPelsWidth = (DWORD)screenWidth;
			dmScreenSettings.dmPelsHeight = (DWORD)screenHeight;
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
		loRt.right = screenWidth;
		loRt.bottom = screenHeight;
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
		g_pApp = nullptr;
	}

	void App::tick(float elapsedTime)
	{
		Echo::Engine::instance()->tick(elapsedTime);
	}
}


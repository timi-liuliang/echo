#pragma once

#include <Windows.h>
#include <windowsx.h>

#define WINDOW_WIDTH  1366
#define WINDOW_HEIGHT 768

namespace Examples
{
	Application* g_app = NULL;
	LORD::vector<ExampleFactory*>::type	g_exampleFactorys;
}

static HWND g_hwnd; 

static void GetClientSizeImpl(LORD::i32& width, LORD::i32& height)
{
	RECT clientRect;
	GetClientRect(g_hwnd, &clientRect);
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;
}

static void keyboardProcImpl(LORD::ui32 keyChar, bool isKeyDown)
{
	if (isKeyDown)
	{
		switch (keyChar)
		{
		case VK_NUMPAD0:
		case 48: Examples::g_app->startExample(0); break;

		case VK_NUMPAD1:
		case 49: Examples::g_app->startExample(1); break;

		case VK_NUMPAD2:
		case 50: Examples::g_app->startExample(2); break;

		case VK_NUMPAD3:
		case 51: Examples::g_app->startExample(3); break;
		case VK_ESCAPE:  Examples::g_app->startExample(-1); break;
		}
	}
}

// 回调函数;
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool mousrRBDown = false;
	switch (uMsg)
	{
	case WM_CREATE:
	{
		// 创建应用程序
		g_hwnd = hWnd;
		Examples::g_app = new Examples::Application;
		Examples::g_app->start();
	}
	break;

	case WM_NCLBUTTONDOWN:
	{
		Examples::g_app->flushRender();
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;

	// 按键消息
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		Examples::g_app->keyboardProc((LORD::ui32)wParam, true);
	}
	break;

	case WM_MOUSEWHEEL:
	{
		Examples::g_app->mouseWhellProc((int)wParam);
	}
	break;
	case  WM_LBUTTONDOWN:
	{
		Examples::g_app->mouseLBProc(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	break;
	case WM_LBUTTONUP:
	{
		Examples::g_app->mouseLBProc(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), true);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		mousrRBDown = true;
		Examples::g_app->mouseProc(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	break;
	case WM_RBUTTONUP:
	{
		mousrRBDown = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (mousrRBDown)
		{
			Examples::g_app->mouseMoveProc(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
	}
	break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		Examples::g_app->keyboardProc((LORD::ui32)wParam, false);
	}
	break;

	case WM_DESTROY:
	{
		Examples::g_app->end();
		PostQuitMessage(0);
	}
	break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static void PFSMountImpl(LORD::String path, LORD::String internalPath)
{
// 	// PFS Mount
// 	std::wstring rootPath = s2ws(LORD::PathUtil::GetCurrentDir());
// 	PFS::CEnv::Mount(L"/root", rootPath, PFS::FST_NATIVE);
// 
// 	(void)path; 
// 	(void)internalPath; 

}

/**
* 入口函数
*/
int main(int argc, char* argv[])
{
	// 设置当前工作目录
	LORD::String currentPath = LORD::PathUtil::GetFileDirPath(argv[0]);
	SetCurrentDirectory(currentPath.c_str());

	HINSTANCE hInstance = GetModuleHandle(NULL);
	HWND hWnd = 0;
	char appName[] = "Main";

	// 注册窗口类
	WNDCLASS cs = { 0 };
	cs.cbClsExtra = 0;
	cs.cbWndExtra = 0;
	cs.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	cs.hCursor = NULL;
	cs.hIcon = NULL;
	cs.hInstance = hInstance;
	cs.lpfnWndProc = (WNDPROC)WndProc;
	cs.lpszClassName = appName;
	cs.lpszMenuName = NULL;
	cs.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClass(&cs);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowLeft = (screenWidth - 1366) / 2;
	int windowTop = (screenHeight - 768) / 2;

	// 创建窗口
	hWnd = CreateWindow(appName, "Examples", WS_OVERLAPPEDWINDOW, windowLeft, windowTop, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	// 显示窗口
	ShowWindow(hWnd, SW_SHOW);

	// 更新窗口
	UpdateWindow(hWnd);

	// 消息循环
	MSG  nMsg = { 0 };
	while (WM_QUIT != nMsg.message)
	{
		if (PeekMessage(&nMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&nMsg);
			DispatchMessage(&nMsg);
		}
		else
		{
			static LORD::ui32 lastTime = LORD::Time::Instance()->getMilliseconds();

			// 计算delta Time
			LORD::ui32 curTime = LORD::Time::Instance()->getMilliseconds();
			LORD::ui32 elapsedTime = curTime - lastTime;

			Examples::g_app->tick(elapsedTime);

			lastTime = curTime;
		}
	}

	return 0;
}
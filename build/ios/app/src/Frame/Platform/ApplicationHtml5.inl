#pragma once

#include <GL/gl.h>
#include <GL/glut.h>
#include <emscripten.h>
#include <emscripten/html5.h>


static LORD::ui32 WINDOW_WIDTH = 1366;
static LORD::ui32 WINDOW_HEIGHT = 768;
 
namespace Examples
{
	Application* g_app = NULL;
 	LORD::vector<ExampleFactory*>::type	g_exampleFactorys;
}

// 获取窗口大小
static void GetClientSizeImpl(LORD::i32& width, LORD::i32& height)
{
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
}
// 
static void keyboardProcImpl(LORD::ui32 keyChar, bool isKeyDown)
{

}

// 全屏改变回调
EM_BOOL fullscreenchange_callback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData)
{
	if (WINDOW_WIDTH != e->screenWidth || WINDOW_HEIGHT != e->screenHeight)
	{
		glutReshapeWindow(e->screenWidth, e->screenHeight);
	}

	return 0;
}

// 鼠标回调消息
EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent* e, void* userData)
{
	LORD::ui32 xPos = e->canvasX;
	LORD::ui32 yPos = e->canvasY;

 	if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
 	{
 		Examples::g_app->mouseLBProc(xPos, yPos, true);
 	}
 	else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP)
 	{
 		Examples::g_app->mouseLBProc(xPos, yPos, true);
 	}

	return 0;
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
 * Draws the gears
 */
static void gears_draw()
{
	static LORD::ui32 lastTime = LORD::Time::Instance()->getMilliseconds();

	// 计算delta Time
	LORD::ui32 curTime = LORD::Time::Instance()->getMilliseconds();
	LORD::ui32 elapsedTime = curTime - lastTime;

	Examples::g_app->tick(elapsedTime);

	// 交换缓冲区
	glutSwapBuffers();

	lastTime = curTime;
}

/**
 * Handles a new window size or exposure
 */
static void gears_reshape(int width, int height)
{
	WINDOW_WIDTH = width > height ? width : height;
	WINDOW_HEIGHT = width > height ? height : width;
}


/**
 * Handles special glut events 
 */
static void gears_special(int special, int crap, int morecrap)
{

}


/**
 * gears idle
 */
static void gears_idle()
{
// 	static bool isInit = false;
// 	if (!isInit)
// 	{
// 		printf("-------------------------------emscripten_request_fullscreen\n");
// 
// 		// 开启全屏
// 		EmscriptenFullscreenStrategy s;
// 		memset(&s, 0, sizeof(s));
// 		s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
// 		s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
// 		s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
// 		emscripten_request_fullscreen_strategy(0, 1, &s);
// 
// 		isInit = true;
// 	}

	glutPostRedisplay();
}

/**
 * Gears Init
 */
static void gears_init()
{
	// 创建应用程序
	//g_hwnd = hWnd;
	Examples::g_app = new Examples::Application;
	Examples::g_app->start();
}

/** 
 * html5初始化
 */
static void emscripten_init()
{
	// 消息注册
	emscripten_set_mousedown_callback(0, 0, 1, mouse_callback);
	emscripten_set_mouseup_callback(0, 0, 1, mouse_callback);

	// 默认窗口大小设为屏幕大小
	EmscriptenFullscreenChangeEvent fsce;
	emscripten_get_fullscreen_status(&fsce);
	WINDOW_WIDTH = fsce.screenWidth > 0 ? fsce.screenWidth : 800;
	WINDOW_HEIGHT = fsce.screenHeight > 0 ? fsce.screenHeight : 600;

	// 全屏消息回调
	emscripten_set_fullscreenchange_callback(0, 0, 1, fullscreenchange_callback);
}

/**
* 入口函数
*/
int main(int argc, char* argv[])
{
	// 初始化
	emscripten_init();

	// glut初始化
	glutInit( &argc, argv);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutCreateWindow( "html5 webgl");

	// set up glut callback functions
	glutIdleFunc( gears_idle);
	glutReshapeFunc( gears_reshape);
	glutDisplayFunc( gears_draw);
	glutSpecialFunc( gears_special);

	gears_init();
	glutMainLoop();

	return 0;
}
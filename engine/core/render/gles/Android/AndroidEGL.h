#pragma once

//#ifdef ECHO_PLATFORM_ANDROID
#if 0
#include "GLES2RenderStd.h"

namespace Echo
{
	enum EOpenGLCurrentContext
	{
		CONTEXT_Other = -2,
		CONTEXT_Invalid = -1,
		CONTEXT_Shared = 0,
		CONTEXT_Rendering = 1,
	};

	struct AndroidEGLImpl;
	struct ANativeWindow;

	struct FPlatformOpenGLContext
	{
		EGLContext	eglContext;
		GLuint		ViewportFramebuffer;
		EGLSurface	eglSurface;
		GLuint		DefaultVertexArrayObject;

		FPlatformOpenGLContext()
		{
			Reset();
		}

		void Reset()
		{
			eglContext = EGL_NO_CONTEXT;
			eglSurface = EGL_NO_SURFACE;
			ViewportFramebuffer = 0;
			DefaultVertexArrayObject = 0;
		}
	};

	class AndroidEGL
	{
	public:
		AndroidEGL();
		~AndroidEGL();

		bool IsInitialized();
		void InitBackBuffer();
		void DestroyBackBuffer();
		bool Initialize(ui32 majorVersion, ui32 minorVersion, bool bDebug);
		void ReInit();
		void UnBind();
		bool SwapBuffers();
		void Terminate();
		void InitSurface(bool bUseSmallSurface);

		EGLContext CreateContext(EGLContext sharedContext = EGL_NO_CONTEXT);
		EGLBoolean SetCurrentContext(EGLContext context, EGLSurface surface);
		EGLContext  GetCurrentContext();
		void SetSingleThreadRenderingContext();
		void SetMultithreadRenderingContext();

		GLuint GetResolveFrameBuffer();

		void SetNativeWindow(void* window);
		void* GetNativeWindow();
		void SetScreenSize(ui32 width, ui32 height);

		static AndroidEGL* GetInstance();

	private:
		bool initializeEGL();
		void terminateEGL();

		void createEGLSurface(ANativeWindow* window);
		void destroySurface();

		bool initContexts();
		void destroyContext(EGLContext context);

		void resetDisplay();

	private:
		AndroidEGLImpl* m_impl;
		bool m_supportsKHRCreateContext;
		bool m_supportsKHRSurfacelessContext;
		int *m_contextAttributes;
		void* m_nativeWindow;
		ui32 m_screenWidth;
		ui32 m_screenHeight;

		static AndroidEGL* s_instance;
	};
}

#endif
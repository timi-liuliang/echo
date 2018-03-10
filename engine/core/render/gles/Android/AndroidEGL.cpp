#include "AndroidEGL.h"
//#ifdef ECHO_PLATFORM_ANDROID
#if 0
#include "Foundation/Memory/MemAllocDef.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

namespace Echo
{
	const int EGLMinRedBits			= 5;
	const int EGLMinGreenBits		= 6;
	const int EGLMinBlueBits		= 5;
	const int EGLMinAlphaBits		= 0;
	const int EGLMinDepthBits		= 16;
	const int EGLMinStencilBits		= 0;
	const int EGLMinSampleBuffers	= 0;
	const int EGLMinSampleSamples	= 0;

	struct EGLConfigParms
	{
		int validConfig;
		int redSize;
		int greenSize;
		int blueSize;
		int alphaSize;
		int depthSize;
		int stencilSize;
		int sampleBuffers;
		int sampleSamples;

		EGLConfigParms();
		EGLConfigParms(const EGLConfigParms& Parms);
	};

	EGLConfigParms::EGLConfigParms(const EGLConfigParms& Parms)
	{
		validConfig = Parms.validConfig;
		redSize = Parms.redSize;
		greenSize = Parms.greenSize;
		blueSize = Parms.blueSize;
		alphaSize = Parms.alphaSize;
		depthSize = Parms.depthSize;
		stencilSize = Parms.stencilSize;
		sampleBuffers = Parms.sampleBuffers;
		sampleSamples = Parms.sampleSamples;
	}

	EGLConfigParms::EGLConfigParms()  
		: validConfig (0)
		, redSize(8)
		, greenSize(8)
		, blueSize(8)
		, alphaSize(0)
		, depthSize(24)
		, stencilSize(0)
		, sampleBuffers(0)
		, sampleSamples(0)
	{
	}

	const EGLint Attributes[] = {
		EGL_RED_SIZE, EGLMinRedBits,
		EGL_GREEN_SIZE, EGLMinGreenBits,
		EGL_BLUE_SIZE, EGLMinBlueBits,
		EGL_ALPHA_SIZE, EGLMinAlphaBits,
		EGL_DEPTH_SIZE, EGLMinDepthBits,
		EGL_STENCIL_SIZE, EGLMinStencilBits,
		EGL_SAMPLE_BUFFERS, EGLMinSampleBuffers,
		EGL_SAMPLES, EGLMinSampleSamples,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
		EGL_CONFIG_CAVEAT, EGL_NONE,
		EGL_NONE
	};

	struct AndroidEGLImpl
	{
		FPlatformOpenGLContext SharedContext;
		FPlatformOpenGLContext RenderingContext;
		FPlatformOpenGLContext SingleThreadedContext;

		EGLDisplay eglDisplay;
		EGLint eglNumConfigs;
		EGLint eglFormat;
		EGLConfig eglConfigParam;
		EGLSurface eglSurface;
		EGLSurface auxSurface;
		EGLint eglWidth;
		EGLint eglHeight;
		EGLint NativeVisualID;
		float eglRatio;
		EGLConfigParms Parms;
		int DepthSize;
		uint32_t SwapBufferFailureCount;
		ANativeWindow* Window;
		bool Initalized;
		EOpenGLCurrentContext CurrentContextType;
		GLuint OnScreenColorRenderBuffer;
		GLuint ResolveFrameBuffer;
		AndroidEGLImpl();
	};

	AndroidEGLImpl::AndroidEGLImpl()
		: eglDisplay(EGL_NO_DISPLAY)
		, eglNumConfigs(0)
		, eglFormat(-1)
		, eglConfigParam(nullptr)
		, eglSurface(EGL_NO_SURFACE)
		, auxSurface(EGL_NO_SURFACE)
		, eglWidth(0)
		, eglHeight(0)
		, eglRatio(0)
		, DepthSize(0)
		, SwapBufferFailureCount(0)
		, Window(nullptr)
		, Initalized(false)
		, OnScreenColorRenderBuffer(0)
		, ResolveFrameBuffer(0)
		, NativeVisualID(0)
	{
	}

	AndroidEGL* AndroidEGL::s_instance = nullptr;

	bool AndroidEGL::initializeEGL()
	{
		m_impl->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		EGLBoolean result = eglInitialize(m_impl->eglDisplay, 0, 0);
		if (!result)
		{
			EchoLogError("elgInitialize error: 0x%x ", eglGetError());
			return false;
		}

		String extensions = eglQueryString(m_impl->eglDisplay, EGL_EXTENSIONS);
		m_supportsKHRCreateContext = extensions.find("EGL_KHR_create_context") != String::npos;
		m_supportsKHRSurfacelessContext = extensions.find("EGL_KHR_surfaceless_context") != String::npos;

		result = eglBindAPI(EGL_OPENGL_ES_API);
		if (!result)
		{
			EchoLogError("eglBindAPI error: 0x%x ", eglGetError());
			return false;
		}

		EGLConfig EGLConfigList[1];
		if (!(result = eglChooseConfig(m_impl->eglDisplay, Attributes, EGLConfigList, 1, &m_impl->eglNumConfigs)))
		{
			terminateEGL();
			EchoLogError("eglChooseConfig error: 0x%x", eglGetError());
			return false;
		}

		if (m_impl->eglNumConfigs == 0)
		{
			EchoLogError("eglChooseConfig num EGLConfigLists is 0 . error: 0x%x", eglGetError());
		}

		m_impl->eglConfigParam = EGLConfigList[0];
		int ResultValue = 0;
		eglGetConfigAttrib(m_impl->eglDisplay, EGLConfigList[0], EGL_DEPTH_SIZE, &ResultValue); 
		m_impl->DepthSize = ResultValue;
		eglGetConfigAttrib(m_impl->eglDisplay, EGLConfigList[0], EGL_NATIVE_VISUAL_ID, &ResultValue); 
		m_impl->NativeVisualID = ResultValue;

		return true;
	}

	void AndroidEGL::terminateEGL()
	{
		eglTerminate(m_impl->eglDisplay);
		m_impl->Initalized = false;
	}

	void AndroidEGL::createEGLSurface(ANativeWindow* window)
	{
		if (m_impl->eglSurface != EGL_NO_SURFACE)
		{
			return;
		}

		//need ANativeWindow
		m_impl->eglSurface = eglCreateWindowSurface(m_impl->eglDisplay, m_impl->eglConfigParam, window, NULL);

		if (m_impl->eglSurface == EGL_NO_SURFACE)
		{
			if (m_impl->eglSurface == EGL_NO_SURFACE)
			{
				EchoLogError("eglCreateWindowSurface error : 0x%x", eglGetError());
			}
			Terminate();
			return;
		}

		EGLBoolean result = EGL_FALSE;
		if (!(result = (eglQuerySurface(m_impl->eglDisplay, m_impl->eglSurface, EGL_WIDTH, &m_impl->eglWidth) && eglQuerySurface(m_impl->eglDisplay, m_impl->eglSurface, EGL_HEIGHT, &m_impl->eglHeight))))
		{
			Terminate();
			EchoLogError("eglQuerySurface error : 0x%x", eglGetError());
			return;
		}

		EGLint pbufferAttribs[] =
		{
			EGL_WIDTH, 1,
			EGL_HEIGHT, 1,
			EGL_TEXTURE_TARGET, EGL_NO_TEXTURE,
			EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE,
			EGL_NONE
		};

		pbufferAttribs[1] = m_impl->eglWidth;
		pbufferAttribs[3] = m_impl->eglHeight;

		m_impl->auxSurface = eglCreatePbufferSurface(m_impl->eglDisplay, m_impl->eglConfigParam, pbufferAttribs);
		if (m_impl->auxSurface == EGL_NO_SURFACE)
		{
			if (m_impl->auxSurface == EGL_NO_SURFACE)
			{
				EchoLogError("eglCreatePbufferSurface error : 0x%x", eglGetError());
			}
			Terminate();
		}
	}

	void AndroidEGL::destroySurface()
	{
		if (m_impl->eglSurface != NULL)
		{
			eglDestroySurface(m_impl->eglDisplay, m_impl->eglSurface);
			m_impl->eglSurface = EGL_NO_SURFACE;
		}
		if (m_impl->auxSurface != EGL_NO_SURFACE)
		{
			eglDestroySurface(m_impl->eglDisplay, m_impl->auxSurface);
			m_impl->auxSurface = EGL_NO_SURFACE;
		}
	}

	bool AndroidEGL::initContexts()
	{
		bool Result = true;
		m_impl->SharedContext.eglContext = CreateContext();
		m_impl->RenderingContext.eglContext = CreateContext(m_impl->SharedContext.eglContext);
		m_impl->SingleThreadedContext.eglContext = CreateContext();
		return Result;
	}

	void AndroidEGL::destroyContext(EGLContext context)
	{
		if (context != EGL_NO_CONTEXT)
		{
			eglDestroyContext(m_impl->eglDisplay, context);
		}
	}

	void AndroidEGL::resetDisplay()
	{
		if (m_impl->eglDisplay != EGL_NO_DISPLAY)
		{
			eglMakeCurrent(m_impl->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			m_impl->CurrentContextType = CONTEXT_Invalid;
		}
	}

	AndroidEGL::AndroidEGL()
		: m_supportsKHRCreateContext(false)
		, m_supportsKHRSurfacelessContext(false)
		, m_contextAttributes(nullptr)
		, m_nativeWindow(nullptr)
		, m_screenWidth(0)
		, m_screenHeight(0)
	{
		m_impl = EchoNew(AndroidEGLImpl);
	}

	AndroidEGL::~AndroidEGL()
	{
		EchoSafeDelete(m_impl, AndroidEGLImpl);
	}

	bool AndroidEGL::IsInitialized()
	{
		return m_impl->Initalized;
	}

	void AndroidEGL::InitBackBuffer()
	{
		//add check to see if any context was made current. 
		GLint OnScreenWidth, OnScreenHeight;
		m_impl->ResolveFrameBuffer = 0;
		m_impl->OnScreenColorRenderBuffer = 0;
		OnScreenWidth = m_impl->eglWidth;
		OnScreenHeight = m_impl->eglHeight;

		m_impl->RenderingContext.ViewportFramebuffer = GetResolveFrameBuffer();
		m_impl->SharedContext.ViewportFramebuffer = GetResolveFrameBuffer();
		m_impl->SingleThreadedContext.ViewportFramebuffer = GetResolveFrameBuffer();
	}

	void AndroidEGL::DestroyBackBuffer()
	{
		if (m_impl->ResolveFrameBuffer)
		{
			glDeleteFramebuffers(1, &m_impl->ResolveFrameBuffer);
			m_impl->ResolveFrameBuffer = 0;
		}
		if (m_impl->OnScreenColorRenderBuffer)
		{
			glDeleteRenderbuffers(1, &(m_impl->OnScreenColorRenderBuffer));
			m_impl->OnScreenColorRenderBuffer = 0;
		}
	}

	bool AndroidEGL::Initialize(ui32 majorVersion, ui32 minorVersion, bool bDebug)
	{
		if (m_impl->Initalized)
			return false;
		
		if (!initializeEGL())
			return false;

		if (m_supportsKHRCreateContext)
		{
			const ui32 MaxElements = 13;
			ui32 Flags = 0;

			Flags |= bDebug ? EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0;

			m_contextAttributes = new int[MaxElements];
			ui32 Element = 0;

			m_contextAttributes[Element++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
			m_contextAttributes[Element++] = majorVersion;
			m_contextAttributes[Element++] = EGL_CONTEXT_MINOR_VERSION_KHR;
			m_contextAttributes[Element++] = minorVersion;
			m_contextAttributes[Element++] = EGL_CONTEXT_FLAGS_KHR;
			m_contextAttributes[Element++] = Flags;
			m_contextAttributes[Element++] = EGL_NONE;

			if (Element > MaxElements)
			{
				EchoLogError("Too many elements in config list");
				return false;
			};
		}
		else
		{
			// Fall back to the least common denominator
			m_contextAttributes = new int[3];
			m_contextAttributes[0] = EGL_CONTEXT_CLIENT_VERSION;
			m_contextAttributes[1] = 2;
			m_contextAttributes[2] = EGL_NONE;
		}

		initContexts();
		m_impl->Initalized = true;
	}

	void AndroidEGL::ReInit()
	{
		InitSurface(false);
	}

	void AndroidEGL::UnBind()
	{
		resetDisplay();
		destroySurface();
	}

	bool AndroidEGL::SwapBuffers()
	{
		if (m_impl->eglSurface == nullptr || !eglSwapBuffers(m_impl->eglDisplay, m_impl->eglSurface))
		{
			m_impl->SwapBufferFailureCount++;

			if (m_impl->eglSurface == NULL)
			{
				return false;
			}
			else
			{
				if (eglGetError() == EGL_CONTEXT_LOST)
				{
					EchoLogError("swapBuffers: EGL11.EGL_CONTEXT_LOST");
				}
			}

			return false;
		}

		return true;
	}

	void AndroidEGL::Terminate()
	{
		resetDisplay();
		destroyContext(m_impl->SharedContext.eglContext);
		m_impl->SharedContext.Reset();
		destroyContext(m_impl->RenderingContext.eglContext);
		m_impl->RenderingContext.Reset();
		destroyContext(m_impl->SingleThreadedContext.eglContext);
		m_impl->SingleThreadedContext.Reset();
		destroySurface();
		terminateEGL();
	}

	void AndroidEGL::InitSurface(bool bUseSmallSurface)
	{
		ANativeWindow* window = (ANativeWindow*)GetNativeWindow();
		if (!window)
		{
			EchoLogError("GetNativeWindow() failed.");
			return;
		}

		m_impl->Window = window;
		ANativeWindow_setBuffersGeometry(m_impl->Window, m_screenWidth, m_screenHeight, m_impl->NativeVisualID);
		createEGLSurface(m_impl->Window);

		m_impl->SharedContext.eglSurface = m_impl->auxSurface;
		m_impl->RenderingContext.eglSurface = m_impl->eglSurface;
		m_impl->SingleThreadedContext.eglSurface = m_impl->eglSurface;
	}

	EGLContext AndroidEGL::CreateContext(EGLContext sharedContext /*= EGL_NO_CONTEXT*/)
	{
		return eglCreateContext(m_impl->eglDisplay, m_impl->eglConfigParam,  sharedContext , m_contextAttributes);
	}

	EGLBoolean AndroidEGL::SetCurrentContext(EGLContext context, EGLSurface surface)
	{
		EGLBoolean Result = EGL_FALSE;
		EGLContext CurrentContext = GetCurrentContext();

		if (CurrentContext != context)
		{
			if (CurrentContext != EGL_NO_CONTEXT)
			{
				glFlush();
			}
			if (context == EGL_NO_CONTEXT && surface == EGL_NO_SURFACE)
			{
				resetDisplay();
			}
			else
			{
				Result = eglMakeCurrent(m_impl->eglDisplay, surface, surface, context);
				if (!Result)
				{
					EchoLogError("SetCurrentSharedContext eglMakeCurrent failed : 0x%x", eglGetError());
				}
			}
		}
		return Result;
	}

	EGLContext AndroidEGL::GetCurrentContext()
	{
		return eglGetCurrentContext();
	}

	GLuint AndroidEGL::GetResolveFrameBuffer()
	{
		return m_impl->ResolveFrameBuffer;
	}

	void AndroidEGL::SetNativeWindow(void* window)
	{
		m_nativeWindow = window;
	}

	void* AndroidEGL::GetNativeWindow()
	{
		return m_nativeWindow;
	}

	void AndroidEGL::SetScreenSize(ui32 width, ui32 height)
	{
		m_screenWidth = width;
		m_screenHeight = height;
	}

	AndroidEGL* AndroidEGL::GetInstance()
	{
		if (!s_instance)
		{
			s_instance = EchoNew(AndroidEGL);
		}
		return s_instance;
	}

	void AndroidEGL::SetSingleThreadRenderingContext()
	{
		m_impl->CurrentContextType = CONTEXT_Rendering;
		SetCurrentContext(m_impl->SingleThreadedContext.eglContext, m_impl->SingleThreadedContext.eglSurface);
	}

	void AndroidEGL::SetMultithreadRenderingContext()
	{
		m_impl->CurrentContextType = CONTEXT_Rendering;
		SetCurrentContext(m_impl->RenderingContext.eglContext, m_impl->RenderingContext.eglSurface);
	}

}

#endif
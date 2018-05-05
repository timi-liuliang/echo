#include "GLES2RenderStd.h"
#include "GLES2Loader.h"

namespace Echo
{
	GLES2Loader::GLES2Loader()
	{
	}

	GLES2Loader::~GLES2Loader()
	{
	}

	// instance
	GLES2Loader* GLES2Loader::instance()
	{
		static GLES2Loader* inst = EchoNew(GLES2Loader);
		return inst;
	}

	bool GLES2Loader::initialize()
	{
	#if defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_WINDOWS)
		genVertexArraysOES = (ECHO_PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
		if(!genVertexArraysOES)
			EchoLogDebug("Load GLES extensions glGenVertexArraysOES() failed...");

		bindVertexArrayOES = (ECHO_PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
		if(!bindVertexArrayOES)
			EchoLogDebug("Load GLES extensions glBindVertexArrayOES() failed...");

		deleteVertexArraysOES = (ECHO_PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
		if(!deleteVertexArraysOES)
			EchoLogDebug("Load GLES extensions glDeleteVertexArrayOES() failed...");

		isVertexArrayOES = (ECHO_PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");
		if(!isVertexArrayOES)
			EchoLogDebug("Load GLES extensions glEsVertexArrayOES() failed...");

		mapBufferOES = (ECHO_PFNGLMAPBUFFEROESPROC)eglGetProcAddress("glMapBufferOES");
		if(!mapBufferOES)
			EchoLogDebug("Load GLES extensions glMapBufferOES() failed...");

		unmapBufferOES = (ECHO_PFNGLUNMAPBUFFEROESPROC)eglGetProcAddress("glUnmapBufferOES");
		if(!unmapBufferOES)
			EchoLogDebug("Load GLES extensions glUnmapBufferOES() failed...");
	#endif
		return true;
	}

}

#include "GLES2RenderStd.h"
#include "GLES2Loader.h"

namespace LORD
{
	GLES2Loader::GLES2Loader()
	{
	}

	GLES2Loader::~GLES2Loader()
	{
	}

	bool GLES2Loader::initialize()
	{
	#if defined(LORD_PLATFORM_ANDROID) || defined(LORD_PLATFORM_WINDOWS)
		genVertexArraysOES = (LORD_PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
		if(!genVertexArraysOES)
			LordLogDebug("Load GLES extensions glGenVertexArraysOES() failed...");

		bindVertexArrayOES = (LORD_PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
		if(!bindVertexArrayOES)
			LordLogDebug("Load GLES extensions glBindVertexArrayOES() failed...");

		deleteVertexArraysOES = (LORD_PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
		if(!deleteVertexArraysOES)
			LordLogDebug("Load GLES extensions glDeleteVertexArrayOES() failed...");

		isVertexArrayOES = (LORD_PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");
		if(!isVertexArrayOES)
			LordLogDebug("Load GLES extensions glEsVertexArrayOES() failed...");

		mapBufferOES = (LORD_PFNGLMAPBUFFEROESPROC)eglGetProcAddress("glMapBufferOES");
		if(!mapBufferOES)
			LordLogDebug("Load GLES extensions glMapBufferOES() failed...");

		unmapBufferOES = (LORD_PFNGLUNMAPBUFFEROESPROC)eglGetProcAddress("glUnmapBufferOES");
		if(!unmapBufferOES)
			LordLogDebug("Load GLES extensions glUnmapBufferOES() failed...");
	#endif
		return true;
	}

}

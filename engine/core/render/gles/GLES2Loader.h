#ifndef __ECHO_GLES2LOADER_H__
#define __ECHO_GLES2LOADER_H__

#include "engine/core/Util/Singleton.h"

#ifdef ECHO_PLATFORM_ANDROID
	#ifndef GL_APIENTRYP

	#define GL_APIENTRYP GL_APIENTRY*

	#endif
#endif

namespace Echo
{
	class ECHO_GLES2RENDER_API GLES2Loader: public Singleton<GLES2Loader>
	{
		typedef void (GL_APIENTRYP ECHO_PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
		typedef void (GL_APIENTRYP ECHO_PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint* arrays);
		typedef void (GL_APIENTRYP ECHO_PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint* arrays);
		typedef GLboolean (GL_APIENTRYP ECHO_PFNGLISVERTEXARRAYOESPROC) (GLuint array);

		typedef void* (GL_APIENTRYP ECHO_PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
		typedef GLboolean (GL_APIENTRYP ECHO_PFNGLUNMAPBUFFEROESPROC) (GLenum target);
		typedef void (GL_APIENTRYP ECHO_PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, GLvoid** params);

	public:
		GLES2Loader();
		~GLES2Loader();

	public:
		bool initialize();

		ECHO_PFNGLGENVERTEXARRAYSOESPROC		genVertexArraysOES;
		ECHO_PFNGLBINDVERTEXARRAYOESPROC		bindVertexArrayOES;
		ECHO_PFNGLDELETEVERTEXARRAYSOESPROC		deleteVertexArraysOES;
		ECHO_PFNGLISVERTEXARRAYOESPROC			isVertexArrayOES;

		ECHO_PFNGLMAPBUFFEROESPROC				mapBufferOES;
		ECHO_PFNGLUNMAPBUFFEROESPROC			unmapBufferOES;
		ECHO_PFNGLGETBUFFERPOINTERVOESPROC		getBufferPointervOES;
	};
}

#endif

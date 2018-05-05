#pragma once

#ifdef ECHO_PLATFORM_ANDROID
	#ifndef GL_APIENTRYP

	#define GL_APIENTRYP GL_APIENTRY*

	#endif
#endif

namespace Echo
{
	class GLES2Loader
	{
		typedef void (GL_APIENTRYP ECHO_PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
		typedef void (GL_APIENTRYP ECHO_PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint* arrays);
		typedef void (GL_APIENTRYP ECHO_PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint* arrays);
		typedef GLboolean (GL_APIENTRYP ECHO_PFNGLISVERTEXARRAYOESPROC) (GLuint array);

		typedef void* (GL_APIENTRYP ECHO_PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
		typedef GLboolean (GL_APIENTRYP ECHO_PFNGLUNMAPBUFFEROESPROC) (GLenum target);
		typedef void (GL_APIENTRYP ECHO_PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, GLvoid** params);

	public:
		~GLES2Loader();

		// instance
		static GLES2Loader* instance();

	public:
		bool initialize();

		ECHO_PFNGLGENVERTEXARRAYSOESPROC		genVertexArraysOES;
		ECHO_PFNGLBINDVERTEXARRAYOESPROC		bindVertexArrayOES;
		ECHO_PFNGLDELETEVERTEXARRAYSOESPROC		deleteVertexArraysOES;
		ECHO_PFNGLISVERTEXARRAYOESPROC			isVertexArrayOES;

		ECHO_PFNGLMAPBUFFEROESPROC				mapBufferOES;
		ECHO_PFNGLUNMAPBUFFEROESPROC			unmapBufferOES;
		ECHO_PFNGLGETBUFFERPOINTERVOESPROC		getBufferPointervOES;

	private:
		GLES2Loader();
	};
}
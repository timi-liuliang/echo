#ifndef __LORD_GLES2LOADER_H__
#define __LORD_GLES2LOADER_H__

#include "Foundation/Util/Singleton.h"

namespace LORD
{
	class LORD_GLES2RENDER_API GLES2Loader: public ObjectAlloc, public Singleton<GLES2Loader>
	{
		typedef void (GL_APIENTRYP LORD_PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
		typedef void (GL_APIENTRYP LORD_PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint* arrays);
		typedef void (GL_APIENTRYP LORD_PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint* arrays);
		typedef GLboolean (GL_APIENTRYP LORD_PFNGLISVERTEXARRAYOESPROC) (GLuint array);

		typedef void* (GL_APIENTRYP LORD_PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
		typedef GLboolean (GL_APIENTRYP LORD_PFNGLUNMAPBUFFEROESPROC) (GLenum target);
		typedef void (GL_APIENTRYP LORD_PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, GLvoid** params);

	public:
		GLES2Loader();
		~GLES2Loader();

	public:
		bool initialize();

		LORD_PFNGLGENVERTEXARRAYSOESPROC		genVertexArraysOES;
		LORD_PFNGLBINDVERTEXARRAYOESPROC		bindVertexArrayOES;
		LORD_PFNGLDELETEVERTEXARRAYSOESPROC		deleteVertexArraysOES;
		LORD_PFNGLISVERTEXARRAYOESPROC			isVertexArrayOES;

		LORD_PFNGLMAPBUFFEROESPROC				mapBufferOES;
		LORD_PFNGLUNMAPBUFFEROESPROC			unmapBufferOES;
		LORD_PFNGLGETBUFFERPOINTERVOESPROC		getBufferPointervOES;
	};
}

#endif

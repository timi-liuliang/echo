#ifndef __LORD_GLES2RENDERSTD_H__
#define __LORD_GLES2RENDERSTD_H__

#include <Foundation/Base/LordDef.h>
#include <Foundation/Util/LogManager.h>

#ifdef LORD_PLATFORM_WINDOWS
// #	ifdef LORD_GLES2RENDER_STATIC
#		define LORD_GLES2RENDER_API
// #	else
// #		ifdef LORD_GLES2RENDER_EXPORTS
// #			define LORD_GLES2RENDER_API __declspec(dllexport)
// #		else
// #			define LORD_GLES2RENDER_API __declspec(dllimport)
// #		endif
// #	endif
#else
#	define LORD_GLES2RENDER_API
#endif

#ifndef GL_GLEXT_PROTOTYPES
#  define  GL_GLEXT_PROTOTYPES
#endif

#ifdef LORD_PLATFORM_MAC_IOS
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
#   endif
#elif defined(LORD_PLATFORM_ANDROID) || defined(LORD_PLATFORM_NACL)
#	include <GLES2/gl2platform.h>
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#	include <EGL/egl.h>
#	if defined(LORD_PLATFORM_NACL)
#		include "ppapi/cpp/completion_callback.h"
#       include "ppapi/cpp/instance.h"
#       include "ppapi/c/ppp_graphics_3d.h"
#       include "ppapi/cpp/graphics_3d.h"
#       include "ppapi/cpp/graphics_3d_client.h"
#		include "ppapi/gles2/gl2ext_ppapi.h"
#       undef GL_OES_get_program_binary
#       undef GL_OES_mapbuffer
#	endif
#elif defined(LORD_PLATFORM_HTML5)
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
#else
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#   include <GLES3/gl3.h>
#   include <GLES3/gl3ext.h>
#   include <EGL/egl.h>
#   include <EGL/eglext.h>
#endif

#if defined(LORD_PLATFORM_ANDROID) || defined(LORD_PLATFORM_WINDOWS)
#	define LORD_LOAD_GLES_EXT
#endif

namespace LORD
{
	enum GLES2Error
	{
		GLES2ERR_NO_ERROR,
		GLES2ERR_INVALID_ENUM,
		GLES2ERR_INVALID_VALUE,
		GLES2ERR_INVALID_OPERATION,
		GLES2ERR_STACK_OVERFLOW,
		GLES2ERR_STACK_UNDERFLOW,
		GLES2ERR_OUT_OF_MEMORY,
	};

	void DebugGLError(const char* filename, int lineNum);

#ifdef _DEBUG
	#define OGLESDebug(Func) Func;LORD::DebugGLError( __FILE__, __LINE__ );
#else
	#define OGLESDebug(Func)  Func;
#endif

}

#endif

#pragma once

#include <engine/core/base/echo_def.h>
#include <engine/core/log/Log.h>

#ifndef GL_GLEXT_PROTOTYPES
#  define  GL_GLEXT_PROTOTYPES
#endif

#ifdef ECHO_PLATFORM_MAC_IOS
#	include <OpenGLES/ES3/gl.h>
#	include <OpenGLES/ES3/glext.h>
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
#   endif
#elif defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_NACL)
#	include <GLES3/gl3.h>
#	include <GLES3/gl3ext.h>
#	include	<GLES3/gl3platform.h>
#	include <EGL/egl.h>
#	if defined(ECHO_PLATFORM_NACL)
#		include "ppapi/cpp/completion_callback.h"
#       include "ppapi/cpp/instance.h"
#       include "ppapi/c/ppp_graphics_3d.h"
#       include "ppapi/cpp/graphics_3d.h"
#       include "ppapi/cpp/graphics_3d_client.h"
#		include "ppapi/gles2/gl2ext_ppapi.h"
#       undef GL_OES_get_program_binary
#       undef GL_OES_mapbuffer
#	endif
#ifndef GL_OES_texture_half_float
#define GL_HALF_FLOAT_OES                                       0x8D61
#endif

/* GL_OES_packed_depth_stencil */
#ifndef GL_OES_packed_depth_stencil
#define GL_DEPTH_STENCIL_OES                                    0x84F9
#define GL_UNSIGNED_INT_24_8_OES                                0x84FA
#define GL_DEPTH24_STENCIL8_OES                                 0x88F0
#endif

/* GL_EXT_read_format_bgra */
#ifndef GL_EXT_read_format_bgra
#define GL_BGRA_EXT                                             0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT                       0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT                       0x8366
#endif

/* GL_OES_depth32 */
#ifndef GL_OES_depth32
#define GL_DEPTH_COMPONENT32_OES                                0x81A7
#endif

/* GL_IMG_texture_compression_pvrtc */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#endif

#define GL_MIN_EXT                        0x8007

#define GL_MAX_EXT                        0x8008

#elif defined(ECHO_PLATFORM_HTML5)
	#include <GLES3/gl3.h>
	#include <GLES3/gl3ext.h>
	#include <EGL/egl.h>
#else
#   include <GLES3/gl3.h>
#ifndef ECHO_POWERVR_SDK
#   include <GLES3/gl3ext.h>
#endif
#   include <EGL/egl.h>
#   include <EGL/eglext.h>
#endif

#if defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_WINDOWS)
#	define ECHO_LOAD_GLES_EXT
#endif

namespace Echo
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
	#define OGLESDebug(Func) Func;Echo::DebugGLError( __FILE__, __LINE__ );
#else
	#define OGLESDebug(Func)  Func;
#endif

}

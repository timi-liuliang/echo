/* -*- mode: c; tab-width: 8; -*- */
/* vi: set sw=4 ts=8: */
/* Platform-specific types and definitions for egl.h
 * Last modified 2008/10/22
 *
 * If you make additions or modifications to eglplatform.h specific to
 * your implementation or runtime environment, please send them to
 * Khronos (preferably by filing a bug in the member or public Bugzillas
 * and attaching a copy) for possible inclusion in future versions.
 */

/*
** Copyright (c) 2007-2008 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#ifndef __eglplatform_h_
#define __eglplatform_h_

// AMD commented out this include so WinCE would build
//#include <sys/types.h>
#include <KHR/khrplatform.h>

/* Macros used in EGL function prototype declarations.
 *
 * EGLAPI return-type EGLAPIENTRY eglFunction(arguments);
 * typedef return-type (EXPAPIENTRYP PFNEGLFUNCTIONPROC) (arguments);
 *
 * On Windows, EGLAPIENTRY can be defined like APIENTRY.
 * On most other platforms, it should be empty.
 */

#ifndef EGLAPI
#  if (defined(_WIN32) || defined(__VC32__)) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__) /* Win32 and WinCE */
#    ifdef __EGL_EXPORTS
#      define EGLAPI __declspec(dllexport)
#    else
#      define EGLAPI __declspec(dllimport)
#    endif
#  elif defined (__SYMBIAN32__)            /* Symbian */
#    define EGLAPI IMPORT_C
#  else
#    define EGLAPI
#  endif
#endif

#if (defined(_WIN32) || defined(__VC32__)) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__) && !defined(_WIN32_WCE) /* Win32 */
#define EGLAPIENTRY __stdcall
#else
#define EGLAPIENTRY
#endif

#define EGLAPIENTRYP EGLAPIENTRY *

/* The types NativeDisplayType, NativeWindowType, and NativePixmapType
 * are aliases of window-system-dependent types, such as X Display * or
 * Windows Device Context. They must be defined in platform-specific
 * code below. The EGL-prefixed versions of Native*Type are the same
 * types, renamed in EGL 1.3 so all types in the API start with "EGL".
 */

#if defined(_WIN32) || defined(__VC32__) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)  /* Win32 and WinCE */
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

typedef HDC     EGLNativeDisplayType;
typedef HBITMAP EGLNativePixmapType;
typedef HWND    EGLNativeWindowType;

/*
typedef HDC NativeDisplayType;
typedef HBITMAP NativePixmapType;
typedef HWND NativeWindowType;
*/

#elif defined(__WINSCW__) || defined(__SYMBIAN32__) /* Symbian */

typedef int   EGLNativeDisplayType;
typedef void *EGLNativeWindowType;
typedef void *EGLNativePixmapType;
/*
typedef int NativeDisplayType;
typedef void *NativeWindowType;
typedef void *NativePixmapType;
*/
#elif defined(__ANDROID__) || defined(ANDROID)

#include <android/native_window.h>

struct egl_native_pixmap_t;

typedef struct ANativeWindow*           EGLNativeWindowType;
typedef struct egl_native_pixmap_t*     EGLNativePixmapType;
typedef void*                           EGLNativeDisplayType;

#elif defined (__ARMCC_VERSION) || defined(__ARM__) || defined(_LINUX)

typedef void *EGLNativeDisplayType;
typedef void *EGLNativeWindowType;
typedef void *EGLNativePixmapType;
#define NativeDisplayType void *
#define NativeWindowType  void *
#define NativePixmapType  void *

/*
#define NativeDisplayType void *
#define NativeWindowType  void *
#define NativePixmapType  void *
*/

#elif defined(__APPLE__) || defined(__linux__)

/* X11 (tentative)  */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef Display *EGLNativeDisplayType;
typedef Pixmap   EGLNativePixmapType;
typedef Window   EGLNativeWindowType;
/*
typedef Display *NativeDisplayType;
typedef Pixmap NativePixmapType;
typedef Window NativeWindowType;
*/
#else
#error "Platform not recognized"
#endif

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType  NativePixmapType;
typedef EGLNativeWindowType  NativeWindowType;


/*
typedef NativeDisplayType EGLNativeDisplayType;
typedef NativePixmapType  EGLNativePixmapType;
typedef NativeWindowType  EGLNativeWindowType;
*/
/*
 Define EGLint. This must be an integral type large enough to contain
 * all legal attribute names and values passed into and out of EGL,
 * whether their type is boolean, bitmask, enumerant (symbolic
 * constant), integer, handle, or other.
 * While in general a 32-bit integer will suffice, if handles are
 * represented as pointers, then EGLint should be defined as a 64-bit
 * integer type.
 */

 typedef khronos_int32_t EGLint;

 
 /*
 #if (defined(__STDC__) && __STDC__ && __STDC_VERSION__ >= 199901L) || defined(__GNUC__) || defined (__ARMCC_VERSION) || defined(__ARM__) || defined(_LINUX)
#include <stdint.h>
typedef int32_t EGLint;
#elif (defined(_WIN32) || defined(__VC32__)) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)  // Win32 and WinCE
#include <windows.h>
typedef INT32 EGLint;
#else
typedef int EGLint;
#endif
*/

#endif /* __eglplatform_h */

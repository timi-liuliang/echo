#pragma once

#include "echo_config.h"
#include "type_def.h"
#include <assert.h>

#include <unordered_map>
#include <unordered_set>

// Echo Engine version
#define ECHO_VERSION 1.3

// Mode
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
	#define ECHO_DEBUG
#else
	#define ECHO_RELEASE
#endif

// Disable Warnings
#pragma warning(disable:4996)
#pragma warning(disable:4819)

// Platform recognition
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#define ECHO_PLATFORM_WINDOWS
#elif defined(_WIN64) || defined(__WIN64__) || defined(WIN64)
	#define  ECHO_PLATFORM_WINDOWS
#elif defined(__APPLE_CC__)
// Device                                                     Simulator
// Both requiring OS version 4.0 or greater
#   if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 40000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 40000
#       define ECHO_PLATFORM_MAC_IOS
#   else
#       define ECHO_PLATFORM_MAC_OSX
#   endif
#elif defined(__ANDROID__)
#	define ECHO_PLATFORM_ANDROID
#elif defined(linux) || defined(__linux) || defined(__linux__)
#	define ECHO_PLATFORM_LINUX
#elif defined(__native_client__)
#   define ECHO_PLATFORM_NACL
#elif defined(__EMSCRIPTEN__)
	#define ECHO_PLATFORM_HTML5	
#else
#	error "Couldn't recognize platform"
#endif

#define INLINE inline

// Initial compiler-related stuff to set.
#define ECHO_COMPILER_MSVC		1
#define ECHO_COMPILER_CLANG		2
#define ECHO_COMPILER_GNUC		3

// CPU Architecture
#define ECHO_ARCH_32BIT         1
#define ECHO_ARCH_64BIT         2
#define ECHO_ARCH_ARM			3

// Endian
#define ECHO_ENDIAN_LITTLE      1
#define ECHO_ENDIAN_BIG         2

// Endian Settings
// static union { char c[4]; unsigned long nVar; } _EchoEndian = { { 'L', '?', '?', 'B' } };
// static const int _EchoEndianFlag = ((char)_EchoEndian.nVar == 'L' ? ECHO_ENDIAN_LITTLE : ECHO_ENDIAN_BIG);
#define ECHO_ENDIAN     ECHO_ENDIAN_LITTLE

#define ECHO_UNUSED(a) (void)a

// Asserts expression is true at compile-time
#define ECHO_COMPILER_ASSERT(x)	typedef int COMPILER_ASSERT_[!!(x)]

// STL Type Define
#if ((ECHO_COMPILER == ECHO_COMPILER_GNUC) && (ECHO_COMPILER_VERSION >= 310) && !defined(STLPORT))
#   if ECHO_COMPILER_VERSION >= 430
#       define EchoHashMap	::std::tr1::unordered_map
#		define EchoHashSet	::std::tr1::unordered_set
#		define EchoHashCode	::std::tr1::hash
#    else
#       define EchoHashMap	::__gnu_cxx::hash_map
#       define EchoHashSet	::__gnu_cxx::hash_set
#       define EchoHashCode	::std::hash
#    endif
#elif ECHO_COMPILER == ECHO_COMPILER_CLANG
#    if defined(_LIBCPP_VERSION)
#       define EchoHashMap	::std::unordered_map
#       define EchoHashSet	::std::unordered_set
#       define EchoHashCode	::std::hash
#    else
#       define EchoHashMap	::std::tr1::unordered_map
#       define EchoHashSet	::std::tr1::unordered_set
#       define EchoHashCode	::std::tr1::hash
#    endif
#else
#   if ECHO_COMPILER == ECHO_COMPILER_MSVC
#       if ECHO_COMPILER_VERSION >= 100 // VC++ 10.0
#			define EchoHashMap	::std::tr1::unordered_map
#           define EchoHashSet	::std::tr1::unordered_set
#			define EchoHashCode	::std::tr1::hash
#		elif ECHO_COMPILER_VERSION > 70 && !defined(_STLP_MSVC)
#           define EchoHashMap	::stdext::hash_map
#           define EchoHashSet	::stdext::hash_set
#           define EchoHashCode	::stdext::hash
#       else
#           define EchoHashMap	::std::hash_map
#           define EchoHashSet	::std::hash_set
#           define EchoHashCode	::std::hash
#		endif
#	else
		#define EchoHashMap	::std::unordered_map
		#define EchoHashSet	::std::unordered_set
		#define EchoHashCode	::std::hash
#	endif
#endif

#if defined(ECHO_PLATFORM_MAC_IOS) || defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_ANDROID)
#define ECHO_RENDER_THREAD_LOCK_FREE
#endif

// 提供两个down cast的安全版本，分引用版和指针版
template<class To,class From>
To ECHO_DOWN_CAST(From& parent)
{
	typedef typename std::remove_reference<To>::type* ToAsPointer;

#ifdef ECHO_DEBUG
	ToAsPointer to = dynamic_cast<ToAsPointer>(&parent);
	if (to)
		return *to;
	else
		return nullptr;
#else
	return *static_cast<ToAsPointer>(&parent);
#endif
}

template<class To, class From>
To ECHO_DOWN_CAST(From* parent)
{
#ifdef ECHO_DEBUG
	To to = dynamic_cast<To>(parent);
	return to;
#else
	return static_cast<To>(parent);
#endif
}

// Charset
#define ECHO_CHARSET_UNICODE    1
#define ECHO_CHARSET_MULTIBYTE  2

// Fmod on-off
#define ECHO_FMOD_OFF			0

// Compiler type and version recognition
#if defined( _MSC_VER )
#   define ECHO_COMPILER            ECHO_COMPILER_MSVC
#	if _MSC_VER >= 1700
#		define ECHO_COMPILER_VERSION 110
#	elif _MSC_VER >= 1600
#		define ECHO_COMPILER_VERSION 100
#	elif _MSC_VER >= 1500
#		define ECHO_COMPILER_VERSION 90
#	elif _MSC_VER >= 1400
#		define ECHO_COMPILER_VERSION 80
#	elif _MSC_VER >= 1300
#		define ECHO_COMPILER_VERSION 70
#	endif
#elif defined( __clang__ )
#   define ECHO_COMPILER			ECHO_COMPILER_CLANG
#   define ECHO_COMPILER_VERSION	(((__clang_major__)*100) + \
(__clang_minor__*10) + \
__clang_patchlevel__)
#elif defined( __GNUC__ )
#   define ECHO_COMPILER            ECHO_COMPILER_GNUC
#   define ECHO_COMPILER_VERSION    (((__GNUC__)*100) + \
(__GNUC_MINOR__*10) + \
__GNUC_PATCHLEVEL__)
#else
#   error "Unknown compiler. Abort! Abort!"
#endif

// CPU architechture type recognition
#if defined(_M_X64) || defined(__x86_64__) || defined(__powerpc64__) || defined(__alpha__) || defined(__ia64__) || defined(__s390__) || defined(__s390x__)
#	define ECHO_ARCHITECTURE        ECHO_ARCH_64BIT
#elif defined(_M_ARM) || defined(__arm__)
#	define ECHO_ARCHITECTURE		ECHO_ARCH_ARM
//#elif defined(_M_IX86) || defined(__i386__)
#else
#	define ECHO_ARCHITECTURE        ECHO_ARCH_32BIT
#endif

// Disable MSVC warning
#if (ECHO_COMPILER == ECHO_COMPILER_MSVC)
#	pragma warning(disable:4251 4275 4819)
#	ifndef _CRT_SECURE_NO_DEPRECATE
#		define _CRT_SECURE_NO_DEPRECATE
#	endif
#	ifndef _SCL_SECURE_NO_DEPRECATE
#		define _SCL_SECURE_NO_DEPRECATE
#	endif
#endif


// Charset Settings
#if defined(_UNICODE) || defined(UNICODE)
#	define ECHO_CHARSET     ECHO_CHARSET_UNICODE
#else
#	define ECHO_CHARSET     ECHO_CHARSET_MULTIBYTE
#endif


#define ECHO_ALIGN16 __declspec(align(16))
#define ECHO_SIMD_ALIGNMENT 16

// Log
#define ECHO_LOG_FILENAME			"Echo.log"

#if (ECHO_COMPILER == ECHO_COMPILER_MSVC)
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#	define _CRT_NON_CONFORMING_SWPRINTFS	
#endif
#endif

#ifdef ECHO_PLATFORM_WINDOWS
#define ECHO_TYPENAME 
#else
#define ECHO_TYPENAME typename
#endif

#ifdef ECHO_PLATFORM_WINDOWS
#define EchoStrcpy( dest, size, src) strcpy_s( dest, size, src)
#define EchoSprintf( dest, size, formats, ...) sprintf_s( dest, size, formats, ##__VA_ARGS__)
#else
#define EchoStrcpy( dest, size, src) strcpy( dest, src)
#define EchoSprintf( dest, size, formats, ...) sprintf( dest, formats, ##__VA_ARGS__)
#endif

// STD including
#ifndef __MFC_FRAME_WORK__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <typeinfo>
#include <cmath>
#include <float.h>
#include <assert.h>
#include <time.h>
#endif

// STL including
#include <limits>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <exception>

#ifdef ECHO_PLATFORM_WINDOWS
	#include <windows.h>
#endif
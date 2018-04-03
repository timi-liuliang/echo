////////////////////////////////////////////////////////////////////////////////
//	基本类型定义
//	2010.05.02 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

//定义NULL
#ifndef NULL
#define NULL 0
#endif

//platform
#ifdef _WIN32
	#define SCL_WIN	1
	#ifdef _WIN64
		#define SCL_WIN64 1
	#else
		#define SCL_WIN32 1
	#endif
#endif //END OF _WIN32

#if defined(linux) && !defined(__ANDROID__)
	#define SCL_LINUX 1
	#ifdef __i386__
		#define SCL_LINUX32 1
	#endif
	#ifdef __x86_64__
		#define SCL_LINUX64 1
	#endif
#endif //END OF linux

#ifdef __APPLE__
    #define SCL_APPLE 1
    #if defined(__x86_64__) || defined(__ppc64__) || defined(__arm64__) || defined(__LP64__)
        #define SCL_APPLE64 1
    #else
        #define SCL_APPLE32 1
    #endif
    #if defined(TARGET_OS_IPHONE)
        #define SCL_APPLE_IOS 1
    #elif defined(TARGET_OS_MAC)
        #define SCL_APPLE_MAC 1
    #endif
#endif //END OF linux

#ifdef __ANDROID__
	#define SCL_ANDROID 1
#endif

#ifdef __EMSCRIPTEN__
	#define SCL_HTML5 1 
#endif

namespace scl {

typedef char				int8;
typedef unsigned char		uchar;
typedef unsigned char		uint8;
typedef unsigned char		byte;
typedef short				int16;
typedef unsigned short		uint16;
typedef int					int32;
typedef unsigned int		uint;
typedef unsigned int		uint32;
typedef wchar_t				wchar;

#ifdef SCL_WIN32
typedef __int64				int64;
typedef unsigned __int64	uint64;
#else
typedef long long			int64;
typedef unsigned long long	uint64;
#endif

} // namespace scl

#define USING_SCL_TYPE using scl::int8; using scl::uchar; using scl::uint8; using scl::byte; using scl::int16; using scl::uint16; using scl::int32; using scl::uint; using scl::uint32; using scl::wchar; using scl::int64; using scl::uint64;

#ifndef DISABLE_SCL_TYPE
USING_SCL_TYPE;
#endif

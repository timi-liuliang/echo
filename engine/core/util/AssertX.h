#ifndef __ECHO_ASSERTX_H__
#define __ECHO_ASSERTX_H__

#include <assert.h>
#include "StringUtil.h"

namespace Echo
{
	// Assert function return values
	enum ErrRet
	{
		ERRRET_IGNORE = 0,
		ERRRET_CONTINUE,
		ERRRET_BREAKPOINT,
		ERRRET_ABORT
	};

	//- Global functions
	ErrRet NotifyAssert(const char* condition, const char* fileName, int lineNumber, const char* formats, ...);
}

#ifdef ECHO_DEBUG
#	ifdef ECHO_PLATFORM_WINDOWS
#		define EchoAssertX(x, formats, ...) { \
	static bool _ignoreAssert = false; \
	if (!_ignoreAssert && !(x)) \
	{ \
	Echo::ErrRet _err_result = Echo::NotifyAssert(#x, __FILE__, __LINE__, formats, ##__VA_ARGS__); \
	if (_err_result == Echo::ERRRET_IGNORE) \
	{ \
	_ignoreAssert = true; \
	} \
	else if (_err_result == Echo::ERRRET_BREAKPOINT) \
	{ \
	__debugbreak(); \
	} \
	}}
#	else

#define EchoAssertX(x, formats, ...) { \
	if (!(x)) \
	{ \
	Echo::String comment = Echo::StringUtil::Format(formats, ##__VA_ARGS__); \
	Echo::String msg; \
	if(!comment.empty()){ msg = "Assert comment:" + comment + "\n"; } \
	fprintf (stderr, "%s", msg.c_str()); \
	assert(x); \
	exit(-1);\
	}}

#	endif
#else
#	define EchoAssertX(x, formats, ...)
#endif

#ifdef ECHO_DEBUG
#	define EchoAssert(x)		EchoAssertX(x, "")
#else
#	define EchoAssert(x)
#endif

#endif

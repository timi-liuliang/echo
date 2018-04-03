////////////////////////////////////////////////////////////////////////////////
//	string
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

////////////////////////////////////////////////////////////////////////

#include "scl/string.h"
#include "scl/wstring.h"
#include "scl/stringdef.h"

namespace scl {

////////////////////////////////////
//	_ANSI
//	这个宏的主要作用是:
//		把源代码中的char*中文，转换为指定编码的char*中文
//		具体转换为哪种char*编码，请参考宏_SCL_ENCODING_GBK_的说明
////////////////////////////////////
#define _ANSI(s) scl::debug_to_ansi(s).c_str()


#define MAX_DEBUG_SOURCE_STRING_LENGTH 128

#ifdef SCL_WIN
//参见#define _ANSI(s)的说明
inline string<MAX_DEBUG_SOURCE_STRING_LENGTH> debug_to_ansi(char* s)
{
#ifdef _SCL_ENCODING_GBK_
	return s;
#else
	wstring<MAX_DEBUG_SOURCE_STRING_LENGTH> ws;
	ws.from_gbk(s); //由于在windows下，源代码总是gbk编码，所以这里from_gbk
	string<MAX_DEBUG_SOURCE_STRING_LENGTH> ss;
	ws.to_ansi(ss.c_str(), ss.capacity());
	return ss;
#endif
}
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
//参见#define _ANSI(s)的说明
inline string<MAX_DEBUG_SOURCE_STRING_LENGTH> debug_to_ansi(char* s)
{
#ifdef _SCL_ENCODING_GBK_
	wstring<MAX_DEBUG_SOURCE_STRING_LENGTH> ws;
	ws.from_utf8(s); //由于在linux下，源代码总是utf8编码，所以这里from_utf8
	string<MAX_DEBUG_SOURCE_STRING_LENGTH> ss;
	ws.to_ansi(ss.c_str(), ss.capacity());
	return ss;
#else
	return s;
#endif
}
#endif




} //namespace scl



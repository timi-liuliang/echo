
/*******************************************************************
 *
 *    DESCRIPTION:      Basic string definitions header file
 *
 *    AUTHOR:           Tom Hudson
 *
 *    HISTORY:          File created 9/6/94
 *
 *******************************************************************/

#pragma once

#define WIN95STUFF

// To set up Max to use Unicode, define _UNICODE, and don't define _MBCS
// To set up Max to use multi-byte character sets, define _MBCS and 
//              don't define _UNICODE
// To set up Max to use single-byte ANSI character strings, don't define
//              either _UNICODE or _MBCS

#if defined(_UNICODE) && defined(_MBCS)
#error Cannot compile with both _UNICODE and _MBCS enabled!
#endif

// #define _UNICODE     // turn on Unicode support

#ifndef _MBCS
#define _MBCS   // if Unicode is off, turn on multi-byte character support
#endif


#ifdef _UNICODE

#ifdef _MBCS
#undef _MBCS    // can't have both Unicode and MBCS at once -- Unicode wins
#endif

#undef UNICODE
#define UNICODE

#undef STRCONST
#define STRCONST L

#endif

// Bring in the generic international text header file
#include <tchar.h>

// Starting with VS2005 the default is to have wchar_t defined as a type, so that's
// what we'll use as well (plus it coincides with the VS2005 Max9 release)
typedef __wchar_t       mwchar_t;

//
// MAX is not compiled in UNICODE (yet).  TCHAR has been changed to MCHAR where
// appropriate, so you can compile in UNICODE or not, and not have Max's headers
// change.
// 

#ifdef _UNICODE
	// If the module uses Unicode and needs to translate to
	// an MBCS Max, define _UNICODE_MODULE_FOR_MBCS_MAX
	#ifdef _UNICODE_MODULE_FOR_MBCS_MAX
		#define MCHAR           char
		#define LPCMSTR         const MCHAR*
		#define LPMSTR			MCHAR*
		#define _M_TEXT(x)		x
		#define M_STD_STRING	std::string
		#define M_LOADSTRING	::LoadStringA
		#define M_STD_OSTRINGSTREAM	    std::ostringstream
		#define M_STD_ISTRINGSTREAM	    std::istringstream
		#define M_STD_CERR				std::cerr
		#define M_STD_OSTREAM			std::ostream
		#define M_STD_ISTREAM			std::istream
		#define M_STD_COUT				std::cout
	#else
		#define MCHAR           mwchar_t
		#define LPCMSTR         const MCHAR*
		#define LPMSTR			MCHAR*
		#define _M_TEXT(x)		L ## x
		#define M_STD_STRING	std::wstring
		#define M_LOADSTRING	::LoadStringW
		#define M_STD_OSTRINGSTREAM	    std::wostringstream
		#define M_STD_ISTRINGSTREAM	    std::wistringstream
		#define M_STD_CERR				std::wcerr
		#define M_STD_OSTREAM			std::wostream
		#define M_STD_ISTREAM			std::wistream
		#define M_STD_COUT				std::wcout
	#endif
#else
	#define MCHAR           char
	#define LPCMSTR         const MCHAR*
	#define LPMSTR			MCHAR*
	#define _M_TEXT(x)		x
	#define M_STD_STRING	std::string
	#define M_LOADSTRING	::LoadStringA
	#define M_STD_OSTRINGSTREAM	    std::ostringstream
	#define M_STD_ISTRINGSTREAM	    std::istringstream
	#define M_STD_CERR				std::cerr
	#define M_STD_OSTREAM			std::ostream
	#define M_STD_ISTREAM			std::istream
	#define M_STD_COUT				std::cout
#endif
#define _M(x)           _M_TEXT(x)

// These macros work in conjunction with the VC conversion macros (A2W, A2T, W2T, etc)
// See the MSDN documentation for details.  If you get compile errors such as
// 
// error C2065: '_lpa' : undeclared identifier
// error C2065: '_convert' : undeclared identifier
// error C2065: '_acp' : undeclared identifier
// 
// when trying to use any of these macros, it's probably because there's a
// 
// USES_CONVERSION
// 
// missing in your function.

// M2A: Build-specific to ASCII
// A2M: ASCII to build-specific
// M2W: Build-specific to wide char
// W2M: Wide char to build-specific
// M2T: Max definition of TCHAR to current project definition of TCHAR
// T2M: current project definition of TCHAR to Max definition of TCHAR 

// NOTE: converted strings are stored in a stack-based buffer. so don't say
// something like:
//   return A2M(msg.data());

#ifdef _UNICODE
	// If the module uses Unicode and needs to translate to
	// an MBCS Max, define _UNICODE_MODULE_FOR_MBCS_MAX
	#ifdef _UNICODE_MODULE_FOR_MBCS_MAX
		#define M2A(p)          (p)
		#define A2M(p)          (p)
		#define M2W(p)          A2W(p)
		#define W2M(p)          W2A(p)
		#define M2T(p)          A2T(p)
		#define T2M(p)          T2A(p)
	#else
		#define M2A(p)          W2A(p)
		#define A2M(p)          A2W(p)
		#define M2W(p)          (p)
		#define W2M(p)          (p)
		#define M2T(p)          (p)
		#define T2M(p)          (p)
	#endif
#else
	#define M2A(p)          (p)
	#define A2M(p)          (p)
	#define M2W(p)          A2W(p)
	#define W2M(p)          W2A(p)
	#define M2T(p)          A2T(p)
	#define T2M(p)          T2A(p)
#endif

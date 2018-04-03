////////////////////////////////////////////////////////////////////////////////
//	assert
//	
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cmath>
using namespace std;

#include "scl/type.h"

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include "scl/backtrace.h"
#endif

#ifdef SCL_WIN
#include <excpt.h>
#endif

namespace scl { 

void assert_write(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expresion);

void assert_writef(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression,
	const char* const format,
	...);

//undef system assert
#ifdef assert
#undef assert
#endif

//////////////////////////////////////////////////////////////////
//	win32 
//////////////////////////////////////////////////////////////////
#ifdef SCL_WIN32

#ifdef _DEBUG
	#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); __asm { int 3 }; } } while(0)
	//assert format
	#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, __VA_ARGS__); __asm { int 3 }; } } while(0)
	#define SCL_ASSERT_TRY 
	#define SCL_ASSERT_CATCH while(0)
#else
	#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); throw 1; } } while(0)
	#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, __VA_ARGS__); throw 1; } } while(0)
	#define SCL_ASSERT_TRY __try 
	#define SCL_ASSERT_CATCH __except(scl::except_handler(GetExceptionInformation())) 
#endif  // _DEBUG

#endif // SCL_WIN32


//////////////////////////////////////////////////////////////////
//	win64
//////////////////////////////////////////////////////////////////
#ifdef SCL_WIN64
#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); throw 1; } } while(0)

//assert format
#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, __VA_ARGS__); throw 1; } } while(0)

#define SCL_ASSERT_TRY __try 
#define SCL_ASSERT_CATCH __except(scl::except_handler(GetExceptionInformation())) 

#endif //SCL_WIN64


//////////////////////////////////////////////////////////////////
//linux assert
//////////////////////////////////////////////////////////////////
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)

#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); scl::print_stack_to_file(); throw 1; } } while(0)
//#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); __asm__("int3"); } } while(0)

//assert format
#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, ##__VA_ARGS__); scl::print_stack_to_file(); throw 1; } } while(0)

#define SCL_ASSERT_TRY try 
#define SCL_ASSERT_CATCH catch(...) 
#endif // SCL_LINUX

} //namespace scl

#include "scl/assert.h"
#include "scl/log.h"

#include <stdio.h>
#include <stdarg.h>

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <string.h>
#endif

#ifdef SCL_WIN
#include <windows.h>
#define snprintf sprintf_s
#endif 

namespace scl {

void assert_write(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression)
{
	const int MAX_ASSERT_STRING_LENGTH = 1024 * 16;
	char logMessage[MAX_ASSERT_STRING_LENGTH] = { 0 };
	snprintf(
		logMessage, 
		MAX_ASSERT_STRING_LENGTH - 1, 
		"<file: %s>\n<function: %s>\n<line: %d>\n<expresion: %s>\n", 
		fileName, 
		functionName, 
		lineNumber, 
		expression);
	logMessage[MAX_ASSERT_STRING_LENGTH - 1] = 0;

#ifdef SCL_WIN
	OutputDebugStringA(logMessage);
#endif

	printf("%s", logMessage);

	scl::urgency_log(logMessage);
}


void assert_writef(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression,
	const char* const format,
	...)
{
	const int MAX_ASSERT_STRING_LENGTH = 1024 * 16;
	char formatStringBuffer[1024 * 4] = { 0 };
	if (NULL != format)
	{
		va_list args;
		va_start(args, format);

#ifdef SCL_WIN
		vsprintf_s(formatStringBuffer, sizeof(formatStringBuffer), format, args);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
		vsnprintf(formatStringBuffer, sizeof(formatStringBuffer), format, args);
#endif

		va_end(args);
	}
	char logMessage[MAX_ASSERT_STRING_LENGTH] = { 0 };
	snprintf(
		logMessage, 
		MAX_ASSERT_STRING_LENGTH - 1, 
		"<file: %s>\n<function: %s>\n<line: %d>\n<expresion: %s>\n<message: %s>\n", 
		fileName, 
		functionName, 
		lineNumber, 
		expression,
		formatStringBuffer);
	logMessage[MAX_ASSERT_STRING_LENGTH - 1] = 0;

#ifdef SCL_WIN
	OutputDebugStringA(logMessage);
#endif

	printf("%s", logMessage);

	scl::urgency_log(logMessage);
}

} //namespace scl

#include "Function.h"
#include <iostream>

#ifdef _WIN32
#include <stdarg.h>
#include <windows.h>
#endif

int lord_sprintf(char* buff, const int bufflen, const char* const format, ...)
{
	va_list arg;
	va_start(arg, format);
	int r = 0;
#ifdef _WIN32
	r = vsnprintf(buff, bufflen, format, arg);
#else
	r = vsnprintf(buff, bufflen, format, arg);
#endif
	va_end(arg);

	buff[bufflen - 1] = 0;
	return r;
}

int lord_sscanf(const char* buff, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	int r = 0;

	//#ifdef _WIN32
	//#pragma warning(push)
	//#pragma warning(disable:4996)
	//#endif

	r = vsscanf(buff, format, arg);

	//#ifdef _WIN32
	//#pragma warning(pop)
	//#endif

	return r;
}
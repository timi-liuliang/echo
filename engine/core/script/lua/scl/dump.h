#pragma once

#ifdef _WIN32 
//#include <Windows.h>
struct _CONTEXT;

namespace scl {

void init_dump(const wchar_t* szAppName, const wchar_t* dumpPath = L"", const wchar_t* version = L"");

void dump_print_stack(_CONTEXT *context, const wchar_t* const filename, const char* const datetime = 0);
} //namespace scl

//LONG WINAPI _toplevel_exception_filter(_EXCEPTION_POINTERS *pExceptionInfo);

#endif

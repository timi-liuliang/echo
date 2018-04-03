#ifdef _WIN32 

#include "dump.h"

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <dbghelp.h>

LONG WINAPI		_toplevel_exception_filter		(_EXCEPTION_POINTERS *pExceptionInfo);
void			_dump_purecall_handler			();
void			_dump_invalid_parameter_handler	(const wchar_t* s1, const wchar_t* s2, const wchar_t* s3, unsigned int, uintptr_t);

namespace scl {

static const int MAX_APPNAME = 128;
wchar_t dump_appname[MAX_APPNAME]	= { 0 };
wchar_t dump_path	[MAX_PATH]		= { 0 };

void make_datetime_string(wchar_t* out, SYSTEMTIME* pst = NULL)
{
	//根据时间生成文件名
	SYSTEMTIME st;
	if (NULL == pst)
	{
		::GetLocalTime(&st);
		pst = &st;
	}
	swprintf_s(out, 64, L"%d%.2d%.2d_%.2d%.2d%.2d_%.4d", pst->wYear, pst->wMonth, pst->wDay, pst->wHour, pst->wMinute, pst->wSecond, pst->wMilliseconds);
}

void init_dump(const wchar_t* szAppName, const wchar_t* dumpPath, const wchar_t* version)
{
	wcscpy_s(dump_appname, MAX_APPNAME, szAppName);
	wcscat_s(dump_appname, MAX_APPNAME, version); 
	wcscpy_s(dump_path, MAX_PATH, dumpPath);
	size_t len = wcslen(dump_path);
	if (dump_path[0] != 0 && dump_path[len - 1] != L'\\' && dump_path[len - 1] != L'/')
		wcscat_s(dump_path, MAX_PATH, L"/");
	::SetUnhandledExceptionFilter	(_toplevel_exception_filter);
	_set_purecall_handler			(_dump_purecall_handler);
	_set_invalid_parameter_handler	(_dump_invalid_parameter_handler);
}

class DbgHelpFunctions
{
public:
	DbgHelpFunctions();
	~DbgHelpFunctions();

	bool loadSym();
	bool loadMiniDump();

	typedef BOOL	(WINAPI *FuncType_MiniDumpWriteDump			)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE dumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION	exceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION userStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION callbackParam);
	typedef BOOL	(WINAPI *FuncType_SymGetSymFromAddr64		)(HANDLE hProcess, DWORD64 qwAddr, PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64  Symbol);
	typedef DWORD	(WINAPI *FuncType_SymGetOptions				)();
	typedef BOOL	(WINAPI *FuncType_SymInitializeW			)(HANDLE hProcess, PCWSTR UserSearchPath, BOOL fInvadeProcess);
	typedef BOOL	(WINAPI *FuncType_SymFromAddr				)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol);
	typedef BOOL	(WINAPI *FuncType_StackWalk64				)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
	typedef BOOL	(WINAPI *FuncType_SymCleanup				)(HANDLE hProcess);
	typedef PVOID	(WINAPI *FuncType_SymFunctionTableAccess64	)(HANDLE hProcess, DWORD64 AddrBase);
	typedef DWORD64	(WINAPI *FuncType_SymGetModuleBase64		)(HANDLE hProcess, DWORD64 qwAddr);
	typedef BOOL	(WINAPI *FuncType_SymGetLineFromAddr64		)(HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64);
	typedef DWORD	(WINAPI *FuncType_SymSetOptions				)(DWORD SymOptions);

	FuncType_SymGetSymFromAddr64		_SymGetSymFromAddr64;		
	FuncType_SymInitializeW				_SymInitializeW;			
	FuncType_SymGetOptions				_SymGetOptions;				
	FuncType_SymFromAddr				_SymFromAddr;				
	FuncType_StackWalk64				_StackWalk64;				
	FuncType_SymCleanup					_SymCleanup;				
	FuncType_SymFunctionTableAccess64	_SymFunctionTableAccess64;
	FuncType_SymGetModuleBase64			_SymGetModuleBase64;		
	FuncType_MiniDumpWriteDump			_MiniDumpWriteDump;			
	FuncType_SymGetLineFromAddr64		_SymGetLineFromAddr64;
	FuncType_SymSetOptions				_SymSetOptions;

private:
	HMODULE dll;
};


DbgHelpFunctions::DbgHelpFunctions() :
	_SymGetSymFromAddr64		(NULL),
	_SymInitializeW				(NULL),
	_SymGetOptions				(NULL),
	_SymFromAddr				(NULL),
	_StackWalk64				(NULL),
	_SymCleanup					(NULL),
	_SymFunctionTableAccess64	(NULL),
	_SymGetModuleBase64			(NULL),
	_MiniDumpWriteDump			(NULL),
	_SymGetLineFromAddr64		(NULL),
	_SymSetOptions				(NULL),
	dll							(NULL)
{

}

DbgHelpFunctions::~DbgHelpFunctions()
{
	if (NULL != dll)
	{
		::FreeLibrary(dll);
		dll = NULL;
	}
}

bool DbgHelpFunctions::loadMiniDump()
{
	if (NULL != dll)
		return false;

	dll = ::LoadLibraryW(L"dbghelp.dll");
	if (NULL == dll)
	{
		::MessageBoxW(NULL, L"dbghelp.dll not found!", NULL, MB_OK);
		return false;
	}
	_MiniDumpWriteDump = (FuncType_MiniDumpWriteDump)(::GetProcAddress(dll, "MiniDumpWriteDump"));
	if (NULL == _MiniDumpWriteDump)
		return false;

	return true;
}

bool DbgHelpFunctions::loadSym()
{
	if (NULL != dll)
		return false;
	dll = ::LoadLibraryW(L"dbghelp.dll");
	if (NULL == dll)
	{
		::MessageBoxW(NULL, L"dbghelp.dll not found!", NULL, MB_OK);
		return false;
	}
	_SymGetSymFromAddr64		= (FuncType_SymGetSymFromAddr64			)(::GetProcAddress(dll, "SymGetSymFromAddr64"));
	_SymInitializeW				= (FuncType_SymInitializeW				)(::GetProcAddress(dll, "SymInitializeW"));
	_SymGetOptions				= (FuncType_SymGetOptions				)(::GetProcAddress(dll, "SymGetOptions"));
	_SymFromAddr				= (FuncType_SymFromAddr					)(::GetProcAddress(dll, "SymFromAddr"));
	_StackWalk64				= (FuncType_StackWalk64					)(::GetProcAddress(dll, "StackWalk64"));
	_SymCleanup					= (FuncType_SymCleanup					)(::GetProcAddress(dll, "SymCleanup"));
	_SymFunctionTableAccess64	= (FuncType_SymFunctionTableAccess64	)(::GetProcAddress(dll, "SymFunctionTableAccess64"));
	_SymGetModuleBase64			= (FuncType_SymGetModuleBase64			)(::GetProcAddress(dll, "SymGetModuleBase64"));
	_SymGetLineFromAddr64		= (FuncType_SymGetLineFromAddr64		)(::GetProcAddress(dll, "SymGetLineFromAddr64"));
	_SymSetOptions				= (FuncType_SymSetOptions				)(::GetProcAddress(dll, "SymSetOptions"));

	if (NULL == _SymGetSymFromAddr64
		|| NULL == _SymInitializeW
		|| NULL == _SymGetOptions
		|| NULL == _SymFromAddr
		|| NULL == _StackWalk64
		|| NULL == _SymCleanup
		|| NULL == _SymFunctionTableAccess64
		|| NULL == _SymGetModuleBase64
		|| NULL == _SymSetOptions
		|| NULL == _SymGetLineFromAddr64)
		return false;

	return true;
}


void dump_print_stack(CONTEXT *context, FILE* f, const char* const datetime = NULL)
{
	DbgHelpFunctions funcs;
	funcs.loadSym();

	DWORD opt = funcs._SymSetOptions(funcs._SymGetOptions() | SYMOPT_LOAD_LINES);

	if(!funcs._SymInitializeW(GetCurrentProcess(), NULL, TRUE))
		fprintf(f, "Init dbghelp failed.\n");

	fprintf(f, "================[exception]====================\n");
	if (NULL != datetime)
		fprintf(f, "%s\n", datetime);

	STACKFRAME64 sf;
	memset(&sf, 0, sizeof(STACKFRAME64));

#ifdef _WIN64
	sf.AddrPC.Offset	= context->Rip;
	sf.AddrStack.Offset = context->Rsp;
	sf.AddrFrame.Offset = context->Rbp;
#else
	sf.AddrPC.Offset = context->Eip;
	sf.AddrStack.Offset = context->Esp;
	sf.AddrFrame.Offset = context->Ebp;
#endif

	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode	= AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;
	

	DWORD machineType	= IMAGE_FILE_MACHINE_I386;
	HANDLE hProcess		= GetCurrentProcess();
	HANDLE hThread		= GetCurrentThread();

	while (1)	
	{
		if(!funcs._StackWalk64(machineType, hProcess, hThread, &sf, context, 0, funcs._SymFunctionTableAccess64, funcs._SymGetModuleBase64, 0))
			break;

		if(sf.AddrFrame.Offset == 0)
			break;

		BYTE symbolBuffer[sizeof(SYMBOL_INFO) + 1024]		= { 0 };
		PSYMBOL_INFO pSymbol								= (PSYMBOL_INFO)symbolBuffer;
		pSymbol->SizeOfStruct								= sizeof( symbolBuffer );
		pSymbol->MaxNameLen									= 1024;
		DWORD64 symDisplacement								= 0;

		//打印caller function在源代码中的文件和行号
		DWORD64  dummy										= 0;
		HANDLE process										= ::GetCurrentProcess();

		//打印caller function的name
		char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255]	= { 0 };
		IMAGEHLP_SYMBOL64* symbol							= (IMAGEHLP_SYMBOL64*)symbol_buffer;
		symbol->SizeOfStruct									= sizeof(IMAGEHLP_SYMBOL64) + 255;
		symbol->MaxNameLength								= 254;

		if (!funcs._SymGetSymFromAddr64(process, sf.AddrPC.Offset, &dummy, symbol)) 
			fprintf(f, "!!!error!!!SymGetSymFromAddr errno[%d]", ::GetLastError());

		IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
		DWORD dwLineDisplacement;

		if(!funcs._SymGetLineFromAddr64( hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo ) )
			fprintf(f, "!!!error!!!SymGetLineFromAddr errno[%d]", ::GetLastError());

		//文件名
		if (NULL != lineInfo.FileName && 0 != lineInfo.FileName[0] )
			fprintf(f, "%s", lineInfo.FileName);

		//行号
		fprintf(f, " (%d) ", lineInfo.LineNumber);

		//函数名
		if (NULL != symbol->Name && 0 != symbol->Name[0])
			fprintf(f, ": %s", symbol->Name);

		fprintf(f, "\n");
	}

	if(!funcs._SymCleanup(GetCurrentProcess()))
		fprintf(f, "Cleanup dbghelp failed.\n");
}

void dump_print_stack(CONTEXT *context, const wchar_t* const filename, const char* const datetime)
{
	FILE* f = NULL;
	_wfopen_s(&f, filename, L"a+b");
	dump_print_stack(context, f, datetime);
	fclose(f);
}

int dump_generate(_EXCEPTION_POINTERS *pExceptionInfo, const wchar_t* const filename)
{
	DbgHelpFunctions funcs;
	if (!funcs.loadMiniDump())
	{
		::MessageBoxW(NULL, L"load dbghelp.dll failed!", NULL, MB_OK);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	HANDLE hFile = ::CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		::MessageBoxW(NULL, L"can't create dump file!", NULL, MB_OK);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId				= ::GetCurrentThreadId();
	ExInfo.ExceptionPointers	= pExceptionInfo;
	ExInfo.ClientPointers		= NULL;

	// write the dump
	if (!funcs._MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL))
	{
		::MessageBoxW(NULL, L"write dump failed!", NULL, MB_OK);
	}

	::CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

} //namespace scl


LONG WINAPI _toplevel_exception_filter(_EXCEPTION_POINTERS *pExceptionInfo)
{
	wchar_t datetime[64] = { 0 };
	scl::make_datetime_string(datetime);

	//stack file
	wchar_t stack_path[MAX_PATH] = { 0 }; 
	swprintf_s(stack_path, MAX_PATH, L"%s%s_%s_stack.txt", scl::dump_path, scl::dump_appname, datetime);
	CONTEXT ctx;
	memcpy(&ctx, pExceptionInfo->ContextRecord, sizeof(ctx));
	scl::dump_print_stack(&ctx, stack_path);

	//dump file
	wchar_t _dump_path[MAX_PATH] = { 0 };
	swprintf_s(_dump_path, MAX_PATH, L"%s%s_%s.dmp", scl::dump_path, scl::dump_appname, datetime);
	LONG r = EXCEPTION_EXECUTE_HANDLER;
	r = scl::dump_generate(pExceptionInfo, _dump_path);

	return r;
}

void _dump_purecall_handler()
{
	throw 1;
	exit(0);
}

void _dump_invalid_parameter_handler(const wchar_t* s1, const wchar_t* s2, const wchar_t* s3, unsigned int, uintptr_t)
{
	throw 1;
	exit(0);
}


#endif


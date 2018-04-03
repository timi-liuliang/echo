#include "backtrace.h"

#include "scl/time.h"
#include "scl/string.h"
#include "scl/dump.h"

#include <time.h>

#ifdef SCL_WIN
#include <Windows.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <cxxabi.h>
#include <stdlib.h> // free(void*)
#include <string.h> // strlen
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#define sprintf_s snprintf
#endif

#ifdef SCL_ANDROID
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#define sprintf_s snprintf
#endif


namespace scl {

bool _check_frequency();

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
void print_stack_to_file()
{
	__inner_error_log_mutex.lock();
	
	//if (!_check_frequency())
	//	break;
	
	FILE* f = fopen("error.log", "ab+");
	if (NULL == f)
		return;

	const int MAX_STACK_TEXT = 1024 * 8;
	void* stack[62] = { NULL };
	int c = scl::backtrace(stack, 62);
	char s[MAX_STACK_TEXT] = { 0 };
	scl::print_stack(stack, c, s, MAX_STACK_TEXT);

	scl::time t;
	t.now();
	scl::string32 strtime;
	t.to_string(strtime);
	fprintf(f, "================[assert]====================\n");
	fprintf(f, "%s\n", strtime.c_str());
	fprintf(f, "%s\n", s);

	fclose(f);
	
	__inner_error_log_mutex.unlock();
}
#endif

#ifdef SCL_WIN 
DWORD except_handler(LPEXCEPTION_POINTERS lpEP)
{
	// init dbghelp.dll
	if(!SymInitialize(GetCurrentProcess(), NULL, TRUE))
		printf("Init dbghelp failed.\n");

	scl::dump_print_stack(lpEP->ContextRecord, L"error.log");

	if(!SymCleanup(GetCurrentProcess()))
		printf("Cleanup dbghelp failed.\n");

	////在最前端弹出一个messagebox
	//HINSTANCE hInstance = GetModuleHandle(0);
	////注册窗口类
	//const TCHAR szWindowClass[] = L"MainWindowClass";			// 主窗口类名
	//WNDCLASSEX wcex;
	//wcex.cbSize			= sizeof(WNDCLASSEX);
	//wcex.style			= CS_HREDRAW | CS_VREDRAW;
	//wcex.lpfnWndProc	= reinterpret_cast<WNDPROC>(&DefWindowProc);
	//wcex.cbClsExtra		= 0;
	//wcex.cbWndExtra		= 0;
	//wcex.hInstance		= hInstance;
	////wcex.hIcon			= LoadIcon(hInstance, szIconName);
	////wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	//wcex.lpszMenuName	= NULL;
	//wcex.lpszClassName	= szWindowClass;
	//wcex.hIconSm		= 0;
	//RegisterClassEx(&wcex);

	//标题栏文本
	//const TCHAR szTitle[]		= _T("main");
	//创建窗口
	//HWND m_windowHandle = ::CreateWindow(		
	//	szWindowClass,			//  LPCTSTR lpClassName,
	//	L"title",				//  LPCTSTR lpWindowName,
	//	WS_OVERLAPPEDWINDOW,	//  DWORD dwStyle,
	//	CW_USEDEFAULT,			//  int x,
	//	400,		//  int y,
	//	200,	//  int nWidth,
	//	200,	//  int nHeight,
	//	NULL,					//  HWND hWndParent,
	//	NULL,					//  HMENU hMenu,
	//	hInstance,				//  HINSTANCE hInstance,
	//	NULL);					//  LPVOID lpParam

	//if (!m_windowHandle)
	//	return false;

	//ShowWindow(static_cast<HWND>(m_windowHandle), SW_SHOW);
	//UpdateWindow(static_cast<HWND>(m_windowHandle));

	wchar_t title[128] = { 0 };
	GetConsoleTitleW(title, 128);
	HWND handle = FindWindowW(NULL, title);
	SwitchToThisWindow(handle, TRUE);
#ifdef SCL_BACKTRACE_WARN
	::MessageBoxW(handle, L"Exception! Call Stack has been write to error.log", L"WoW!", MB_OK);
#endif

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

bool _check_frequency()
{
	static uint64	_frequencyTimer = 0;
	static int		_frequencyCount = 0;

	uint64 now = ::time(NULL);
	if (now - _frequencyTimer > 60)	//每隔一分钟清空一次计数器
	{
		_frequencyTimer = now;
		_frequencyCount = 0;
	}

	if (_frequencyCount > 100)		//计数器大于100就不再打印日志
		return false;

	++_frequencyCount;

	return true;
}


#ifdef SCL_WIN
int _print_addr(void* addr, char* out, const int outlen, const int format)
{
	if (NULL == addr)
		return -1;

	static bool init = false;
	HANDLE hProcess = GetCurrentProcess();
	if (!init)
	{
		SymInitialize(hProcess, NULL, TRUE);
		init = true;
	}

	//输出信息长度
	int slen = 0;

	//获取函数信息
	if (format & scl::FUNCTION_NAME)
	{
		BYTE			symbolBuffer[sizeof(SYMBOL_INFO)+MAX_SYM_NAME * sizeof(TCHAR)] = { 0 };
		PSYMBOL_INFO	pSymbol = (PSYMBOL_INFO)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;
		if (!SymFromAddr(hProcess, (DWORD64)addr, NULL, pSymbol))
			slen += sprintf_s(out + slen, outlen > slen ? outlen - slen : 0, "<no function name>\t");
		else
		{
			if (NULL != pSymbol->Name && 0 != pSymbol->Name[0])
				slen += sprintf_s(out + slen, outlen > slen ? outlen - slen : 0, "%s\t", pSymbol->Name);
		}
	}

	//获取文件和行号
	if (format & scl::FILE_NAME)
	{
		IMAGEHLP_LINE64 line;
		line.SizeOfStruct = sizeof(line);
		SymSetOptions(SYMOPT_LOAD_LINES);
		DWORD dis = 0;
		if (!SymGetLineFromAddr64(hProcess, (DWORD64)addr, &dis, &line))
			slen += sprintf_s(out + slen, outlen > slen ? outlen - slen : 0, "<no file name>");
		else
		{
			if (NULL != line.FileName && 0 != line.FileName[0])
				slen += sprintf_s(out + slen, outlen > slen ? outlen - slen : 0, "%s", line.FileName);
			if (format & scl::LINE_NUM)
				slen += sprintf_s(out + slen, outlen > slen ? outlen - slen : 0, " (%d) ", line.LineNumber);
		}
	}

	return slen;
}
#endif

#ifdef SCL_ANDROID
struct BacktraceState
{
    void** current;
    void** end;
};

static _Unwind_Reason_Code _callback_android_unwind(struct _Unwind_Context* context, void* arg)
{
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end)
            return _URC_END_OF_STACK;
        else
            *state->current++ = reinterpret_cast<void*>(pc);
    }
    return _URC_NO_REASON;
}


size_t _android_backtrace(void** buffer, size_t max)
{
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(_callback_android_unwind, &state);

    return state.current - buffer;
}

int _print_addr(void* addr, char* output, const int outputlen, const int format)
{
	const char* symbol = "";
	int slen = 0;

	Dl_info info;
	if (dladdr(addr, &info) && info.dli_sname) 
	{
		symbol = info.dli_sname;
	}

	int		status			= 0;
	char	demangled[512]	= { 0 };
	size_t	demangled_len	= 512;
	char* ret = abi::__cxa_demangle(symbol, demangled, &demangled_len, &status);
	const char* function_name = NULL;
	if (status == 0)
		function_name = ret;
	else
		function_name = symbol;

	//slen += snprintf( output + slen, outputlen > slen ? outputlen - slen : 0, "addr[%x] funcname[%s] dl.fname[%s] dl.fbase[%x] dl.sname[%s] dl.saddr[%x]", reinterpret_cast<unsigned int>(addr), function_name, info.dli_fname, reinterpret_cast<unsigned int>(info.dli_fbase), info.dli_sname, reinterpret_cast<unsigned int>(info.dli_saddr));
	if (format & scl::FUNCTION_NAME)
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "%s ", function_name);
}

#endif

#if defined (SCL_LINUX)
int _demangle(char* const s, const int len, char* output, const int outputlen, const int format)
{
	char*	mangled	= NULL;
	char*	offset	= NULL;
	char*	end		= NULL;
	int		slen	= 0;
	
	for (char* p = s; *p; ++p)
	{
		if (*p == '(') mangled = p;
		else if (*p == '+') offset = p;
		else if (mangled && *p ==')') { end = p; break; }
	}
	if (NULL == mangled || NULL == offset || NULL == end || mangled >= offset)
	{
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "%s\n", s);
		break;
	}
	
	*mangled++		= 0;
	*offset++		= 0;
	*end			= 0;
	
	int status = 0;
	char demangled[512] = { 0 };
	size_t demangled_len = 512;
	char* ret = abi::__cxa_demangle(mangled, demangled, &demangled_len, &status);
	if (status == 0)
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "%s : %s+%s\n", symbols[j], ret, offset);
	else
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "%s : %s+%s\n", symbols[j], mangled, offset);
	
	return slen;
}
}
#endif


#if defined (SCL_APPLE)
int _demangle(char* const s, const int len, char* output, const int outputlen, const int format)
{
	//words are seperated by space.
	//parse from:
	//		"0   memtest		0x0000000100d912c7 _Z5test3PKc + 199"
	//to
	//		words[6] = { "0", "memtest", "0x00d912c7", "_Z5test3PKc", "+", "199" };
	
	int nword = 0;
	char* words[16] = { 0 };
	bool seperate = true;
	int slen = 0;
	for (int i = 0; i < len; ++i)
	{
		if (s[i] == ' ')
		{
			if (seperate)
				s[i] = 0;
			continue;
		}
		if (s[i] == '+' && i > 0)
		{
			s[i - 1] = 0;
			seperate = true;
		}
		if (i > 0 && s[i - 1] != 0)
			continue;
		
		words[nword++] = s + i;
		if (nword == 4)
			seperate = false;
	}
	if (nword < 6)
	{
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "error word count = %d\n", nword);
		return slen;
	}
	
	size_t  demangled_len     = 512;
	char    demangled[512]    = { 0 };
	int     status            = 0;
	char* ret = abi::__cxa_demangle(words[3], demangled, &demangled_len, &status);
	char* function_name = NULL;
	if (status == 0)
		function_name = ret;
	else
		function_name = words[3];

	
	if (format & scl::ADDRESS)
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "%s :", words[2]);
	if (format & scl::FUNCTION_NAME)
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "%s ", function_name);
	if (format & scl::FUNCTION_OFFSET)
		slen += snprintf(output + slen, outputlen > slen ? outputlen - slen : 0, "+ %s ", words[5]);

	
	return slen;
}
#endif

int backtrace(void** stack, int maxframe, int skip)
{
	if (maxframe > SCL_MAX_FRAME)
		maxframe = SCL_MAX_FRAME;

	int r = 0;
#ifdef SCL_WIN
	r = CaptureStackBackTrace(0, maxframe, stack, NULL);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
	r = ::backtrace(stack, maxframe);
#endif

#if defined(SCL_ANDROID)
	r = _android_backtrace(stack, maxframe);
#endif

	if (skip > 0)
	{
		for (int i = 0; i < r; ++i)
		{
			if (i + skip < maxframe)
				stack[i] = stack[i + skip];
		}
		r -= skip;
	}

	return r;
}

int print_stack(void* const* stack, const int framecount, char* output, const int outputlen, const int format, const char* const line_seprator)
{
	int		len = 0;

#if defined(SCL_WIN) || defined(SCL_ANDROID)
	for (int i = 0; i < framecount && i < SCL_MAX_FRAME; ++i) 
	{
		if (format & scl::FRAME_INDEX)
			len += sprintf_s(output + len, outputlen > len ? outputlen - len : 0, "[%d] ", i);
#undef ADDRESS
		if (format & scl::ADDRESS)
			len += sprintf_s(output + len, outputlen > len ? outputlen - len : 0, "0x%08llX\t: ", reinterpret_cast<unsigned long long>(stack[i]));

		len += _print_addr(stack[i], output + len, outputlen - len, format);
		len += sprintf_s(output + len, outputlen > len ? outputlen - len : 0, "%s", line_seprator);
	}
#endif

#if defined (SCL_APPLE) || defined(SCL_LINUX) 
	char** symbols = backtrace_symbols(stack, framecount);
	if (symbols == NULL)
		perror("backtrace_symbols");
	
	for (int i = 0; i < framecount; i++)
	{
		len += _demangle(symbols[i], static_cast<int>(strlen(symbols[i])), output + len, outputlen - len, format);
		len += snprintf(output + len, outputlen > len ? outputlen - len : 0, "%s", line_seprator);
	}
	
	::free(symbols);
#endif

	return len;
}

} //namespace scl 



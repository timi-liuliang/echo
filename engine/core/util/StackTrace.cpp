#include "StackTrace.h"
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

namespace Echo
{
    // remember call trace stack
    i32 StackTrace(void **callstack, i32 maxStackDepth)
    {
        return backtrace(callstack, maxStackDepth);
    }
    
    // This function produces a stack backtrace with demangled function & method names.
    std::string StackTraceDesc( void **callstack, int maxStackDepth, int stackDepth, int skip)
    {
        char buf[1024];
        char **symbols = backtrace_symbols(callstack, stackDepth);
        std::ostringstream trace_buf;
        for (int i = skip; i < stackDepth; i++)
        {
            Dl_info info;
            if (dladdr(callstack[i], &info) && info.dli_sname)
            {
                char *demangled = NULL;
                int status = -1;
                if (info.dli_sname[0] == '_')
                    demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
                
                snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n", i-skip+1, int(2 + sizeof(void*) * 2), callstack[i], status == 0 ? demangled : info.dli_sname == 0 ? symbols[i] : info.dli_sname, (char *)callstack[i] - (char *)info.dli_saddr);
                free(demangled);
            }
            else
            {
                snprintf(buf, sizeof(buf), "%-3d %*p %s\n", i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
            }
            trace_buf << buf;
        }
        
        free(symbols);
        if (stackDepth == maxStackDepth)
            trace_buf << "[truncated]\n";
        
        return trace_buf.str();
    }
}

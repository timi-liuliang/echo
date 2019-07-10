#pragma once

#include <string>
#include "engine/core/base/echo_def.h"

namespace Echo
{
    // remember call trace stack
    i32 StackTrace(void **callstack, i32 maxStackDepth);
    
    // get trace info
    std::string StackTraceDesc(void **callstack, int maxStackDepth, int stackDepth, int skip = 1);
}

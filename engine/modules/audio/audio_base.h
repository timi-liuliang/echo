#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_PLATFORM_MAC
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <al.h>
    #include <alc.h>
#endif

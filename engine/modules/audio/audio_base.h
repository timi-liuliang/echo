#pragma once

#include "engine/core/base/echo_def.h"

#if defined(ECHO_PLATFORM_MAC)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#elif  defined(ECHO_PLATFORM_ANDROID)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <al.h>
    #include <alc.h>
#endif

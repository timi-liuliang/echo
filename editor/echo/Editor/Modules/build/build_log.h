#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
    class BuildLog
    {
    public:
        // output
        virtual void log(const char* msg) {}
    };
}

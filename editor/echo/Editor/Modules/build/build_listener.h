#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
    class BuildListener
    {
    public:
        // output
        virtual void log(const char* msg) {}

        // on begin|end
        virtual void onBegin() {}
        virtual void onEnd() {}

        // on exec cmd
        virtual void onExecCmd(const char* cmd, const char* workingDir) {}
    };
}

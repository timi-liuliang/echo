#pragma once

#include <engine/core/log/Log.h>

namespace Echo
{
    class GameLog : public Echo::LogOutput
    {
    public:
        GameLog(const Echo::String& name);

        /** output message */
        virtual void logMessage(Level level, const Echo::String &msg);
    };
}
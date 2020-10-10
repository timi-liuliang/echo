#include "Log.h"
#include <engine/core/util/StringUtil.h>

namespace Echo
{
    GameLog::GameLog(const Echo::String& name)
        : LogOutput(name)
    {
        
    }

    void GameLog::logMessage(Level level, const Echo::String &msg)
    {
        printf(msg.c_str());
    }
}

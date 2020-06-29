#include "Log.h"
#include <engine/core/util/StringUtil.h>
#include <android/log.h>

namespace Echo
{
    GameLog::GameLog(const Echo::String& name)
            : LogOutput(name)
    {

    }

    void GameLog::logMessage(Level level, const Echo::String &msg)
    {
        switch(level)
        {
            case LL_DEBUG:   __android_log_print(ANDROID_LOG_DEBUG, "Echo", "%s", msg.c_str()); break;
            case LL_INFO:    __android_log_print(ANDROID_LOG_INFO, "Echo", "%s", msg.c_str()); break;
            case LL_WARNING: __android_log_print(ANDROID_LOG_WARN, "Echo", "%s", msg.c_str()); break;
            case LL_ERROR:   __android_log_print(ANDROID_LOG_ERROR, "Echo", "%s", msg.c_str()); break;
            case LL_FATAL:   __android_log_print(ANDROID_LOG_FATAL, "Echo", "%s", msg.c_str()); break;
            default:         __android_log_print(ANDROID_LOG_UNKNOWN, "Echo", "%s", msg.c_str()); break;
        }
    }
}
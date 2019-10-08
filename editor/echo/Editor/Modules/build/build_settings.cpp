#include "build_settings.h"

namespace Echo
{
    void BuildSettings::bindMethods()
    {
        
    }

    void BuildSettings::log(const char* formats, ...)
    {
        char szBuffer[4096];
        int bufferLength = sizeof(szBuffer);
        va_list args;
        va_start(args, formats);
        vsnprintf(szBuffer, bufferLength, formats, args);
        va_end(args);

        szBuffer[bufferLength - 1] = 0;

        m_listener->log(szBuffer);
    }
}

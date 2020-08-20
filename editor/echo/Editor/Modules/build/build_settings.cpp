#include "build_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/archive/FilePackage.h>

namespace Echo
{
    void BuildSettings::bindMethods()
    {
        
    }

    void BuildSettings::packageRes(const String& rootFolder)
    {
        StringArray subFolers;
        PathUtil::EnumFilesInDir(subFolers, rootFolder, true, false, true);
        for (const String& folder : subFolers)
        {
            if (!PathUtil::IsFile(folder))
            {
                FilePackage::compressFolder(folder.c_str());
                PathUtil::DelPath(folder);
            }
        }
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

#include "PathUtil.h"
#include <AppKit/AppKit.h>

namespace Echo
{
    String PathUtil::GetCurrentDir()
    {
        return [[[NSBundle mainBundle] resourcePath] UTF8String];
    }
}


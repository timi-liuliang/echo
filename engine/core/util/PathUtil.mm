#include "PathUtil.h"
#include <AppKit/AppKit.h>

namespace Echo
{
    // 获取当前路径
    String PathUtil::GetCurrentDir()
    {
        return [[[NSBundle mainBundle] resourcePath] UTF8String];
    }
}


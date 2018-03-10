#if LORD_PLATFORM==LORD_PLATFORM_MAC_IOS

#include "PathUtil.h"
#import  <UIKit/UIKit.h>

namespace LORD
{
    // 获取当前路径
    String PathUtil::GetCurrentDir()
    {
        return [[[NSBundle mainBundle] resourcePath] UTF8String];
    }
}

#endif
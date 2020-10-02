#include "RenderWindowMetal.h"
#import "RenderWindowMetalDelegate.h"

@implementation RendererWindowMetalDelegate
{
    Studio::RenderWindowMetal* m_renderWindow;
}

/// Initialize with the MetalKit view from which we'll obtain our metal device
- (nonnull instancetype)initWithMetalKitView:(nonnull void*)window
{
    self = [super init];
    m_renderWindow = (Studio::RenderWindowMetal*)window;
    
    return self;
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
}

//- (void)drawInMTKView:(nonnull MTKView *)view;
//{
//    @autoreleasepool
//    {
//        m_renderWindow->Render();
//    }
//}

@end

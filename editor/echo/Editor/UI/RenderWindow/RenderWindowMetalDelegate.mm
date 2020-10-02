#import "RenderWindowMetalDelegate.h"

@implementation RendererWindowMetalDelegate {}

/// Initialize with the MetalKit view from which we'll obtain our metal device
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView
{
    self = [super init];
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
 
}

@end

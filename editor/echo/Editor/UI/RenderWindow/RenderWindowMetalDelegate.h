#pragma once

#import <MetalKit/MTKView.h>

// Our platform independent render class
@interface RendererWindowMetalDelegate : NSObject<MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull void*)window;

@end

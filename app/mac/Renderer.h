#import <MetalKit/MetalKit.h>

// Our platform independent renderer class.   Implements the MTKViewDelegate protocol which
//   allows it to accept per-frame update and drawable resize callbacks.
@interface Renderer : NSObject <MTKViewDelegate>

// init
-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;

// init window size
-(nonnull instancetype)initWindowSize;

@end


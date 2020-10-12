#include <engine/core/input/input.h>
#import "GameViewController.h"
#import "Renderer.h"

@implementation GameViewController
{
    MTKView *_view;

    Renderer *_renderer;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    _view = (MTKView *)self.view;
    _view.device = MTLCreateSystemDefaultDevice();
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;

    if(!_view.device)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }

    _renderer = [[Renderer alloc] initWithMetalKitView:_view];
    [_renderer mtkView:_view drawableSizeWillChange:_view.bounds.size];

    _view.delegate = _renderer;
}

- (Echo::Vector2)mouseLocationInWindow:(NSEvent*)event
{
    return Echo::Vector2([event locationInWindow].x, _view.bounds.size.height - [event locationInWindow].y) * _view.layer.contentsScale;
}

- (void)mouseDown:(NSEvent *)event
{
    Echo::Input::instance()->notifyMouseButtonDown(0, [self mouseLocationInWindow:event]);
}

- (void)mouseUp:(NSEvent *)event
{
    Echo::Input::instance()->notifyMouseButtonUp(0, [self mouseLocationInWindow:event]);
}

- (void)rightMouseDown:(NSEvent *)event
{
    Echo::Input::instance()->notifyMouseButtonDown(1, [self mouseLocationInWindow:event]);
}

- (void)rightMouseUp:(NSEvent *)event
{
    Echo::Input::instance()->notifyMouseButtonUp(1, [self mouseLocationInWindow:event]);
}

- (void)mouseMoved:(NSEvent *)event
{
    Echo::Input::instance()->notifyMouseMove(0, [self mouseLocationInWindow:event]);
}

- (void)scrollWheel:(NSEvent *)event
{
    
}

- (void)keyDown:(NSEvent *)event
{
    Echo::Input::instance()->notifyKeyDown(event.keyCode);
}

- (void)keyUp:(NSEvent *)event
{
    Echo::Input::instance()->notifyKeyUp(event.keyCode);
}

@end

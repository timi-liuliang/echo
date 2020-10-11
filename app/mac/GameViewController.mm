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

- (void)mouseDown:(NSEvent *)event
{
    CGPoint windowPos = [event locationInWindow];
    Echo::Input::instance()->notifyMouseButtonDown(0, Echo::Vector2(windowPos.x, windowPos.y));
}

- (void)mouseUp:(NSEvent *)event
{
    CGPoint windowPos = [event locationInWindow];
    Echo::Input::instance()->notifyMouseButtonUp(0, Echo::Vector2(windowPos.x, windowPos.y));
}

- (void)rightMouseDown:(NSEvent *)event
{
    CGPoint windowPos = [event locationInWindow];
    Echo::Input::instance()->notifyMouseButtonDown(1, Echo::Vector2(windowPos.x, windowPos.y));
}

- (void)rightMouseUp:(NSEvent *)event
{
    CGPoint windowPos = [event locationInWindow];
    Echo::Input::instance()->notifyMouseButtonUp(1, Echo::Vector2(windowPos.x, windowPos.y));
}

- (void)mouseMoved:(NSEvent *)event
{
    CGPoint windowPos = [event locationInWindow];
    Echo::Input::instance()->notifyMouseMove(0, Echo::Vector2(windowPos.x, windowPos.y));
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

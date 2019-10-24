#import "IOSViewController.h"
#import "IOSView.h"
#include "Application.h"
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/input/input.h>

static CGFloat g_viewWidth = 0.0f;
static CGFloat g_viewHeight = 0.0f;
static bool  g_isInited = false;
static float g_nativeScale = 1.f;

// get width
float iOSGetScreenWidth()
{
    return g_viewWidth;
}

// get width
float iOSGetScreenHeight()
{
    return g_viewHeight;
}

@implementation IOSViewController

- (void)loadView
{
    CGRect frame = [[UIScreen mainScreen] bounds];
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!self.context || ![EAGLContext setCurrentContext:self.context])
    {
        return;
    }

    IOSView* view = [[IOSView alloc] initWithFrame:frame];
    view->context = self.context;
    [view CreateFrameBuffer];

    self.view = view;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.view.multipleTouchEnabled = YES;

    CGFloat screenScale = [UIScreen mainScreen].scale;
    CGRect screen_orig_rect = [[UIScreen mainScreen] bounds];
    g_viewWidth = screen_orig_rect.size.width * screenScale;
    g_viewHeight = screen_orig_rect.size.height * screenScale;
    g_nativeScale = screenScale;

    CADisplayLink* display_link = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
    [display_link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)dealloc
{
    [EAGLContext setCurrentContext: nil];
    
    [super dealloc];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    if( g_isInited)
    {
        NSSet* allTouches = [event allTouches];

        int touchIdx = 0;
        for(UITouch* touch in allTouches)
        {
            if(touchIdx==0)
            {
                CGPoint touchPoint = [touch locationInView:[touch view]];
                Echo::Input::instance()->notifyMouseButtonDown(0, Echo::Vector2(touchPoint.x * g_nativeScale, touchPoint.y * g_nativeScale));
            }

            touchIdx++;
        }
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{

}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    if( g_isInited)
    {
        NSSet* allTouches = [event allTouches];

        int touchIdx = 0;
        for(UITouch* touch in allTouches)
        {
            if(touchIdx==0)
            {
                CGPoint touchPoint = [touch locationInView:[touch view]];
                Echo::Input::instance()->notifyMouseButtonUp(0, Echo::Vector2(touchPoint.x * g_nativeScale, touchPoint.y * g_nativeScale));
            }

            touchIdx++;
        }
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{

}

- (void)drawView: (CADisplayLink*)display_link
{
    if( !g_isInited)
    {
        Echo::String rootPath = [[[NSBundle mainBundle] resourcePath] UTF8String];
        Echo::Application::instance()->init( g_viewWidth, g_viewHeight, rootPath);

        g_isInited = true;
    }
    else
    {
        // calc elapsed time
        static float timeStamp = CACurrentMediaTime();
        float currentTime = CACurrentMediaTime();
        float elapsedTime = currentTime - timeStamp;
        timeStamp = currentTime;

        // tick every frame
        Echo::Application::instance()->tick(elapsedTime);
    }
}

- (void)MakeCurrent
{
    [EAGLContext setCurrentContext:self.context];
}

- (void)SwapBuffers
{
    [self.context presentRenderbuffer:GL_RENDERBUFFER];
}

@end

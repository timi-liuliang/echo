#import "GLView.h"
#import <GLKit/GLKit.h>
#include "Application.h"

static int   g_viewWidth = 0;
static int   g_viewHeight = 0;
static bool  g_isInited = false;
static float g_nativeScale = 1.f;

static Echo::Application* g_app = NULL;

#if 0
// 获取iOS平台沉浸屏幕大小
void iOSGetClientSizeImpl(int32_t& width, int32_t& height)
{
    width = g_viewWidth;
    height = g_viewHeight;
}
#endif

@implementation GLViewController

// 初始化
-(void) initialize
{
}

// 销毁
-(void) destroy
{
    
}

// 加载结束
-(void) viewDidLoad
{
    [super viewDidLoad];
    
    self.view.multipleTouchEnabled = YES;
    EAGLRenderingAPI renderAPI = kEAGLRenderingAPIOpenGLES2;
    self.context = [[EAGLContext alloc] initWithAPI:renderAPI];
    self.preferredFramesPerSecond = 60;
    
    if(!self.context)
        return;
    
    GLKView* view = (GLKView*)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self initialize];
}

// 每帧渲染调用
-(void)glkView:(GLKView*)view drawInRect:(CGRect)rect
{
    if( !g_isInited)
    {
        GLint viewPort[4];
        glGetIntegerv(GL_VIEWPORT, viewPort);
        
        g_viewWidth = viewPort[2];
        g_viewHeight= viewPort[3];
        
        g_nativeScale = viewPort[2] / rect.size.width;
        
        g_app = new Echo::Application;
        //g_app->start( viewPort[2], viewPort[3]);
        
        g_isInited = true;
    }
    else
    {
        static float timeStamp = CACurrentMediaTime();
        
        float currentTime = CACurrentMediaTime();
        float elapsedTime = currentTime - timeStamp;
        timeStamp = currentTime;
            
        // 更新
        //g_app->tick(elapsedTime * 1000);
    }
}

// 触摸开始
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    if( g_isInited)
    {
        NSSet* allTouches = [event allTouches];
        
        int touchIdx = 0;
        for(UITouch* touch in allTouches)
        {
            CGPoint touchPoint = [touch locationInView:[touch view]];
            //g_app->mouseLBProc( touchPoint.x * g_nativeScale, touchPoint.y * g_nativeScale);
        }
    }
}

// 触摸结束
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    if( g_isInited)
    {
        NSSet* allTouches = [event allTouches];
        
        int touchIdx = 0;
        for(UITouch* touch in allTouches)
        {
            CGPoint touchPoint = [touch locationInView:[touch view]];
            //g_app->mouseLBProc( touchPoint.x * g_nativeScale, touchPoint.y * g_nativeScale, true);
        }
    }
}

- (void)MakeCurrent
{
    [EAGLContext setCurrentContext:self.context];
}

@end

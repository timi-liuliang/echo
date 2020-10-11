#include "Log.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/HashGenerator.h"
#include "engine/core/render/base/renderer.h"
#include <engine/core/main/Engine.h>
#include <engine/core/main/GameSettings.h>
#import "Renderer.h"

// globa metal view
static MTKView * g_view = nullptr;

// implement Renderer
@implementation Renderer
{
    Echo::String    m_projectFile;
    Echo::GameLog*  m_log;
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    self = [super init];
    
    g_view = view;
    m_projectFile = [[[NSBundle mainBundle] resourcePath] UTF8String] + Echo::String("/data/app.echo");
    Echo::PathUtil::FormatPath(m_projectFile, false);
    
    m_log = EchoNew(Echo::GameLog("Game"));
    Echo::Log::instance()->addOutput(m_log);
    
    Echo::initRender((size_t)view);
    
    Echo::Engine::Config rootcfg;
    rootcfg.m_projectFile = m_projectFile;
    rootcfg.m_isGame = true;
    rootcfg.m_userPath = Echo::PathUtil::GetCurrentDir() + "/user/" + Echo::StringUtil::Format("u%d/", Echo::BKDRHash(m_projectFile.c_str()));
    Echo::PathUtil::FormatPath(rootcfg.m_userPath);
    Echo::Engine::instance()->initialize(rootcfg);
    
    [self initWindowSize];
    
    return self;
}

-(nonnull instancetype)initWindowSize
{
    // default window size
    Echo::GameSettings* settings = Echo::GameSettings::instance();
    if(settings->isFullScreen())
    {
        NSRect screenRect = [[NSScreen mainScreen] frame];
        [g_view setFrameSize:screenRect.size];
        
        NSWindow* window = [g_view window];
        [window toggleFullScreen:self];
    }
    else
    {
        float  contentsScale = 1.f;
        CGSize newSize = { settings->getWindowWidth() * contentsScale, settings->getWindowHeight() * contentsScale};
        [g_view setFrameSize:newSize];
    }
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    // calc elapsed time
    static float timeStamp = CACurrentMediaTime();
    float currentTime = CACurrentMediaTime();
    float elapsedTime = currentTime - timeStamp;
    timeStamp = currentTime;

    // tick every frame
    Echo::Engine::instance()->tick(elapsedTime);
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    Echo::Engine::instance()->onSize(size.width, size.height);
}

@end

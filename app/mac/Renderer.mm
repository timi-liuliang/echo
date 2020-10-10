#include "Log.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/HashGenerator.h"
#include "engine/core/render/base/renderer.h"
#include <engine/core/main/Engine.h>
#import "Renderer.h"

@implementation Renderer
{
    Echo::String    m_projectFile;
    Echo::GameLog*  m_log;
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
{
    self = [super init];
    
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
    
    return self;
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
    
}

@end

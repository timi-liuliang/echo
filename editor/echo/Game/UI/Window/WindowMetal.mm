#include "WindowMetal.h"
#include <QHBoxLayout>
#include <QWindow>
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <Metal/MTLRenderPipeline.h>
#include <MetalKit/MTKView.h>
#include <QDateTime>
#include <QMacCocoaViewContainer>
#include "GameMainWindow.h"
#include <engine/core/main/GameSettings.h>

@interface GameRenderWindowMetalDelegate : NSObject <MTKViewDelegate>

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;

@end

@implementation GameRenderWindowMetalDelegate
{
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    self = [super init];
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    @autoreleasepool
    {
        static Echo::Dword lastTime = QDateTime::currentMSecsSinceEpoch();

        // calc delta Time
        Echo::Dword curTime = QDateTime::currentMSecsSinceEpoch();
        Echo::Dword elapsedTime = curTime - lastTime;

        // Call the main render function
        Echo::Engine::instance()->tick(elapsedTime * 0.001f);

        lastTime = curTime;
    }
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
}

@end

namespace Game
{
    WindowMetal::WindowMetal(QWidget* parent)
        : Window(parent)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setMargin(0);
        this->setLayout(layout);
        layout->addWidget(newMetalWindow());
    }

    WindowMetal::~WindowMetal()
    {
        
    }

    void WindowMetal::start(const Echo::String& echoProject)
    {
        m_app->init((size_t)m_mtkView, echoProject);

        // change window size
        GameMainWindow::instance()->setRenderWindowSize(Echo::GameSettings::instance()->getWindowWidth(), Echo::GameSettings::instance()->getWindowHeight());

        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(checkWindowSize()));
        m_timer->start(50);
    }

    QWidget* WindowMetal::newMetalWindow()
    {
        MTKView *view = [[[MTKView alloc] init] autorelease];
        view.device = MTLCreateSystemDefaultDevice();
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
        
        // Create Renderer
        GameRenderWindowMetalDelegate* renderer = [[GameRenderWindowMetalDelegate alloc] initWithMetalKitView:view];
        [renderer mtkView:view drawableSizeWillChange:view.drawableSize];
        view.delegate = renderer;

        // Create and show a Qt Window which controls the metal view
        QMacCocoaViewContainer* container = new QMacCocoaViewContainer(view, this);
        m_mtkView = view;
        
        return container;
    }
}

#include "WindowMetal.h"
#include <QHBoxLayout>
#include <QWindow>
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <Metal/MTLRenderPipeline.h>
#include <MetalKit/MTKView.h>
#include <QDateTime>
#include "GameMainWindow.h"
#include <engine/core/main/GameSettings.h>

namespace Game
{
    WindowMetal::WindowMetal(QWidget* parent)
        : Window(parent)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setMargin(0);
        this->setLayout(layout);
        layout->addWidget(QWidget::createWindowContainer(newMetalWindow()));
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
        connect(m_timer, SIGNAL(timeout()), this, SLOT(renderMetal()));
        m_timer->start(10);
    }

    void  WindowMetal::renderMetal()
    {
        @autoreleasepool
        {
            checkWindowSize();

            static Echo::Dword lastTime = QDateTime::currentMSecsSinceEpoch();

            // calc delta Time
            Echo::Dword curTime = QDateTime::currentMSecsSinceEpoch();
            Echo::Dword elapsedTime = curTime - lastTime;

            // Call the main render function
            m_app->tick(elapsedTime * 0.001f);

            lastTime = curTime;
        }
    }

    QWindow* WindowMetal::newMetalWindow()
    {
        MTKView *view = [[[MTKView alloc] init] autorelease];
        view.device = MTLCreateSystemDefaultDevice();
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;

        // Create and show a Qt Window which controls the metal view
        QWindow* window = QWindow::fromWinId((WId)view);
        window->setSurfaceType(QSurface::MetalSurface);
        m_mtkView = view;
        
        return window;
    }
}

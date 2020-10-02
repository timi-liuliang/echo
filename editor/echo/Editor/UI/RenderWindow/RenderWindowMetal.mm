#include "RenderWindowMetal.h"
#include <QHBoxLayout>
#include <QWindow>
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <Metal/MTLRenderPipeline.h>
#include <MetalKit/MTKView.h>
#include "EchoEngine.h"
#include "MainWindow.h"
#include "InputController2d.h"
#include "InputController3d.h"
#include "OperationManager.h"
#include <QDateTime>

namespace Studio
{
    RenderWindowMetal::RenderWindowMetal(QWidget* parent)
        : RenderWindow(parent)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setMargin(0);
        this->setLayout(layout);
        layout->addWidget(QWidget::createWindowContainer(newMetalWindow()));
    }

    RenderWindowMetal::~RenderWindowMetal()
    {
        
    }

    void RenderWindowMetal::BeginRender()
    {
        EchoEngine::instance()->Initialize((size_t)m_mtkView);

        if (!m_inputController2d)
            m_inputController2d = new InputController2d;

        if (!m_inputController3d)
            m_inputController3d = new InputController3d;

        m_inputController = m_inputController2d;

        if (!m_transformWidget)
            m_transformWidget = EchoNew(TransformWidget);
        
        m_timer = new QTimer(this);
        QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(RenderlMetal()));
        m_timer->start(10);
    }

    void RenderWindowMetal::RenderlMetal()
    {
        @autoreleasepool
        {
            ResizeWindow();

            static Echo::Dword lastTime = QDateTime::currentMSecsSinceEpoch();

            // calc delta Time
            Echo::Dword curTime = QDateTime::currentMSecsSinceEpoch();
            Echo::Dword elapsedTime = curTime - lastTime;

            QPointF pos = m_inputController->mousePosition();
            Qt::MouseButton button = m_inputController->pressedMouseButton();
            float elapsed = elapsedTime * 0.001f;
            InputContext ctx(pos, button, elapsed);

            m_inputController->tick(ctx);

            // operations
            OperationManager::instance()->tick();

            // Call the main render function
            EchoEngine::instance()->Render(elapsed, this->isVisible());

            lastTime = curTime;
        }
    }

    QWindow* RenderWindowMetal::newMetalWindow()
    {
        MTKView *view = [[[MTKView alloc] init] autorelease];
        view.device = MTLCreateSystemDefaultDevice();
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
        
        // Create Renderer
        //RendererWindowMetalDelegate* renderer = [[RendererWindowMetalDelegate alloc] initWithMetalKitView:view];
        //[renderer mtkView:view drawableSizeWillChange:view.drawableSize];
        //view.delegate = renderer;

        // Create and show a Qt Window which controls the metal view
        QWindow* window = QWindow::fromWinId((WId)view);
        window->setSurfaceType(QSurface::MetalSurface);
        m_mtkView = view;
        
        return window;
    }
}

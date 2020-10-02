#include "RenderWindowMetal.h"
#include <QHBoxLayout>
#include <QWindow>
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <Metal/MTLRenderPipeline.h>
#include <MetalKit/MTKView.h>
#include "RenderWindowMetalDelegate.h"

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

#ifdef ECHO_PLATFORM_MAC
//    RenderWindow* RenderWindow::create()
//    {
//        return new RenderWindowMetal;
//    }
#endif

    QWindow* RenderWindowMetal::newMetalWindow()
    {
        MTKView *view = [[[MTKView alloc] init] autorelease];
        view.device = MTLCreateSystemDefaultDevice();
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        view.depthStencilPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
        
        // Create Renderer
        RendererWindowMetalDelegate* renderer = [[RendererWindowMetalDelegate alloc] initWithMetalKitView:view];
        view.delegate = renderer;

        // Create and show a Qt Window which controls the metal view
        QWindow* window = QWindow::fromWinId((WId)view);
        return window;
    }
}

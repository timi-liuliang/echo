#include "mt_framebuffer_window.h"
#include "mt_renderer.h"

namespace Echo
{
    MTFrameBufferWindow::MTFrameBufferWindow(ui32 width, ui32 height, void* handle)
        : FrameBuffer(0, width, height)
    {
        makeViewMetalCompatible(handle);
    }

    MTFrameBufferWindow::~MTFrameBufferWindow()
    {
    }

    bool MTFrameBufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {

        return true;
    }

    bool MTFrameBufferWindow::end()
    {
        return true;
    }

    void MTFrameBufferWindow::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;
        
        float  contentsScale = m_metalLayer.contentsScale;
        CGSize newSize = { m_width * contentsScale, m_height * contentsScale};
        
        [m_metalLayer setDrawableSize: newSize];
    }
    
    NSView* MTFrameBufferWindow::makeViewMetalCompatible(void* handle)
    {
        MTRenderer* mtRenderer = ECHO_DOWN_CAST<MTRenderer*>(Renderer::instance());
        
        m_metalView = (NSView*)handle;
        
        if (![m_metalView.layer isKindOfClass:[CAMetalLayer class]])
        {
            m_metalLayer = [CAMetalLayer layer];
            m_metalLayer.device = mtRenderer->getMetalDevice();
            m_metalLayer.framebufferOnly = true;
            m_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            [m_metalView setLayer:m_metalLayer];
            [m_metalView setWantsLayer:YES];
        }
        
        return m_metalView;
    }
}

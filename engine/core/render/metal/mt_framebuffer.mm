#include "mt_framebuffer.h"
#include "mt_renderer.h"

namespace Echo
{
    static FrameBuffer* g_current = nullptr;
    
    MTFrameBufferOffscreen::MTFrameBufferOffscreen(ui32 width, ui32 height)
        : FrameBufferOffScreen(width, height)
    {
    }
    
    MTFrameBufferOffscreen::~MTFrameBufferOffscreen()
    {
    }
    
    bool MTFrameBufferOffscreen::begin(const Color& backgroundColor, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        g_current = this;
        return true;
    }
    
    bool MTFrameBufferOffscreen::end()
    {
        return true;
    }
    
    MTFrameBufferWindow::MTFrameBufferWindow(void* handle)
        : FrameBufferWindow()
    {
        makeViewMetalCompatible(handle);
    }
    
    MTFrameBufferWindow::~MTFrameBufferWindow()
    {
    }
    
    bool MTFrameBufferWindow::begin(const Color& backgroundColor, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        g_current = this;
        return true;
    }
    
    bool MTFrameBufferWindow::end()
    {
        return true;
    }
    
    void MTFrameBufferWindow::onSize(ui32 width, ui32 height)
    {
    }
    
    NSView* MTFrameBufferWindow::makeViewMetalCompatible(void* handle)
    {
        MTRenderer* mtRenderer = ECHO_DOWN_CAST<MTRenderer*>(Renderer::instance());
        
        m_metalView = (MTKView*)handle;
        
        return m_metalView;
    }
}

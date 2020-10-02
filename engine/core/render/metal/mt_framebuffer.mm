#include "mt_framebuffer.h"
#include "mt_renderer.h"

namespace Echo
{
    static MTFrameBuffer* g_current = nullptr;
    
    void MTFrameBuffer::onSize(ui32 width, ui32 height)
    {
        
    }
    
    MTFrameBuffer* MTFrameBuffer::current()
    {
        return g_current;
    }
    
    bool MTFrameBuffer::begin(bool isClearColor, const Color& bgColor, bool isClearDepth, float depthValue, bool isClearStencil, ui8 stencilValue)
    {
        g_current = this;
        
        return false;
    }
    
    MTFrameBufferOffscreen::MTFrameBufferOffscreen(ui32 id, ui32 width, ui32 height)
        : MTFrameBuffer(id, width, height)
    {
    }
    
    MTFrameBufferOffscreen::~MTFrameBufferOffscreen()
    {
    }
    
    bool MTFrameBufferOffscreen::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        MTFrameBuffer::begin(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);
        
        return true;
    }
    
    bool MTFrameBufferOffscreen::end()
    {
        return true;
    }
    
    MTFrameBufferWindow::MTFrameBufferWindow(ui32 width, ui32 height, void* handle)
        : MTFrameBuffer(0, width, height)
    {
        makeViewMetalCompatible(handle);
    }
    
    MTFrameBufferWindow::~MTFrameBufferWindow()
    {
    }
    
    bool MTFrameBufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        MTFrameBuffer::begin(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);
        
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
        
        //float  contentsScale = m_metalLayer.contentsScale;
        //CGSize newSize = { m_width * contentsScale, m_height * contentsScale};
        
        //[m_metalLayer setDrawableSize: newSize];
    }
    
    NSView* MTFrameBufferWindow::makeViewMetalCompatible(void* handle)
    {
        MTRenderer* mtRenderer = ECHO_DOWN_CAST<MTRenderer*>(Renderer::instance());
        
        m_metalView = (MTKView*)handle;
        
        return m_metalView;
    }
}

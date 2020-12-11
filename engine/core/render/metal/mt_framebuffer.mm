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
    
    MTFrameBufferWindow::MTFrameBufferWindow()
        : FrameBufferWindow()
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
            m_metalCommandQueue = [device newCommandQueue];
    }
    
    MTFrameBufferWindow::~MTFrameBufferWindow()
    {
    }
    
    bool MTFrameBufferWindow::begin(const Color& backgroundColor, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        MTKView* view = MTRenderer::instance()->getMetalView();
        if(view)
        {
            // clear color
            view.clearColor = { backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a};
            
            // create command buffer
            m_metalCommandBuffer = [m_metalCommandQueue commandBuffer];

            // creat a command encoder
            m_metalRenderCommandEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:view.currentRenderPassDescriptor];
            
            // set current
            MTRenderer::instance()->setMetalRenderCommandEncoder(m_metalRenderCommandEncoder);
            
            return true;
        }
        
        return false;
    }
    
    bool MTFrameBufferWindow::end()
    {
        MTKView* view = MTRenderer::instance()->getMetalView();
        if(view)
        {
            [m_metalRenderCommandEncoder endEncoding];
            [m_metalCommandBuffer presentDrawable:view.currentDrawable];
            [m_metalCommandBuffer commit];
            
            return true;
        }
        
        return false;
    }
    
    void MTFrameBufferWindow::onSize(ui32 width, ui32 height)
    {
    }
}

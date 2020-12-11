#pragma once

#include "engine/core/render/base/frame_buffer.h"
#include "mt_render_base.h"

namespace Echo
{
    class MTFrameBufferOffscreen : public FrameBufferOffScreen
    {
    public:
        MTFrameBufferOffscreen(ui32 width, ui32 height);
        virtual ~MTFrameBufferOffscreen();
        
        // begin render
        virtual bool begin(const Color& backgroundColor, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;
        
    private:
    };
    
    class MTFrameBufferWindow : public FrameBufferWindow
    {
    public:
        MTFrameBufferWindow();
        virtual ~MTFrameBufferWindow();
        
        // begin render
        virtual bool begin(const Color& backgroundColor, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;
        
        // on resize
        virtual void onSize(ui32 width, ui32 height) override;
        
    public:        
        // get metal render command encoder
        id<MTLRenderCommandEncoder> getMetalRenderCommandEncoder() { return m_metalRenderCommandEncoder; }
        
    private:
        id<MTLCommandQueue>             m_metalCommandQueue;
        id<MTLCommandBuffer>            m_metalCommandBuffer;
        id<MTLRenderCommandEncoder>     m_metalRenderCommandEncoder;
    };
}

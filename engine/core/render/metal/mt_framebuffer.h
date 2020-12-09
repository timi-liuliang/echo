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
        MTFrameBufferWindow(void* handle);
        virtual ~MTFrameBufferWindow();
        
        // begin render
        virtual bool begin(const Color& backgroundColor, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;
        
        // on resize
        virtual void onSize(ui32 width, ui32 height) override;
        
    public:
        // get metal layer
        MTKView* getMetalView() { return m_metalView; }
        
    private:
        // make metal layer
        NSView* makeViewMetalCompatible(void* handle);
        
    private:
        MTKView*                        m_metalView = nullptr;
    };
}

#pragma once

#include "engine/core/render/interface/FrameBuffer.h"
#include "mt_render_base.h"

namespace Echo
{
    class MTFrameBufferWindow : public FrameBuffer
    {
    public:
        MTFrameBufferWindow(ui32 width, ui32 height, void* handle);
        virtual ~MTFrameBufferWindow();

        // begin render
        virtual bool begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;
        
    public:
        // get metal layer
        CAMetalLayer* getMetalLayer() { return m_metalLayer; }
        
    private:
        // make metal layer
        NSView* makeViewMetalCompatible(void* handle);
        
    private:
        NSView*                         m_metalView = nullptr;
        CAMetalLayer*                   m_metalLayer = nullptr;
    };
}

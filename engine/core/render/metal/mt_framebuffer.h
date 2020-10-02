#pragma once

#include "engine/core/render/base/frame_buffer.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTFrameBuffer : public FrameBuffer
	{
	public:
        MTFrameBuffer(ui32 id, ui32 width, ui32 height) : FrameBuffer( id, width, height) {}
        
        // current
        static MTFrameBuffer* current();
        
        // begin render
        virtual bool begin(bool isClearColor, const Color& bgColor, bool isClearDepth, float depthValue, bool isClearStencil, ui8 stencilValue) override;
        virtual bool end() override {return true; }
        
        // on resize
        virtual void onSize(ui32 width, ui32 height) override;
	};
    
    class MTFrameBufferOffscreen : public MTFrameBuffer
    {
    public:
        MTFrameBufferOffscreen(ui32 id, ui32 width, ui32 height);
        virtual ~MTFrameBufferOffscreen();
        
        // begin render
        virtual bool begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;
        
    private:
    };
    
    class MTFrameBufferWindow : public MTFrameBuffer
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
        MTKView* getMetalView() { return m_metalView; }
        
    private:
        // make metal layer
        NSView* makeViewMetalCompatible(void* handle);
        
    private:
        MTKView*                        m_metalView = nullptr;
    };
}

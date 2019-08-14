#pragma once

#include "engine/core/render/interface/RenderTarget.h"

namespace Echo
{
	class MTRenderTarget : public RenderTarget
	{
	public:
        MTRenderTarget() : RenderTarget( 0, 0, 0, PF_RGBA8_UINT) {}
        
        // create
        virtual bool create() override { return true; }
        
        // begin render
        virtual bool beginRender(bool clearColor, const Color& bgColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override {return true;}
        
        // clear
        virtual void clear(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override {}
        
        // end render
        virtual bool endRender() override {return true; }
        
        // disable frame buffer
        virtual bool invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil) override {return true;}
        
        // on resize
        virtual void onResize(ui32 width, ui32 height);
        
        // save target
        virtual bool save(const char* file) override { return true;}
	};
}

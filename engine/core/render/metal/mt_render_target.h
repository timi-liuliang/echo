#pragma once

#include "engine/core/render/interface/RenderTarget.h"

namespace Echo
{
	class MTRenderTarget : public RenderTarget
	{
	public:
        MTRenderTarget() : RenderTarget( 0, 0, 0, PF_RGBA8_UINT) {}
        
        // store restore renderTarget
        virtual bool doStoreDefaultRenderTarget() override { return true;}
        virtual bool doRestoreDefaultRenderTarget() override { return true;}
        
    protected:
        virtual bool doCreate() { return true;}
        virtual bool doCreateCubemap() { return true; }
        virtual bool doBeginRender( bool _clearColor, const Color& _backgroundColor,  bool _clearDepth, float _depthValue, bool _clearStencil, ui8 stencilValue ) {return true; }
        virtual bool doEndRender() {return true;}
        virtual bool doInvalidateFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil) { return true;}
        virtual void doClear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value) {}
        virtual void doOnResize( ui32 _width, ui32 _height ) {}
        virtual bool doSaveTo(const char* file) {return true;}
	};
}

#pragma once

#include "engine/core/render/interface/FrameBuffer.h"

namespace Echo
{
	class MTFrameBuffer : public FrameBuffer
	{
	public:
        MTFrameBuffer() : FrameBuffer( 0, 0, 0) {}
        
        // begin render
        virtual bool begin(bool isClearColor, const Color& bgColor, bool isClearDepth, float depthValue, bool isClearStencil, ui8 stencilValue) override { return false; }
        virtual bool end() override {return true; }
        
        // on resize
        virtual void onSize(ui32 width, ui32 height) override;
	};
}

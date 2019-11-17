#pragma once

#include "interface/FrameBuffer.h"

namespace Echo
{
    class GLESFramebufferWindow : public FrameBuffer
    {
    public:
        GLESFramebufferWindow(ui32 width, ui32 height);
        virtual ~GLESFramebufferWindow();

        // begin render
        virtual bool begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;
    };
}

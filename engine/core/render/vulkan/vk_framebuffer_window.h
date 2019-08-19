#pragma once

#include "interface/FrameBuffer.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKFramebufferWindow : public FrameBuffer
    {
    public:
        VKFramebufferWindow(ui32 width, ui32 height);
        virtual ~VKFramebufferWindow();

        // begin render
        virtual bool begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool end() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height);
    };
}
#include "vk_framebuffer_window.h"

namespace Echo
{
    VKFramebufferWindow::VKFramebufferWindow(ui32 width, ui32 height)
        : FrameBuffer(0, width, height)
    {
    }

    VKFramebufferWindow::~VKFramebufferWindow()
    {
    }

    bool VKFramebufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {

        return true;
    }

    bool VKFramebufferWindow::end()
    {
        return true;
    }

    void VKFramebufferWindow::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;
    }
}
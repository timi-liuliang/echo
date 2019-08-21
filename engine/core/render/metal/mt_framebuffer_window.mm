#include "mt_framebuffer_window.h"

namespace Echo
{
    MTFrameBufferWindow::MTFrameBufferWindow(ui32 width, ui32 height)
        : FrameBuffer(0, width, height)
    {
    }

    MTFrameBufferWindow::~MTFrameBufferWindow()
    {
    }

    bool MTFrameBufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {

        return true;
    }

    bool MTFrameBufferWindow::end()
    {
        return true;
    }

    void MTFrameBufferWindow::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;
    }
}

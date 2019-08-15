#include "GLESFrameBufferWindow.h"
#include "GLESFrameBuffer.h"

namespace Echo
{

    GLESFramebufferWindow::GLESFramebufferWindow(ui32 width, ui32 height)
        : FrameBuffer(0, width, height)
    {
    }

    GLESFramebufferWindow::~GLESFramebufferWindow()
    {
    }

    bool GLESFramebufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        // bind frame buffer
        OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        OGLESDebug(glViewport(0, 0, m_width, m_height));

        // clear
        GLESFramebuffer::clear(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);

        return true;
    }

    bool GLESFramebufferWindow::end()
    {
        return true;
    }

    void GLESFramebufferWindow::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;
    }
}
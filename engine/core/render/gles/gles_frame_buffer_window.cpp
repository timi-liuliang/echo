#include "gles_frame_buffer_window.h"
#include "gles_frame_buffer.h"

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
    #if defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_ANDROID)
        OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    #endif

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

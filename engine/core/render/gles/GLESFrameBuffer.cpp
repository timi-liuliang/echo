#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "GLESRenderBase.h"
#include "GLESMapping.h"
#include "GLESFrameBuffer.h"
#include "GLESTexture2D.h"

namespace Echo
{
    #define INVALIDE  0xFFFFFFFF

    GLESFramebuffer::GLESFramebuffer( ui32 id, ui32 width, ui32 height, PixelFormat pixelFormat)
		: FrameBuffer(id, width, height)
	{
        OGLESDebug(glGenFramebuffers(1, &m_fbo));
	}

    GLESFramebuffer::~GLESFramebuffer()
	{
		OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
	}

    // attach render view
    void GLESFramebuffer::attach(Attachment attachment, RenderView* renderView)
    {
        GLESTexture2D* texture = dynamic_cast<GLESTexture2D*>(renderView->getTexture());
        GLenum esAttachment = attachment == Attachment::DepthStencil ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0;

        OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
        OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, esAttachment, GL_TEXTURE_2D, texture->m_glesTexture, 0));
        OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        m_views[(ui8)attachment] = renderView;
    }

	bool GLESFramebuffer::beginRender(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
	{
		// bind frame buffer
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glViewport(0, 0, m_width, m_height));

		// clear
		clear( clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue );

		return true;
	}

	bool GLESFramebuffer::endRender()
	{
		return true;
	}

	bool GLESFramebuffer::invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
	{
		int attachment_count = 0;
		GLenum attachments[128] = { 0 };
		if (invalidateColor)
			attachments[attachment_count++] = GL_COLOR_ATTACHMENT0;
		if (invalidateDepth)
			attachments[attachment_count++] = GL_DEPTH_ATTACHMENT;
		if (invalidateStencil)
			attachments[attachment_count++] = GL_STENCIL_ATTACHMENT;

		if (attachment_count > 0)
		{
			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

#ifndef ECHO_PLATFORM_ANDROID
			OGLESDebug(glInvalidateFramebuffer(GL_FRAMEBUFFER, attachment_count, attachments));
#endif
		}
		return true;
	}

	void GLESFramebuffer::clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
	{
		GLbitfield mask = 0;

		if (clear_color)
		{
			OGLESDebug(glClearColor(color.r, color.g, color.b, color.a));
			mask |= GL_COLOR_BUFFER_BIT;
		}

		if (clear_depth)
		{
			OGLESDebug(glClearDepthf(depth_value));
			mask |= GL_DEPTH_BUFFER_BIT;
		}

		OGLESDebug(glDepthMask(clear_depth));

		if (clear_stencil)
		{
			OGLESDebug(glClearStencil(stencil_value));
			mask |= GL_STENCIL_BUFFER_BIT;
		}

		OGLESDebug(glStencilMask(clear_stencil));

		if (mask != 0)
			OGLESDebug(glClear(mask));

		Renderer::instance()->setDepthStencilState( Renderer::instance()->getDefaultDepthStencilState());
	}

	void GLESFramebuffer::onSize( ui32 width, ui32 height )
	{
        m_width = width;
        m_height = height;

        for (RenderView* colorView : m_views)
        {
            if (colorView)
                colorView->onSize(width, height);
        }
	}
}

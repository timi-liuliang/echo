#include "engine/core/util/PathUtil.h"
#include "base/Renderer.h"
#include "gles_render_base.h"
#include "gles_mapping.h"
#include "gles_frame_buffer.h"
#include "gles_texture_2d.h"
#include "gles_texture_render.h"

namespace Echo
{
    GLESFrameBufferOffScreen::GLESFrameBufferOffScreen(ui32 width, ui32 height)
		: FrameBufferOffScreen(width, height)
        , m_fbo(0)
	{
        OGLESDebug(glGenFramebuffers(1, &m_fbo));
		m_esTextures.assign(0);
	}

    GLESFrameBufferOffScreen::~GLESFrameBufferOffScreen()
	{
		OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
	}

	bool GLESFrameBufferOffScreen::bind(i32& width, i32& height)
	{
		if (checkScreenSize(width, height))
		{
			// Bind framebuffer
			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

			// Attach textures to frame buffer
			attach();

			// Specifies a list of color buffers to be drawn into
			specifyColorBuffers();

			// Return result
			return checkFramebufferStatus();
		}

		return false;
	}

	bool GLESFrameBufferOffScreen::checkScreenSize(i32& width, i32& height)
	{
		if (!hasColorAttachment()) return false;
		if (!hasDepthAttachment()) return false;

		width = m_views[i32(Attachment::DepthStencil)]->getWidth();
		height = m_views[i32(Attachment::DepthStencil)]->getHeight();
		if (!width || !height) return false;

		for (i32 i = i32(Attachment::ColorA); i <= i32(Attachment::DepthStencil); i++)
		{
			if (m_views[i])
			{
				if (width != m_views[i]->getWidth()) return false;
				if (height != m_views[i]->getHeight()) return false;
			}
		}

		return true;
	}

    void GLESFrameBufferOffScreen::attach()
    {
		for (i32 i = i32(Attachment::ColorA); i <= i32(Attachment::DepthStencil); i++)
		{
			GLESTextureRender* texture = dynamic_cast<GLESTextureRender*>(m_views[i].ptr());
			GLuint esTexture = texture ? texture->getGlesTexture() : 0;
			if (esTexture != m_esTextures[i])
			{
				if (i != Attachment::DepthStencil)
				{
					OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, esTexture, 0));
				}
				else
				{
					OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, esTexture));
				}

				m_esTextures[i] = esTexture;
			}
		}
    }

	void GLESFrameBufferOffScreen::specifyColorBuffers()
	{
		m_attachments.clear();

		for (i32 i = i32(Attachment::ColorA); i < i32(Attachment::DepthStencil); i++)
		{
			GLESTextureRender* texture = dynamic_cast<GLESTextureRender*>(m_views[i].ptr());
			if (texture && texture->getGlesTexture())
			{
				m_attachments.emplace_back(GL_COLOR_ATTACHMENT0 + i);
			}
		}

		OGLESDebug(glDrawBuffers(m_attachments.size(), m_attachments.data()));
	}

	bool GLESFrameBufferOffScreen::checkFramebufferStatus()
	{
#ifdef ECHO_EDITOR_MODE
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status == GL_FRAMEBUFFER_COMPLETE)
		{
			return true;
		}
		else
		{
			EchoLogError("Bind frame buffer [%s] failed.", getPath().c_str());

			return false;
		}
#else
		return true;
#endif
	}

	bool GLESFrameBufferOffScreen::begin()
	{
		i32 width = 0;
		i32 height = 0;
		if (bind(width, height))
		{
			OGLESDebug(glViewport(0, 0, width, height));

			clear(m_isClearDepth, m_clearDepth, m_isClearStencil, m_clearStencil);

			return true;
		}

		return false;
	}

	bool GLESFrameBufferOffScreen::end()
	{
		return true;
	}

	void GLESFrameBufferOffScreen::clear(bool clearDepth, float depth_value, bool clear_stencil, ui8 stencil_value)
	{
		for (i32 i = i32(Attachment::ColorA); i < i32(Attachment::DepthStencil); i++)
		{
			if (m_isClearColor[i] && m_views[i])
			{
				float bgColor[4] = { m_clearColor[i].r, m_clearColor[i].g, m_clearColor[i].b, m_clearColor[i].a };
				OGLESDebug(glClearBufferfv(GL_COLOR, i, bgColor));
			}
		}

		GLbitfield mask = 0;
		if (clearDepth)
		{
			OGLESDebug(glClearDepthf(depth_value));
			mask |= GL_DEPTH_BUFFER_BIT;
		}

		OGLESDebug(glDepthMask(clearDepth));

		if (clear_stencil)
		{
			OGLESDebug(glClearStencil(stencil_value));
			mask |= GL_STENCIL_BUFFER_BIT;
		}

		OGLESDebug(glStencilMask(clear_stencil));

		if (mask != 0)
		{
			OGLESDebug(glClear(mask));
		}
	}

	void GLESFrameBufferOffScreen::onSize( ui32 width, ui32 height )
	{
        for (TextureRenderTarget2D* colorView : m_views)
        {
            if (colorView)
                colorView->onSize(width, height);
        }
	}

	// https://docs.gl/es3/glReadPixels
	bool GLESFrameBufferOffScreen::readPixels(Attachment attach, Pixels& pixels)
	{
		if (m_views[attach])
		{
			if (attach != Attachment::DepthStencil)
			{
				PixelFormat pixFmt = PixelFormat::PF_RGBA8_UNORM;
				GLenum glFmt = GLESMapping::MapInternalFormat(pixFmt);
				GLenum glType = GLESMapping::MapDataType(pixFmt);
				pixels.set(m_views[attach]->getWidth(), m_views[attach]->getHeight(), pixFmt);

				OGLESDebug(glReadBuffer(GL_COLOR_ATTACHMENT0 + attach));
				OGLESDebug(glReadPixels(0, 0, pixels.m_width, pixels.m_height, glFmt, glType, pixels.m_data.data()));
			}
			else
			{
				pixels.set(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight(), PixelFormat::PF_R32_FLOAT);

				OGLESDebug(glReadBuffer(GL_DEPTH_ATTACHMENT));
				OGLESDebug(glReadPixels(0, 0, pixels.m_width, pixels.m_height, GL_DEPTH_COMPONENT, GL_FLOAT, pixels.m_data.data()));
			}

			return true;
		}

		return false;
	}

	GLESFramebufferWindow::GLESFramebufferWindow()
		: FrameBufferWindow()
	{
	}

	GLESFramebufferWindow::~GLESFramebufferWindow()
	{
	}

	bool GLESFramebufferWindow::begin()
	{
		// bind frame buffer
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		ui32 width = Renderer::instance()->getWindowWidth();
		ui32 height = Renderer::instance()->getWindowHeight();
		OGLESDebug(glViewport(0, 0, width, height));

		// clear
		clear(isClearColor(), getClearColorValue(), m_isClearDepth, m_clearDepth, m_isClearStencil, m_clearStencil);

		return true;
	}

	bool GLESFramebufferWindow::end()
	{
		return true;
	}

	void GLESFramebufferWindow::onSize(ui32 width, ui32 height)
	{
	}

	void GLESFramebufferWindow::clear(bool clearColor, const Color& color, bool clearDepth, float depth_value, bool clear_stencil, ui8 stencil_value)
	{
		GLbitfield mask = 0;
		if (clearColor)
		{
			OGLESDebug(glClearColor(color.r, color.g, color.b, color.a));
			mask |= GL_COLOR_BUFFER_BIT;
		}

		if (clearDepth)
		{
			OGLESDebug(glClearDepthf(depth_value));
			mask |= GL_DEPTH_BUFFER_BIT;
		}

		OGLESDebug(glDepthMask(clearDepth));

		if (clear_stencil)
		{
			OGLESDebug(glClearStencil(stencil_value));
			mask |= GL_STENCIL_BUFFER_BIT;
		}

		OGLESDebug(glStencilMask(clear_stencil));

		if (mask != 0)
		{
			OGLESDebug(glClear(mask));
		}
	}

	bool GLESFramebufferWindow::readPixels(Attachment attach, Pixels& pixels)
	{
		if (attach == Attachment::ColorA)
		{
			pixels.set(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight(), PixelFormat::PF_RGBA8_UNORM);

			// https://docs.gl/es3/glReadBuffer
			OGLESDebug(glReadBuffer(GL_COLOR_ATTACHMENT0));
			OGLESDebug(glReadPixels(0, 0, pixels.m_width, pixels.m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.m_data.data()));

			return true;
		}
		else if (attach == Attachment::DepthStencil)
		{
			pixels.set(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight(), PixelFormat::PF_R32_FLOAT);

			OGLESDebug(glReadBuffer(GL_DEPTH_ATTACHMENT));
			OGLESDebug(glReadPixels(0, 0, pixels.m_width, pixels.m_height, GL_RED, GL_FLOAT, pixels.m_data.data()));

			return true;
		}

		return false;
	}
}

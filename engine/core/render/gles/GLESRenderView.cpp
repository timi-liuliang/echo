#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "GLESRenderBase.h"
#include "GLESMapping.h"
#include "GLESRenderView.h"
#include "GLESTexture2D.h"

namespace Echo
{
    #define INVALIDE  0xFFFFFFFF

    GLESRenderView::GLESRenderView(ui32 width, ui32 height, PixelFormat pixelFormat)
		: RenderView(width, height, pixelFormat)
		, m_fbo(0)
	{
		SamplerState::SamplerDesc desc;
		desc.addrUMode = SamplerState::AM_CLAMP;
		desc.addrVMode = SamplerState::AM_CLAMP;
		desc.addrWMode = SamplerState::AM_CLAMP;
		desc.mipFilter = SamplerState::FO_NONE;

        static ui32 id = 0;
		m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(id++));
		m_bindTexture->setSamplerState(desc);
	}

    GLESRenderView::~GLESRenderView()
	{
		if (m_fbo != INVALIDE)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
		}

		m_bindTexture->subRefCount();
	}

	bool GLESRenderView::create()
	{
		EchoAssert(m_bindTexture);
		Texture* texture = m_bindTexture;
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_depth = 1;
		texture->m_pixFmt = m_format;
		texture->m_isCompressed = false;
		texture->m_compressType = Texture::CompressType_Unknown;

		return createTexture2D();
	}

	bool GLESRenderView::createTexture2D()
	{
		GLESTexture2D* texture = dynamic_cast<GLESTexture2D*>(m_bindTexture);
		EchoAssert(texture);

        bool isDepthForamt = PixelUtil::IsDepth(m_format);

		OGLESDebug(glGenTextures(1, &texture->m_glesTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, texture->m_glesTexture));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapInternalFormat(m_format), m_width, m_height, 0, GLES2Mapping::MapFormat(m_format), GLES2Mapping::MapDataType(m_format), (GLvoid*)0));

		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, isDepthForamt ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_glesTexture, 0));
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_pixFmt = m_format;

		const SamplerState* sampleState = m_bindTexture->getSamplerState();
		EchoAssert(sampleState);
		sampleState->active(NULL);

		GLuint uStatus = OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
		if (uStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			EchoLogError("Create RenderTarget Failed !");
		}

#if defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_ANDROID)
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif

		return true;
	}

	bool GLESRenderView::beginRender(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
	{
		// bind frame buffer
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glViewport(0, 0, m_width, m_height));

		// clear
		clear( clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue );

		return true;
	}

	bool GLESRenderView::endRender()
	{
		return true;
	}

	bool GLESRenderView::invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
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

	void GLESRenderView::clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
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

	void GLESRenderView::onResize( ui32 _width, ui32 _height )
	{
		m_width = _width;
		m_height = _height;

		if (m_fbo != INVALIDE)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
			m_fbo = 0;
		}

        static ui32 id = 0;
		EchoAssert(m_bindTexture);
		m_bindTexture->subRefCount();
		m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(id++));

		// recreate
		create();
	}
}

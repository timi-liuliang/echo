#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "GLESRenderBase.h"
#include "GLESMapping.h"
#include "GLESFrameBuffer.h"
#include "GLESTexture2D.h"

namespace Echo
{
    #define INVALIDE  0xFFFFFFFF

    GLESFramebuffer::GLESFramebuffer( ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const Options& option)
		: FrameBuffer(_id, _width, _height, _pixelFormat, option)
		, m_fbo(0)
	{
		SamplerState::SamplerDesc desc;
		desc.addrUMode = SamplerState::AM_CLAMP;
		desc.addrVMode = SamplerState::AM_CLAMP;
		desc.addrWMode = SamplerState::AM_CLAMP;
		desc.mipFilter = SamplerState::FO_NONE;

		m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(_id));
		m_bindTexture->setSamplerState(desc);
		m_depthTexture = Renderer::instance()->createTexture2D(("rtDEPTH_") + StringUtil::ToString(_id));
		m_depthTexture->setSamplerState(desc);
	}

    GLESFramebuffer::~GLESFramebuffer()
	{
		if (m_fbo != INVALIDE)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
		}

		m_bindTexture->subRefCount();
		m_depthTexture->subRefCount();
	}

	bool GLESFramebuffer::create()
	{
		EchoAssert(m_bindTexture);
		Texture* texture = m_bindTexture;
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_depth = 1;
		texture->m_pixFmt = m_pixelFormat;
		texture->m_isCompressed = false;
		texture->m_compressType = Texture::CompressType_Unknown;

		return createTexture2D();
	}

	bool GLESFramebuffer::createTexture2D()
	{
		GLESTexture2D* texture = dynamic_cast<GLESTexture2D*>(m_bindTexture);
		EchoAssert(texture);

		OGLESDebug(glGenTextures(1, &texture->m_glesTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, texture->m_glesTexture));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapInternalFormat(m_pixelFormat), m_width, m_height, 0, GLES2Mapping::MapFormat(m_pixelFormat), GLES2Mapping::MapDataType(m_pixelFormat), (GLvoid*)0));

		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_glesTexture, 0));
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_pixFmt = m_pixelFormat;

		const SamplerState* sampleState = m_bindTexture->getSamplerState();
		EchoAssert(sampleState);
		sampleState->active(NULL);

		if( m_isHasDepth )
		{
			GLESTexture2D* depthTexture = dynamic_cast<GLESTexture2D*>(m_depthTexture);
			EchoAssert(depthTexture);

			// 将深度缓冲区映射到纹理上(这里应该分情况讨论，rbo效率更高，在不需要depth tex时应该优先使用
			OGLESDebug(glGenTextures(1, &depthTexture->m_glesTexture));
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, depthTexture->m_glesTexture));
			OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0,  GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
			OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->m_glesTexture, 0));

			depthTexture->m_width = m_width;
			depthTexture->m_height = m_height;
			depthTexture->m_pixFmt = m_pixelFormat;

			const SamplerState* depthSampleState = m_depthTexture->getSamplerState();
			EchoAssert(depthSampleState);
			depthSampleState->active(NULL);
		}

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

	void GLESFramebuffer::onResize( ui32 _width, ui32 _height )
	{
		if( m_id != 0)
		{
			m_width = _width;
			m_height = _height;

			if (m_fbo != INVALIDE)
			{
				OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
				m_fbo = 0;
			}

			EchoAssert(m_bindTexture);
			m_bindTexture->subRefCount();
			m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(m_id));

			EchoAssert(m_depthTexture);
			m_depthTexture->subRefCount();
			m_depthTexture = Renderer::instance()->createTexture2D("rtDEPTH_" + StringUtil::ToString(m_id));

			// recreate
			create();
		}
		else
		{
			m_width  = _width;
			m_height = _height;
		}
	}
}

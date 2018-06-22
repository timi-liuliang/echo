#include "GLES2RenderBase.h"
#include "GLES2Renderer.h"
#include "GLES2DepthStencilView.h"
#include "GLES2Mapping.h"
#include <engine/core/Util/Exception.h>
#include "Render/PixelFormat.h"

namespace Echo
{
	GLES2DepthStencilView::GLES2DepthStencilView(PixelFormat pixFmt, ui32 width, ui32 height)
		: DepthStencilView(pixFmt, width, height)
	{
		OGLESDebug(glGenRenderbuffers(1, &m_hRBO));
		if (!m_hRBO)
		{
			EchoLogError("Create GLES2DepthStencilView failed.");
		}

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_hRBO));

		GLenum glPixFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, glPixFmt, width, height));
	}
	
	GLES2DepthStencilView::~GLES2DepthStencilView()
	{
		OGLESDebug(glDeleteRenderbuffers(1, &m_hRBO));
	}
	
	void GLES2DepthStencilView::onAttached()
	{
		if (PixelUtil::IsDepth(m_pixFmt))
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hRBO));

		if (PixelUtil::IsStencil(m_pixFmt))
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_hRBO));
	}
	
	void GLES2DepthStencilView::onDetached()
	{
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));
	}
	
	GLuint GLES2DepthStencilView::getRBOHandle() const
	{
		return m_hRBO;
	}
}

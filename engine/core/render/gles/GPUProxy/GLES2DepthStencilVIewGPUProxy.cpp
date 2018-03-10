#include "GLES2RenderStd.h"
#include "GLES2Mapping.h"
#include "GLES2DepthStencilVIewGPUProxy.h"
#include "engine/core/Util/Exception.h"


namespace Echo
{

	void GLES2DepthStencilViewGPUProxy::createView(PixelFormat pixFmt, ui32 width, ui32 height)
	{
		OGLESDebug(glGenRenderbuffers(1, &m_hRBO));
		if (!m_hRBO)
		{
			EchoException("Create GLES2DepthStencilView failed.");
		}

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_hRBO));

		GLenum glPixFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, glPixFmt, width, height));

		//glBindRenderbuffer(GL_RENDERBUFFER, NULL);
	}

	void GLES2DepthStencilViewGPUProxy::destroyView()
	{
		OGLESDebug(glDeleteRenderbuffers(1, &m_hRBO));
	}

	void GLES2DepthStencilViewGPUProxy::onAttached(PixelFormat pixFmt)
	{
		if (PixelUtil::IsDepth(pixFmt))
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hRBO));

		if (PixelUtil::IsStencil(pixFmt))
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_hRBO));
	}

	void GLES2DepthStencilViewGPUProxy::onDetached()
	{
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));
	}

}
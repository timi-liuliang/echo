#include "GLES2RenderStd.h"
#include "Render/PixelFormat.h"
#include "engine/core/Util/Exception.h"
#include "GLES2Mapping.h"
#include "GLES2TargetViewGPUProxy.h"

namespace Echo
{

	void GLES2TargetViewGPUProxy::createRenderBuffer(PixelFormat pixFmt, ui32 width, ui32 height)
	{
		OGLESDebug(glGenRenderbuffers(1, &m_hRBO));
		if (!m_hRBO)
		{
			EchoException("Create GLES2RenderTargetView failed.");
		}

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_hRBO));

		GLenum internalFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, internalFmt, width, height));

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, 0));
	}

	void GLES2TargetViewGPUProxy::deleteRenderBuffer()
	{
		OGLESDebug(glDeleteRenderbuffers(1, &m_hRBO));
	}

	void GLES2TargetViewGPUProxy::onAttached()
	{
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_hRBO));
	}

}
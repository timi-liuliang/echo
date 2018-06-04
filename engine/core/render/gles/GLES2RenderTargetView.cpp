#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2RenderTargetView.h"
#include "GLES2Mapping.h"
#include "engine/core/Util/Exception.h"

namespace Echo
{
	GLES2RenderTargetView::GLES2RenderTargetView(PixelFormat pixFmt, ui32 width, ui32 height)
		: RenderTargetView(pixFmt, width, height)
	{
		OGLESDebug(glGenRenderbuffers(1, &m_hRBO));
		if (!m_hRBO)
		{
			EchoLogError("Create GLES2RenderTargetView failed.");
		}

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_hRBO));

		GLenum internalFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, internalFmt, width, height));

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, 0));
	}

	GLES2RenderTargetView::GLES2RenderTargetView(ui32 hRBO, ui32 width, ui32 height)
		: RenderTargetView(PF_RGBA8_UNORM, width, height)
	{
		m_hRBO = hRBO;
	}

	GLES2RenderTargetView::~GLES2RenderTargetView()
	{
		OGLESDebug(glDeleteRenderbuffers(1, &m_hRBO));
	}
	
	void GLES2RenderTargetView::onAttached(ui32 idx)
	{
		EchoAssertX(idx == 0, "GLES2 only support one render target.");
		m_attIdx = idx;

		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_hRBO));
	}
	
	void GLES2RenderTargetView::onDetached()
	{
		m_attIdx = -1;
	}

}

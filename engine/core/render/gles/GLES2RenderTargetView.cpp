#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2Renderer.h"
#include "GLES2RenderTargetView.h"
#include "GLES2Mapping.h"
#include "engine/core/Util/Exception.h"


namespace Echo
{
	GLES2RenderTargetView::GLES2RenderTargetView(PixelFormat pixFmt, ui32 width, ui32 height)
		: RenderTargetView(pixFmt, width, height)
#ifdef ECHO_RENDER_THREAD
		, m_gpuProxy(EchoNew(GLES2TargetViewGPUProxy))
#endif
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2TargetViewTaskCreateRenderBuffer>::CreateTask(m_gpuProxy, pixFmt, width, height);
#else
		OGLESDebug(glGenRenderbuffers(1, &m_hRBO));
		if (!m_hRBO)
		{
			EchoException("Create GLES2RenderTargetView failed.");
		}

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_hRBO));

		GLenum internalFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, internalFmt, width, height));

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, 0));
#endif
	}

	GLES2RenderTargetView::GLES2RenderTargetView(ui32 hRBO, ui32 width, ui32 height)
		: RenderTargetView(PF_RGBA8_UNORM, width, height)
#ifdef ECHO_RENDER_THREAD
		, m_gpuProxy(EchoNew(GLES2TargetViewGPUProxy))
#endif
	{
#ifdef ECHO_RENDER_THREAD
		m_gpuProxy->m_hRBO = hRBO;
#else
		m_hRBO = hRBO;
#endif
	}

	GLES2RenderTargetView::~GLES2RenderTargetView()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2TargetViewTaskDeleteRenderBuffer>::CreateTask(m_gpuProxy);
#else
		OGLESDebug(glDeleteRenderbuffers(1, &m_hRBO));
#endif
	}
	
	void GLES2RenderTargetView::onAttached(ui32 idx)
	{
		EchoAssertX(idx == 0, "GLES2 only support one render target.");
		m_attIdx = idx;
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2TargetViewTaskOnAttached>::CreateTask(m_gpuProxy);
#else
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_hRBO));
#endif
	}
	
	void GLES2RenderTargetView::onDetached()
	{
		m_attIdx = -1;
	}

}

#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "GLES2Renderer.h"
#include "GLES2DepthStencilView.h"
#include "GLES2Mapping.h"
#include "GLES2DepthStencilVIewGPUProxy.h"
#include "GLES2DepthStencilViewTasks.h"
#include <engine/core/Util/Exception.h>
#include "Render/PixelFormat.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"



namespace Echo
{
	GLES2DepthStencilView::GLES2DepthStencilView(PixelFormat pixFmt, ui32 width, ui32 height)
		: DepthStencilView(pixFmt, width, height)
#ifdef ECHO_RENDER_THREAD
		, m_gpuProxy(EchoNew(GLES2DepthStencilViewGPUProxy))
#endif
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2DepthStencilViewTaskCreateView>::CreateTask(m_gpuProxy, pixFmt, width, height);
#else
		OGLESDebug(glGenRenderbuffers(1, &m_hRBO));
		if (!m_hRBO)
		{
			EchoException("Create GLES2DepthStencilView failed.");
		}

		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_hRBO));

		GLenum glPixFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, glPixFmt, width, height));
#endif
	}
	
	GLES2DepthStencilView::~GLES2DepthStencilView()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2DepthStencilViewTaskDestroyView>::CreateTask(m_gpuProxy);
#else
		OGLESDebug(glDeleteRenderbuffers(1, &m_hRBO));
#endif
	}
	
	void GLES2DepthStencilView::onAttached()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2DepthStencilViewTaskOnAttached>::CreateTask(m_gpuProxy, m_pixFmt);
#else
		if (PixelUtil::IsDepth(m_pixFmt))
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hRBO));

		if (PixelUtil::IsStencil(m_pixFmt))
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_hRBO));
#endif
	}
	
	void GLES2DepthStencilView::onDetached()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2DepthStencilViewTaskOnDetached>::CreateTask(m_gpuProxy);
#else
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));
#endif
	}
	
#ifndef ECHO_RENDER_THREAD
	GLuint GLES2DepthStencilView::getRBOHandle() const
	{
		return m_hRBO;
	}
#endif

}

#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESFrameBuffer.h"
#include "GLESRenderState.h"
#include <engine/core/log/Log.h>

namespace Echo
{
	GLES2FrameBuffer::GLES2FrameBuffer()
	{
		//OGLESDebug(glGenFramebuffers(1, &m_hFBO));
	}

	GLES2FrameBuffer::GLES2FrameBuffer(GLuint hFBO)
		: m_hFBO(hFBO)
	{
	}

	GLES2FrameBuffer::~GLES2FrameBuffer()
	{
		if (m_hFBO > 0)
		{
			//OGLESDebug(glDeleteFramebuffers(1, &m_hFBO));
		}
	}

	//void GLES2FrameBuffer::clear(Dword clearFlags, const Color& color, float depth, int stencil)
	//{
	//	GLbitfield mask = 0;

	//	if (clearFlags & CM_COLOR)
	//	{
	//		OGLESDebug(glClearColor(color.r, color.g, color.b, color.a));
	//		mask |= GL_COLOR_BUFFER_BIT;
	//	}

	//	//if(m_pDSV)
	//	{
	//		if ((clearFlags & CM_DEPTH))
	//		{
	//			OGLESDebug(glClearDepthf(depth));
	//			mask |= GL_DEPTH_BUFFER_BIT;
	//		}

	//		if ((clearFlags & CM_STENCIL))
	//		{
	//			OGLESDebug(glClearStencil(stencil));
	//			mask |= GL_STENCIL_BUFFER_BIT;
	//		}
	//	}

	//	if (mask != 0)
	//	{
	//		GLES2DepthStencilState* pDSState = (GLES2DepthStencilState*)Renderer::instance()->getDepthStencilState();
	//		bool needRevertDepthWrite;
	//		if (pDSState)
	//		{
	//			needRevertDepthWrite = !(pDSState->getDesc().bWriteDepth);
	//			if (needRevertDepthWrite)
	//				OGLESDebug(glDepthMask(GL_TRUE));
	//		}

	//		OGLESDebug(glClear(mask));
	//		if (needRevertDepthWrite)
	//		{
	//			OGLESDebug(glDepthMask(GL_FALSE));
	//		}
	//	}
	//}

	//void GLES2FrameBuffer::bind()
	//{
	//	OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_hFBO));
	//}

	//GLuint GLES2FrameBuffer::getFBOHandle() const
	//{
	//	return m_hFBO;
	//}
}

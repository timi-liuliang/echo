#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2FrameBuffer.h"
#include "GLES2RenderState.h"
#include <engine/core/log/LogManager.h>

namespace Echo
{
	GLES2FrameBuffer::GLES2FrameBuffer()
	{
		OGLESDebug(glGenFramebuffers(1, &m_hFBO));
	}

	GLES2FrameBuffer::GLES2FrameBuffer(GLuint hFBO)
		: m_hFBO(hFBO)
	{
	}

	GLES2FrameBuffer::~GLES2FrameBuffer()
	{
		if (m_hFBO > 0)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_hFBO));
		}
	}

	void GLES2FrameBuffer::clear(Dword clearFlags, const Color& color, float depth, int stencil)
	{
		GLbitfield mask = 0;

		if (clearFlags & CM_COLOR)
		{
			OGLESDebug(glClearColor(color.r, color.g, color.b, color.a));
			mask |= GL_COLOR_BUFFER_BIT;
		}

		//if(m_pDSV)
		{
			if ((clearFlags & CM_DEPTH))
			{
				OGLESDebug(glClearDepthf(depth));
				mask |= GL_DEPTH_BUFFER_BIT;
			}

			if ((clearFlags & CM_STENCIL))
			{
				OGLESDebug(glClearStencil(stencil));
				mask |= GL_STENCIL_BUFFER_BIT;
			}
		}

		if (mask != 0)
		{
			GLES2DepthStencilState* pDSState = (GLES2DepthStencilState*)Renderer::instance()->getDepthStencilState();
			bool needRevertDepthWrite;
			if (pDSState)
			{
				needRevertDepthWrite = !(pDSState->getDesc().bWriteDepth);
				if (needRevertDepthWrite)
					OGLESDebug(glDepthMask(GL_TRUE));
			}

			OGLESDebug(glClear(mask));
			if (needRevertDepthWrite)
			{
				OGLESDebug(glDepthMask(GL_FALSE));
			}
		}
	}

	// °ó¶¨Ö¡»º³å
	void GLES2FrameBuffer::bind()
	{
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_hFBO));
	}

	GLuint GLES2FrameBuffer::getFBOHandle() const
	{
		return m_hFBO;
	}

	inline void GLES2FrameBuffer::swap()
	{
		// angleproject do not implement GLext glDiscardFramebufferEXT function
#if 0	// (ECHO_PLATFORM == ECHO_PLATFORM_WINDOWS)
		if (glDiscardFramebufferEXT != NULL)
		{
			const GLint numAttachments = 3;
			GLenum attachments[numAttachments];
			GLint currentAttachment = 0;

			attachments[currentAttachment] = GL_COLOR_EXT;
			currentAttachment++;

			attachments[currentAttachment] = GL_DEPTH_EXT;
			currentAttachment++;

			attachments[currentAttachment] = GL_STENCIL_EXT;
			currentAttachment++;

			glDiscardFramebufferEXT(GL_FRAMEBUFFER, currentAttachment, attachments);
		}

		if (GLES2EchoRender)
		{
			eglSwapBuffers(static_cast<EGLDisplay>(static_cast<GLES2Renderer *>(GLES2EchoRender)->getDisplay()), static_cast<EGLSurface>(static_cast<GLES2Renderer *>(GLES2EchoRender)->getSurface()));

			EGLint no_erro = eglGetError();

			if (no_erro != GL_NO_ERROR && no_erro != EGL_SUCCESS)
			{
				EchoLogError("GLES2FrameBuffer: Swap() Failed !");
				return;
			}
		}
#endif
	}
}

#include "GLES2RenderStd.h"
#include "GLES2RendererProxy.h"
#include "GLES2Renderer.h"

#ifdef ECHO_PLATFORM_WINDOWS
namespace Echo
{
	extern GLES2Renderer* g_renderer;
}

void present()
{
	if (Echo::g_renderer->glDiscardFramebufferEXT)
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

		OGLESDebug(Echo::g_renderer->glDiscardFramebufferEXT(GL_FRAMEBUFFER, currentAttachment, attachments));
	}


	eglSwapBuffers(static_cast<EGLDisplay>(Echo::g_renderer->getDisplay()), static_cast<EGLSurface>(Echo::g_renderer->getSurface()));
	EGLint no_erro = eglGetError();

	if (no_erro != GL_NO_ERROR && no_erro != EGL_SUCCESS)
	{
		EchoLogError("GLES2Renderer: Present() Failed !");
	}
}
#endif

namespace Echo
{
	void vertexAttribPointer(ui32 index, int count, ui32 type, bool normalized, size_t stride, size_t offset)
	{
		OGLESDebug(glVertexAttribPointer(index, count, type, normalized, stride, (GLvoid*)offset));
	}

	void enableVertexAttribArray(ui32 index)
	{
		OGLESDebug(glEnableVertexAttribArray(index));
	}

	void disableVertexAttribArray(ui32 index)
	{
		OGLESDebug(glDisableVertexAttribArray(index));
	}

	void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
	{
		OGLESDebug(glDrawElements(mode, count, type, indices));
	}

	void DrawArrays(GLenum mode, GLint first, GLsizei count)
	{
		OGLESDebug(glDrawArrays(mode, first, count));
	}

	void PolygonMode(GLenum face, GLenum mode)
	{
		EchoLogWarning("no implementation.");
	}

	void BindTexture(ui32 slot, GLenum target, GLuint texture)
	{
		OGLESDebug(glActiveTexture(GL_TEXTURE0 + slot));
		OGLESDebug(glBindTexture(target, texture));
	}

	void Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		OGLESDebug(glEnable(GL_SCISSOR_TEST));
		OGLESDebug(glScissor(x, y, width, height));
	}

	void EndScissor()
	{
		OGLESDebug(glDisable(GL_SCISSOR_TEST));
	}

	void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		OGLESDebug(glViewport(x, y, width, height));
	}

	void GetViewport(GLint viewPort[4])
	{
		OGLESDebug(glGetIntegerv(GL_VIEWPORT, viewPort));
	}

}
#include "Render/PixelFormat.h"
#include "GLES2RenderStd.h"
#include "GLES2RenderTarget.h"
#include "GLES2TargetGPUProxy.h"
#include "GLES2Mapping.h"

namespace LORD
{

	GLES2TargetGPUProxy::GLES2TargetGPUProxy()
		: m_fbo(RenderTarget::Invalid_Value)
		, m_rbo(RenderTarget::Invalid_Value)
	{
	}

	void GLES2TargetGPUProxy::createFrameBuffer(uint* frame_texture_handle, PixelFormat pixel_format, uint width, uint height)
	{
		OGLESDebug(glGenTextures(1, frame_texture_handle));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, *frame_texture_handle));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapFormat(pixel_format), width, height, 0, GLES2Mapping::MapFormat(pixel_format), GL_UNSIGNED_BYTE, (GLvoid*)0));

		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *frame_texture_handle, 0));
	}

	void GLES2TargetGPUProxy::createDepthBuffer(uint* depth_texture_handle, uint width, uint height)
	{
		// FIXME: renderring depth buffer to texture
		// 			OGLESDebug(glGenRenderbuffers(1, &m_gpu_proxy->m_rbo));
		// 			OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_gpu_proxy->m_rbo));
		 
		// 			OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, internalFmt, m_width, m_height));
		// 			OGLESDebug(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_gpu_proxy->m_rbo ));

		// 将深度缓冲区映射到纹理上(这里应该分情况讨论，rbo效率更高，在不需要depth tex时应该优先使用		
		OGLESDebug(glGenTextures(1, depth_texture_handle));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, *depth_texture_handle));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));

		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_texture_handle, 0));
	}

	void GLES2TargetGPUProxy::createDepthBufferCube(uint width, uint height)
	{
		OGLESDebug(glGenRenderbuffers(1, &m_rbo));
		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_rbo));
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo));
	}

	void GLES2TargetGPUProxy::createFrameBufferCube(uint* frame_texture_handle, PixelFormat pixel_format, uint width, uint height)
	{
		OGLESDebug(glGenTextures(1, frame_texture_handle));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, *frame_texture_handle));
		for (int f = 0; f < 6; f++)
		{
			OGLESDebug(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GLES2Mapping::MapFormat(pixel_format), width, height, 0, GLES2Mapping::MapFormat(pixel_format), GL_UNSIGNED_BYTE, (GLvoid*)0));
		}
		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, *frame_texture_handle, 0));
	}

	void GLES2TargetGPUProxy::postFrameBufferCreate()
	{
		GLuint uStatus = OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
		if (uStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			LordLogError("Create RenderTarget Failed !");
		}

#if defined(LORD_PLATFORM_WINDOWS) || defined(LORD_PLATFORM_ANDROID )
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
	}

	void GLES2TargetGPUProxy::deleteBuffers()
	{
		if (m_fbo != RenderTarget::Invalid_Value)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
			m_fbo = 0;
		}

		if (m_rbo != RenderTarget::Invalid_Value)
		{
			OGLESDebug(glDeleteRenderbuffers(1, &m_rbo));
			m_rbo = 0;
		}
	}

	void GLES2TargetGPUProxy::beginRender(bool fb_changed, bool tiled_render, const RenderTarget::Tiled& tile, bool vp_changed, uint width, uint height)
	{
		if (fb_changed)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		}
		GLint fbo, rbo;
		OGLESDebug(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo));
		OGLESDebug(glGetIntegerv(GL_RENDERBUFFER_BINDING, &rbo));

		if (tiled_render)
		{
			OGLESDebug(glViewport(static_cast<GLint>(tile.x_), static_cast<GLint>(tile.y_), static_cast<GLsizei>(tile.width_), static_cast<GLsizei>(tile.height_)));
		}

		if (vp_changed && !tiled_render)
		{
			OGLESDebug(glViewport(0, 0, width, height));
		}
	}

	void GLES2TargetGPUProxy::clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
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
	}

	void GLES2TargetGPUProxy::bindTargetLeft(uint width, uint height)
	{
		OGLESDebug(glViewport(0, 0, width / 2, height));
	}

	void GLES2TargetGPUProxy::bindTargetRight(uint width, uint height)
	{
		OGLESDebug(glViewport(width / 2, 0, width / 2, height));
	}

	void GLES2TargetGPUProxy::readPixels(uint width, uint height, void* pixels)
	{
#ifdef LORD_PLATFORM_WINDOWS
		//	OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER,m_gpu_proxy->m_fbo));
		OGLESDebug(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
#endif
	}

	void GLES2TargetGPUProxy::storeTarget()
	{
		OGLESDebug(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&(m_fbo)));
	}

	void GLES2TargetGPUProxy::restoreTarget()
	{
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
	}

	void GLES2TargetGPUProxy::setCubeFace(Texture::CubeFace cf, uint texture_handle)
	{
#ifdef LORD_PLATFORM_WINDOWS
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, texture_handle));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cf, texture_handle, 0));
		OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
#endif
	}

	void GLES2TargetGPUProxy::bindDepthBuffer(uint* depth_texture_handle)
	{
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_texture_handle, 0));
	}

}
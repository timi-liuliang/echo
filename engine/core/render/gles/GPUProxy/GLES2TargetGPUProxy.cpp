#include "Render/PixelFormat.h"
#include "Render/Renderer.h"
#include "GLES2RenderStd.h"
#include "GLES2RenderTarget.h"
#include "GLES2TargetGPUProxy.h"
#include "GLES2Mapping.h"

namespace Echo
{

	GLES2TargetGPUProxy::GLES2TargetGPUProxy()
		: m_fbo(RenderTarget::Invalid_Value)
		, m_rbo(RenderTarget::Invalid_Value)
	{
	}

	void GLES2TargetGPUProxy::createFrameBuffer(ui32* frame_texture_handle, PixelFormat pixel_format, ui32 width, ui32 height)
	{
		OGLESDebug(glGenTextures(1, frame_texture_handle));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, *frame_texture_handle));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapInternalFormat(pixel_format), width, height, 0, GLES2Mapping::MapFormat(pixel_format), GLES2Mapping::MapDataType(pixel_format), (GLvoid*)0));

		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *frame_texture_handle, 0));
	}

	void GLES2TargetGPUProxy::createDepthBuffer(ui32* depth_texture_handle, ui32 width, ui32 height)
	{
		// FIXME: renderring depth buffer to texture
		// OGLESDebug(glGenRenderbuffers(1, &m_gpu_proxy->m_rbo));
		// OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_gpu_proxy->m_rbo));
		 
		// OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, internalFmt, m_width, m_height));
		// OGLESDebug(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_gpu_proxy->m_rbo ));

		// 将深度缓冲区映射到纹理上(这里应该分情况讨论，rbo效率更高，在不需要depth tex时应该优先使用		
		OGLESDebug(glGenTextures(1, depth_texture_handle));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, *depth_texture_handle));

		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));

		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_texture_handle, 0));
	}

	void GLES2TargetGPUProxy::createDepthBufferCube(ui32 width, ui32 height)
	{
		OGLESDebug(glGenRenderbuffers(1, &m_rbo));
		OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_rbo));
		OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
		OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo));
	}

	void GLES2TargetGPUProxy::createFrameBufferCube(ui32* frame_texture_handle, PixelFormat pixel_format, ui32 width, ui32 height)
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
			EchoLogError("Create RenderTarget Failed !");
		}

#if defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_ANDROID )
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

	void GLES2TargetGPUProxy::beginRender(bool fb_changed, bool tiled_render, const RenderTarget::Tiled& tile, bool vp_changed, ui32 width, ui32 height)
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

	void GLES2TargetGPUProxy::bindTargetLeft(ui32 width, ui32 height)
	{
		OGLESDebug(glViewport(0, 0, width / 2, height));
	}

	void GLES2TargetGPUProxy::bindTargetRight(ui32 width, ui32 height)
	{
		OGLESDebug(glViewport(width / 2, 0, width / 2, height));
	}

	void GLES2TargetGPUProxy::readPixels(ui32 width, ui32 height, void* pixels)
	{
#ifdef ECHO_PLATFORM_WINDOWS
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

	void GLES2TargetGPUProxy::setCubeFace(Texture::CubeFace cf, ui32 texture_handle)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, texture_handle));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cf, texture_handle, 0));
		OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
#endif
	}

	void GLES2TargetGPUProxy::bindDepthBuffer(ui32* depth_texture_handle)
	{
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_texture_handle, 0));
	}
    
    bool GLES2TargetGPUProxy::invalidFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
    {
        int attachment_count = 0;
        GLenum attachments[128] = { 0 };
        if (invalidateColor)
            attachments[attachment_count++] = GL_COLOR_ATTACHMENT0;
        if (invalidateDepth)
            attachments[attachment_count++] = GL_DEPTH_ATTACHMENT;
        if (invalidateStencil)
            attachments[attachment_count++] = GL_STENCIL_ATTACHMENT;
        
        if (attachment_count > 0)
        {
            OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

#ifndef ECHO_PLATFORM_ANDROID
            OGLESDebug(glInvalidateFramebuffer(GL_FRAMEBUFFER, attachment_count, attachments));
#endif
        }
        return true;
    }

}

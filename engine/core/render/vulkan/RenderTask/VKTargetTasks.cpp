#include "Render/PixelFormat.h"
#include "Render/RenderTarget.h"
#include "Render/RenderThread.h"
#include "Foundation/Math/Rect.h"
#include "GLES2RenderStd.h"
#include "GLES2Texture.h"
#include "GLES2TargetGPUProxy.h"
#include "GLES2TextureGPUProxy.h"
#include "GLES2TargetTasks.h"


namespace LORD
{

	GLES2TargetTaskBase::GLES2TargetTaskBase(GLES2TargetGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}


	GLES2TargetTaskCreateFrameBuffer::GLES2TargetTaskCreateFrameBuffer(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, 
		PixelFormat pixel_format, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_frame_texture_proxy(texture_proxy)
		, m_pixel_format(pixel_format)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetTaskCreateFrameBuffer::Execute()
	{
		m_proxy->createFrameBuffer(&m_frame_texture_proxy->m_hTexture, m_pixel_format, m_width, m_height);
	}


	GLES2TargetTaskDestroyFrameBuffer::GLES2TargetTaskDestroyFrameBuffer(GLES2TargetGPUProxy* target_proxy)
		: GLES2TargetTaskBase(target_proxy)
	{
	}

	void GLES2TargetTaskDestroyFrameBuffer::Execute()
	{
		m_proxy->deleteBuffers();
		LordSafeDelete(m_proxy);
	}


	GLES2TargetTaskCreateDepthBuffer::GLES2TargetTaskCreateDepthBuffer(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_depth_texture_proxy(texture_proxy)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetTaskCreateDepthBuffer::Execute()
	{
		m_proxy->createDepthBuffer(&m_depth_texture_proxy->m_hTexture, m_width, m_height);
	}


	GLES2TargetTaskBindDepthBuffer::GLES2TargetTaskBindDepthBuffer(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy)
		: GLES2TargetTaskBase(target_proxy)
		, m_depth_texture_proxy(texture_proxy)
	{
	}

	void GLES2TargetTaskBindDepthBuffer::Execute()
	{
		m_proxy->bindDepthBuffer(&m_depth_texture_proxy->m_hTexture);
	}


	GLES2TargetTaskCreateFrameBufferCube::GLES2TargetTaskCreateFrameBufferCube(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, PixelFormat pixel_format, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_frame_texture_proxy(texture_proxy)
		, m_pixel_format(pixel_format)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetTaskCreateFrameBufferCube::Execute()
	{
		m_proxy->createFrameBufferCube(&m_frame_texture_proxy->m_hTexture, m_pixel_format, m_width, m_height);
	}


	GLES2TargetTaskCreateDepthBufferCube::GLES2TargetTaskCreateDepthBufferCube(GLES2TargetGPUProxy* target_proxy, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_width(width)
		, m_heidht(height)
	{
	}

	void GLES2TargetTaskCreateDepthBufferCube::Execute()
	{
		m_proxy->createDepthBufferCube(m_width, m_heidht);
	}


	GLES2TargetTaskPostFrameBufferCreate::GLES2TargetTaskPostFrameBufferCreate(GLES2TargetGPUProxy* target_proxy)
		: GLES2TargetTaskBase(target_proxy)
	{
	}

	void GLES2TargetTaskPostFrameBufferCreate::Execute()
	{
		m_proxy->postFrameBufferCreate();
	}


	GLES2TargetTaskDeleteBuffersOnly::GLES2TargetTaskDeleteBuffersOnly(GLES2TargetGPUProxy* target_proxy)
		: GLES2TargetTaskBase(target_proxy)
	{
	}

	void GLES2TargetTaskDeleteBuffersOnly::Execute()
	{
		m_proxy->deleteBuffers();
	}

	GLES2TargetTaskBeginRender::GLES2TargetTaskBeginRender(GLES2TargetGPUProxy* target_proxy, bool fb_changed, bool tiled_render, 
		const RenderTarget::Tiled& tile, bool vp_changed, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_fb_changed(fb_changed)
		, m_tiled_render(tiled_render)
		, m_tile(tile)
		, m_vp_changed(vp_changed)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetTaskBeginRender::Execute()
	{
		m_proxy->beginRender(m_fb_changed, m_tiled_render, m_tile, m_vp_changed, m_width, m_height);
	}


	GLES2TargetTaskClear::GLES2TargetTaskClear(GLES2TargetGPUProxy* target_proxy, bool clear_color, const Color& color, 
		bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
		: GLES2TargetTaskBase(target_proxy)
		, m_clear_color(clear_color)
		, m_color(color)
		, m_clear_depth(clear_depth)
		, m_depth_value(depth_value)
		, m_clear_stencil(clear_stencil)
		, m_stencil_value(stencil_value)
	{
	}

	void GLES2TargetTaskClear::Execute()
	{
		m_proxy->clear(m_clear_color, m_color, m_clear_depth, m_depth_value, m_clear_stencil, m_stencil_value);
	}


	GLES2TargetTaskBindTargetLeft::GLES2TargetTaskBindTargetLeft(GLES2TargetGPUProxy* target_proxy, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetTaskBindTargetLeft::Execute()
	{
		m_proxy->bindTargetLeft(m_width, m_height);
	}


	GLES2TargetTaskBindTargetRight::GLES2TargetTaskBindTargetRight(GLES2TargetGPUProxy* target_proxy, uint width, uint height)
		: GLES2TargetTaskBase(target_proxy)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetTaskBindTargetRight::Execute()
	{
		m_proxy->bindTargetRight(m_width, m_height);
	}


	GLES2TargetTaskReadPixels::GLES2TargetTaskReadPixels(GLES2TargetGPUProxy* target_proxy, uint width, uint height, void* pixels)
		: GLES2TargetTaskBase(target_proxy)
		, m_width(width)
		, m_height(height)
		, m_pixels(pixels)
	{
	}

	void GLES2TargetTaskReadPixels::Execute()
	{
		m_proxy->readPixels(m_width, m_height, m_pixels);
	}


	GLES2TargetTaskStoreTarget::GLES2TargetTaskStoreTarget(GLES2TargetGPUProxy* target_proxy)
		: GLES2TargetTaskBase(target_proxy)
	{
	}

	void GLES2TargetTaskStoreTarget::Execute()
	{
		m_proxy->storeTarget();
	}

	GLES2TargetTaskRestoreTarget::GLES2TargetTaskRestoreTarget(GLES2TargetGPUProxy* target_proxy)
		: GLES2TargetTaskBase(target_proxy)
	{
	}

	void GLES2TargetTaskRestoreTarget::Execute()
	{
		m_proxy->restoreTarget();
	}


	GLES2TargetTaskSetCubeFace::GLES2TargetTaskSetCubeFace(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, Texture::CubeFace cf)
		: GLES2TargetTaskBase(target_proxy)
		, m_texture_proxy(texture_proxy)
		, m_cf(cf)
	{
	}

	void GLES2TargetTaskSetCubeFace::Execute()
	{
		m_proxy->setCubeFace(m_cf, m_texture_proxy->m_hTexture);
	}
}
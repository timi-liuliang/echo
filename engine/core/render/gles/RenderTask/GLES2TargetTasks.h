#pragma once

namespace Echo
{
	class GLES2TextureGPUProxy;
	class GLES2TargetGPUProxy;

	class GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskBase(GLES2TargetGPUProxy* proxy);

	protected:
		GLES2TargetGPUProxy* m_proxy;
	};

	class GLES2TargetTaskCreateFrameBuffer : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskCreateFrameBuffer(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, PixelFormat pixel_format, ui32 width, ui32 height);
		void Execute();

	private:
		GLES2TextureGPUProxy* m_frame_texture_proxy;
		PixelFormat m_pixel_format;
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetTaskDestroyFrameBuffer : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskDestroyFrameBuffer(GLES2TargetGPUProxy* target_proxy);
		void Execute();
	};

	class GLES2TargetTaskCreateDepthBuffer : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskCreateDepthBuffer(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, ui32 width, ui32 height);
		void Execute();

	private:
		GLES2TextureGPUProxy* m_depth_texture_proxy;
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetTaskBindDepthBuffer : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskBindDepthBuffer(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy);
		void Execute();

	private:
		GLES2TextureGPUProxy* m_depth_texture_proxy;
	};

	class GLES2TargetTaskCreateFrameBufferCube : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskCreateFrameBufferCube(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, PixelFormat pixel_format, ui32 width, ui32 height);
		void Execute();

	private:
		GLES2TextureGPUProxy* m_frame_texture_proxy;
		PixelFormat m_pixel_format;
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetTaskCreateDepthBufferCube : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskCreateDepthBufferCube(GLES2TargetGPUProxy* target_proxy, ui32 width, ui32 height);
		void Execute();

	private:
		ui32 m_width;
		ui32 m_heidht;
	};

	class GLES2TargetTaskPostFrameBufferCreate : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskPostFrameBufferCreate(GLES2TargetGPUProxy* target_proxy);
		void Execute();
	};

	class GLES2TargetTaskDeleteBuffersOnly : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskDeleteBuffersOnly(GLES2TargetGPUProxy* target_proxy);
		void Execute();
	};

	class GLES2TargetTaskBeginRender : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskBeginRender(GLES2TargetGPUProxy* target_proxy, bool fb_changed, bool tiled_render,
			const RenderTarget::Tiled& tile, bool vp_changed, ui32 width, ui32 height);
		void Execute();

	private:
		bool m_fb_changed;
		bool m_tiled_render;
		RenderTarget::Tiled m_tile;
		bool m_vp_changed;
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetTaskClear : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskClear(GLES2TargetGPUProxy* target_proxy, bool clear_color, const Color& color, 
			bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value);
		void Execute();

	private:
		bool m_clear_color;
		Color m_color;
		bool m_clear_depth;
		float m_depth_value;
		bool m_clear_stencil;
		ui8 m_stencil_value;
	};

	class GLES2TargetTaskBindTargetLeft : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskBindTargetLeft(GLES2TargetGPUProxy* target_proxy, ui32 width, ui32 height);
		void Execute();

	private:
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetTaskBindTargetRight : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskBindTargetRight(GLES2TargetGPUProxy* target_proxy, ui32 width, ui32 height);
		void Execute();

	private:
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetTaskReadPixels : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskReadPixels(GLES2TargetGPUProxy* target_proxy, ui32 width, ui32 height, void* pixels);
		void Execute();

	private:
		ui32 m_width;
		ui32 m_height;
		void* m_pixels;
	};

	class GLES2TargetTaskStoreTarget : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskStoreTarget(GLES2TargetGPUProxy* target_proxy);
		void Execute();
	};

	class GLES2TargetTaskRestoreTarget : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskRestoreTarget(GLES2TargetGPUProxy* target_proxy);
		void Execute();
	};

	class GLES2TargetTaskSetCubeFace : public GLES2TargetTaskBase
	{
	public:
		GLES2TargetTaskSetCubeFace(GLES2TargetGPUProxy* target_proxy, GLES2TextureGPUProxy* texture_proxy, Texture::CubeFace cf);
		void Execute();

	private:
		GLES2TextureGPUProxy* m_texture_proxy;
		Texture::CubeFace m_cf;
	};

}
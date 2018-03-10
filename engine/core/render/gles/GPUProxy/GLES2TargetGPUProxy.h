#pragma once


namespace Echo
{
	class GLES2TargetGPUProxy
	{
		friend class GLES2RenderTarget;
	public:
		GLES2TargetGPUProxy();

		void createFrameBuffer(ui32* frame_texture_handle, PixelFormat pixel_format, ui32 width, ui32 height);
		void createDepthBuffer(ui32* depth_texture_handle, ui32 width, ui32 height);
		void createFrameBufferCube(ui32* frame_texture_handle, PixelFormat pixel_format, ui32 width, ui32 height);
		void createDepthBufferCube(ui32 width, ui32 height);
		void postFrameBufferCreate();
		void deleteBuffers();
		void beginRender(bool fb_changed, bool tiled_render, const RenderTarget::Tiled& tile, bool vp_changed, ui32 width, ui32 height);
		void clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value);
		void bindTargetLeft(ui32 width, ui32 height);
		void bindTargetRight(ui32 width, ui32 height);
		void readPixels(ui32 width, ui32 height, void* pixels);
		void storeTarget();
		void restoreTarget();
		void setCubeFace(Texture::CubeFace cf, ui32 texture_handle);
		void bindDepthBuffer(ui32* depth_texture_handle);
        bool invalidFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil);

	private:
		GLuint m_fbo;
		GLuint m_rbo;
	};
}

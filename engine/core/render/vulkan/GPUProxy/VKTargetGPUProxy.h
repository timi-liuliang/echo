#pragma once


namespace LORD
{
	class GLES2TargetGPUProxy
	{
		friend class GLES2RenderTarget;
	public:
		GLES2TargetGPUProxy();

		void createFrameBuffer(uint* frame_texture_handle, PixelFormat pixel_format, uint width, uint height);
		void createDepthBuffer(uint* depth_texture_handle, uint width, uint height);
		void createFrameBufferCube(uint* frame_texture_handle, PixelFormat pixel_format, uint width, uint height);
		void createDepthBufferCube(uint width, uint height);
		void postFrameBufferCreate();
		void deleteBuffers();
		void beginRender(bool fb_changed, bool tiled_render, const RenderTarget::Tiled& tile, bool vp_changed, uint width, uint height);
		void clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value);
		void bindTargetLeft(uint width, uint height);
		void bindTargetRight(uint width, uint height);
		void readPixels(uint width, uint height, void* pixels);
		void storeTarget();
		void restoreTarget();
		void setCubeFace(Texture::CubeFace cf, uint texture_handle);
		void bindDepthBuffer(uint* depth_texture_handle);

	private:
		GLuint m_fbo;
		GLuint m_rbo;
	};
}
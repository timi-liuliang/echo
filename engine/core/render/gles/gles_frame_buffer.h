#pragma once

#include "base/frame_buffer.h"
#include "gles_render_base.h"

namespace Echo
{
	class GLESFrameBufferOffScreen : public FrameBufferOffScreen
	{
	public:
        GLESFrameBufferOffScreen(ui32 width, ui32 height);
		virtual ~GLESFrameBufferOffScreen();

        // begin render
        virtual bool begin(const Color& bgColor, float depthValue, bool isClearStencil, ui8 stencilValue) override;
        virtual bool end() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;

		// read pixels
		virtual bool readPixels(Attachment attach, PixelBox& pixels) override;

    public:
		// clear render target
		static void clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value);

	protected:
		// prepare
		bool bind(i32& width, i32& height);

		// attach render view
		void attach();

	private:
		GLuint			  m_fbo;
		array<GLuint, 9>  m_esTextures;
	};

	class GLESFramebufferWindow : public FrameBufferWindow
	{
	public:
		GLESFramebufferWindow();
		virtual ~GLESFramebufferWindow();

		// begin render
		virtual bool begin(const Color& backgroundColor,float depthValue, bool clearStencil, ui8 stencilValue) override;
		virtual bool end() override;

		// on resize
		virtual void onSize(ui32 width, ui32 height) override;

		// read pixels
		virtual bool readPixels(Attachment attach, PixelBox& pixels) override;
	};
}

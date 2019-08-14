#pragma once

#include "interface/FrameBuffer.h"

namespace Echo
{
	class GLESFramebuffer : public FrameBuffer
	{
	public:
        GLESFramebuffer(ui32 id, ui32 width, ui32 height, PixelFormat pixelFormat);
		virtual ~GLESFramebuffer();

        // attach render view
        virtual void attach(Attachment attachment, RenderView* renderView) override;

        // begin render
        virtual bool beginRender(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) override;
        virtual bool endRender() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height);

	protected:
		// clear render target
		void clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value);

		// invalide
		virtual bool invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil);

	private:
		GLuint m_fbo;
	};
}

#pragma once

#include "interface/RenderView.h"

namespace Echo
{
	class GLESRenderView : public RenderView
	{
	public:
        GLESRenderView(ui32 width, ui32 height, PixelFormat pixelFormat);
		virtual ~GLESRenderView();

	protected:
		// create
		virtual bool create() override;

		// begin render
		virtual bool beginRender( bool clearColor, const Color& backgroundColor,  bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue);

		// clear render target
		virtual void clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value) override;

		// end render
		virtual bool endRender();

		// invalide
		virtual bool invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil);

		// on resize
		virtual void onResize( ui32 width, ui32 height );

	private:
		// create texutre 2d
		virtual bool createTexture2D();

	private:
		GLuint m_fbo;
	};
}

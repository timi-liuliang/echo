#pragma once

#include "image/PixelFormat.h"
#include "Texture.h"

namespace Echo
{
	class RenderView
	{
	public:
		RenderView(ui32 width, ui32 height, PixelFormat pixelFormat);
		virtual ~RenderView();

		// get pixel format
		virtual PixelFormat	getPixelFormat() const { return m_format; }

        // create
        virtual bool create() { return false; }

        // begin render
        virtual bool beginRender(bool clearColor, const Color& bgColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) { return false; }

        // clear
        virtual void clear(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) {}

        // end render
        virtual bool endRender() { return false; }

	protected:
        ui32				m_width;
        ui32				m_height;
		PixelFormat			m_format;
        Texture*			m_bindTexture = nullptr;
	};
}

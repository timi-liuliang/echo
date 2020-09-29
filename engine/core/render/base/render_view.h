#pragma once

#include "image/pixel_format.h"
#include "texture.h"

namespace Echo
{
	class RenderView
	{
	public:
		virtual ~RenderView();

        // width & height
        ui32 getWidth() const { return m_width; }
        ui32 getHeight() const { return m_height; }

		// pixel format
		PixelFormat	getPixelFormat() const { return m_format; }

        // texture
        Texture* getTexture() { return m_bindTexture; }

        // on resize
        virtual void onSize(ui32 width, ui32 height)=0;

    protected:
        RenderView(ui32 width, ui32 height, PixelFormat pixelFormat);

	protected:
        ui32				m_width;
        ui32				m_height;
		PixelFormat			m_format;
        Texture*			m_bindTexture = nullptr;
	};
}

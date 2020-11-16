#pragma once

#include "image/pixel_format.h"
#include "texture.h"

namespace Echo
{
	class TextureRender : public Texture
	{
		ECHO_RES(TextureRender, Texture, ".rt", TextureRender::create, Res::load)

	public:
		TextureRender();
		TextureRender(const String& name);
		virtual ~TextureRender();

		// create fun
		static Res* create();

        // width & height
        ui32 getWidth() const { return m_width; }
        ui32 getHeight() const { return m_height; }

		// pixel format
		PixelFormat	getPixelFormat() const { return m_format; }

        // on resize
		virtual void onSize(ui32 width, ui32 height) {}

	protected:
        ui32				m_width;
        ui32				m_height;
		PixelFormat			m_format;
	};
}

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

		// type
		virtual TexType getType() const override { return TT_Render; }

		// override width|height
		virtual void setWidth(ui32 width) override;
		virtual void setHeight(ui32 height) override;

		// color
		const Color& getClearColor() const { return m_clearColor; }
		void setClearColor(const Color& color);

        // on resize
		virtual void onSize(ui32 width, ui32 height);

	public:
		// update texture by rect
		virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size) { return false; }
		virtual bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size) { return false; }

	protected:
		// unload
		virtual bool unload() { return false; }

	protected:
		Color			m_clearColor = Color::BLACK;
	};
	typedef ResRef<TextureRender> TextureRenderPtr;
}

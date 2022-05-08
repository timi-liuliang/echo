#pragma once

#include "engine/core/render/base/image/pixel_format.h"
#include "texture.h"
#include "engine/core/util/magic_enum.hpp"

namespace Echo
{
	class TextureRenderTarget2D : public Texture
	{
		ECHO_RES(TextureRenderTarget2D, Texture, ".rt", TextureRenderTarget2D::create, Res::load)

	public:
		TextureRenderTarget2D();
		TextureRenderTarget2D(const String& name);
		virtual ~TextureRenderTarget2D();

		// create fun
		static Res* create();

		// type
		virtual TexType getType() const override { return TT_Render; }

		// override width|height
		virtual void setWidth(ui32 width) override;
		virtual void setHeight(ui32 height) override;

		// set format
		const StringOption getPixelFormatName();
		void setPixelFormatName(const StringOption& option);

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
		Color			m_clearColor	  = Color::BLACK;
	};
	typedef ResRef<TextureRenderTarget2D> TextureRenderTarget2DPtr;
}

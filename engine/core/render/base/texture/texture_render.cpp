#include "texture_render.h"
#include "engine/core/render/base/renderer.h"

namespace Echo
{
	TextureRender::TextureRender()
	{

	}

	TextureRender::TextureRender(const String& name)
		: Texture(name)
	{

	}

	TextureRender::~TextureRender()
	{
	}

	void TextureRender::bindMethods()
	{
		CLASS_BIND_METHOD(TextureRender, getClearColor);
		CLASS_BIND_METHOD(TextureRender, setClearColor);
		CLASS_BIND_METHOD(TextureRender, getPixelFormatName);
		CLASS_BIND_METHOD(TextureRender, setPixelFormatName);

		CLASS_REGISTER_PROPERTY(TextureRender, "ClearColor", Variant::Type::Color, getClearColor, setClearColor);
		CLASS_REGISTER_PROPERTY(TextureRender, "Format", Variant::Type::StringOption, getPixelFormatName, setPixelFormatName);
	}

	Res* TextureRender::create()
	{
		static i32 idx = 0; idx++;
		return Renderer::instance()->createTextureRender(StringUtil::Format("TXTURE_RENDER_%d", idx));
	}

	void TextureRender::setWidth(ui32 width)
	{
		if (m_width != width)
		{
			m_width = width;

			unload();
		}
	}

	void TextureRender::setHeight(ui32 height)
	{
		if (m_height != height)
		{
			m_height = height;

			unload();
		}
	}

	void TextureRender::setPixelFormatName(const StringOption& option)
	{
		if (m_pixelFormatName.getValue() != option.getValue() && m_pixelFormatName.setValue(option.getValue()))
		{
			unload();
		}

		m_pixFmt = magic_enum::enum_cast<PixelFormat>(m_pixelFormatName.getValue()).value_or(PixelFormat::PF_UNKNOWN);
	}

	void TextureRender::setClearColor(const Color& color)
	{ 
		if (m_clearColor != color)
		{
			m_clearColor = color;
			unload();
		}
	}

	void TextureRender::onSize(ui32 width, ui32 height)
	{
		setWidth(width);
		setHeight(height);
	}
}
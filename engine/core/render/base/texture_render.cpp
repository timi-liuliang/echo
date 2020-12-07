#include "texture_render.h"
#include "renderer.h"

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
		CLASS_BIND_METHOD(TextureRender, getClearColor, DEF_METHOD("getClearColor"));
		CLASS_BIND_METHOD(TextureRender, setClearColor, DEF_METHOD("setClearColor"));
		CLASS_BIND_METHOD(TextureRender, getPixelFormatName, DEF_METHOD("getPixelFormatName"));
		CLASS_BIND_METHOD(TextureRender, setPixelFormatName, DEF_METHOD("setPixelFormatName"));

		CLASS_REGISTER_PROPERTY(TextureRender, "ClearColor", Variant::Type::Color, "getClearColor", "setClearColor");
		CLASS_REGISTER_PROPERTY(TextureRender, "Format", Variant::Type::StringOption, "getPixelFormatName", "setPixelFormatName");
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
			if (option.getValue() == "PF_RGBA8_UNORM")
				m_pixFmt = PixelFormat::PF_RGBA8_UNORM;
			else
				m_pixFmt = PixelFormat::PF_D24_UNORM_S8_UINT;

			unload();
		}
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
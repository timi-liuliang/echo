#include "texture_render_target_2d.h"
#include "engine/core/render/base/renderer.h"

namespace Echo
{
	TextureRenderTarget2D::TextureRenderTarget2D()
	{

	}

	TextureRenderTarget2D::TextureRenderTarget2D(const String& name)
		: Texture(name)
	{

	}

	TextureRenderTarget2D::~TextureRenderTarget2D()
	{
	}

	void TextureRenderTarget2D::bindMethods()
	{
		CLASS_BIND_METHOD(TextureRenderTarget2D, getClearColor);
		CLASS_BIND_METHOD(TextureRenderTarget2D, setClearColor);
		CLASS_BIND_METHOD(TextureRenderTarget2D, getPixelFormatName);
		CLASS_BIND_METHOD(TextureRenderTarget2D, setPixelFormatName);
		CLASS_BIND_METHOD(TextureRenderTarget2D, getOnSizeType);
		CLASS_BIND_METHOD(TextureRenderTarget2D, setOnSizeType);

		CLASS_REGISTER_PROPERTY(TextureRenderTarget2D, "ClearColor", Variant::Type::Color, getClearColor, setClearColor);
		CLASS_REGISTER_PROPERTY(TextureRenderTarget2D, "Format", Variant::Type::StringOption, getPixelFormatName, setPixelFormatName);
		CLASS_REGISTER_PROPERTY(TextureRenderTarget2D, "OnSize", Variant::Type::StringOption, getOnSizeType, setOnSizeType);
	}

	Res* TextureRenderTarget2D::create()
	{
		static i32 idx = 0; idx++;
		return Renderer::instance()->createTextureRender(StringUtil::Format("TXTURE_RENDER_%d", idx));
	}

	void TextureRenderTarget2D::setWidth(ui32 width)
	{
		if (m_width != width)
		{
			m_width = width;

			unload();
		}
	}

	void TextureRenderTarget2D::setHeight(ui32 height)
	{
		if (m_height != height)
		{
			m_height = height;

			unload();
		}
	}

	const StringOption TextureRenderTarget2D::getPixelFormatName()
	{
		return StringOption::fromEnum(m_pixFmt);
	}

	void TextureRenderTarget2D::setPixelFormatName(const StringOption& option)
	{
		PixelFormat format = option.toEnum(PixelFormat::PF_RGBA8_UNORM);
		if (m_pixFmt != format)
		{
			unload();
			m_pixFmt = format;
		}
	}

	const StringOption TextureRenderTarget2D::getOnSizeType()
	{
		return StringOption::fromEnum(m_onSizeType);
	}

	void TextureRenderTarget2D::setOnSizeType(const StringOption& option)
	{
		OnSizeType onSizeType = option.toEnum(OnSizeType::Dynamic);
		if (m_onSizeType != onSizeType)
		{
			m_onSizeType = onSizeType;
		}
	}

	void TextureRenderTarget2D::setClearColor(const Color& color)
	{ 
		if (m_clearColor != color)
		{
			m_clearColor = color;
			unload();
		}
	}

	void TextureRenderTarget2D::onSize(ui32 width, ui32 height)
	{
		if (m_onSizeType != OnSizeType::Static)
		{
			setWidth(width);
			setHeight(height);
		}
		else
		{
			unload();
		}
	}
}
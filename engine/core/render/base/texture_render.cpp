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

		CLASS_REGISTER_PROPERTY(TextureRender, "ClearColor", Variant::Type::Color, "getClearColor", "setClearColor");
	}

	Res* TextureRender::create()
	{
		static i32 idx = 0; idx++;
		return Renderer::instance()->createTextureRender(StringUtil::Format("TXTURE_RENDER_%d", idx));
	}

	void TextureRender::setWidth(ui32 width)
	{
		m_width = width;

		unload();
	}

	void TextureRender::setHeight(ui32 height)
	{
		m_height = height;

		unload();
	}

	void TextureRender::setClearColor(const Color& color)
	{ 
		m_clearColor = color; 
		unload(); 
	}

	void TextureRender::onSize(ui32 width, ui32 height)
	{
		setWidth(width);
		setHeight(height);
	}
}
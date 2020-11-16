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

	}

	Res* TextureRender::create()
	{
		return Renderer::instance()->createTextureRender("");
	}
}
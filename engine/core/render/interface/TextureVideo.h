#pragma once

#include "Texture.h"

namespace Echo
{
	class TextureVideo : public Texture
	{
		ECHO_RES(TextureVideo, Res, ".mp4", Res::create<TextureVideo>, Res::load)

	public:
		TextureVideo();
		virtual ~TextureVideo();

	private:
		ResourcePath	m_cubeDiffuseTextureNames[6];
	};
}
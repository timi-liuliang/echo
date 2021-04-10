#pragma once

#include "engine/core/render/base/texture.h"

namespace Echo
{
	class TextureVideo : public Texture
	{
		ECHO_RES(TextureVideo, Res, ".mp4", Res::create<TextureVideo>, Res::load)

	public:
		TextureVideo();
		virtual ~TextureVideo();

	private:
		ResourcePath	m_videoPath;
	};
}

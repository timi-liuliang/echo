#pragma once

#include "Texture.h"

namespace Echo
{
	class TextureCube : public Texture
	{
		ECHO_RES(TextureCube, Res, ".cubetex", Res::create<TextureCube>, Res::load)

	public:
		TextureCube();
		virtual ~TextureCube();

	private:
		ResourcePath	m_cubeDiffuseTextureNames[6];
	};
}
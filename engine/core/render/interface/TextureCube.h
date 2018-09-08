#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
	class TextureCube : public Res
	{
		ECHO_RES(TextureCube, Res, ".cubetex", Res::create<TextureCube>, Res::load)

	public:
		TextureCube();
		virtual ~TextureCube();

	private:
		ResourcePath	m_cubeDiffuseTextureNames[6];
	};
}
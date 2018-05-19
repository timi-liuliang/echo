#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
	class TextureCubeRes : public Res
	{
	public:

	private:
		TextureCubeRes(const ResourcePath& path);
		virtual ~TextureCubeRes();

	private:
		ResourcePath	m_cubeDiffuseTextureNames[6];
	};
}
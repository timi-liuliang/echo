#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
	class TextureCubeRes : public Res
	{
		ECHO_CLASS(TextureCubeRes, Res)

	public:
		TextureCubeRes();
		TextureCubeRes(const ResourcePath& path);
		virtual ~TextureCubeRes();

		// bind methods to script
		static void bindMethods() {}

	private:
		ResourcePath	m_cubeDiffuseTextureNames[6];
	};
}
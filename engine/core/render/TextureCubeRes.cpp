#include "TextureCubeRes.h"

namespace Echo
{
	TextureCubeRes::TextureCubeRes()
		: Res(ResourcePath(""))
	{

	}

	TextureCubeRes::TextureCubeRes(const ResourcePath& path)
		: Res( path)
	{

	}

	TextureCubeRes::~TextureCubeRes()
	{

	}
}
#include "cube_light_custom.h"

namespace Echo
{
	CubeLightCustom::CubeLightCustom()
		: m_usage("Specular", {"Diffuse", "Specular", "Both"})
		, m_cubeMap("", ".cube")
	{

	}

	CubeLightCustom::~CubeLightCustom()
	{

	}

	void CubeLightCustom::bindMethods()
	{
		CLASS_BIND_METHOD(CubeLightCustom, getUsage, DEF_METHOD("getUsage"));
		CLASS_BIND_METHOD(CubeLightCustom, setUsage, DEF_METHOD("setUsage"));
		CLASS_BIND_METHOD(CubeLightCustom, getCubeMap, DEF_METHOD("getCubeMap"));
		CLASS_BIND_METHOD(CubeLightCustom, setCubeMap, DEF_METHOD("setCubeMap"));

		CLASS_REGISTER_PROPERTY(CubeLightCustom, "Usage", Variant::Type::StringOption, "getUsage", "setUsage");
		CLASS_REGISTER_PROPERTY(CubeLightCustom, "CubeMap", Variant::Type::ResourcePath, "getCubeMap", "setCubeMap");
	}

	void CubeLightCustom::setCubeMap(const ResourcePath& cubemap)
	{
		if (m_cubeMap.setPath(cubemap.getPath()))
		{

		}
	}
}
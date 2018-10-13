#include "cube_light.h"

namespace Echo
{
	CubeLightStatic::CubeLightStatic()
		: m_type("Specular", {"Diffuse", "Specular", "Both"})
		, m_cubeMap("", ".cube")
	{

	}

	CubeLightStatic::~CubeLightStatic()
	{

	}

	void CubeLightStatic::bindMethods()
	{
		CLASS_BIND_METHOD(CubeLightStatic, getType, DEF_METHOD("getType"));
		CLASS_BIND_METHOD(CubeLightStatic, setType, DEF_METHOD("setType"));
		CLASS_BIND_METHOD(CubeLightStatic, getCubeMap, DEF_METHOD("getCubeMap"));
		CLASS_BIND_METHOD(CubeLightStatic, setCubeMap, DEF_METHOD("setCubeMap"));

		CLASS_REGISTER_PROPERTY(CubeLightStatic, "Type", Variant::Type::StringOption, "getType", "setType");
		CLASS_REGISTER_PROPERTY(CubeLightStatic, "CubeMap", Variant::Type::ResourcePath, "getCubeMap", "setCubeMap");
	}

	void CubeLightStatic::setCubeMap(const ResourcePath& cubemap)
	{
		if (m_cubeMap.setPath(cubemap.getPath()))
		{

		}
	}
}
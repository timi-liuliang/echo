#include "pcg_terrain.h"

namespace Echo
{
	PCGTerrain::PCGTerrain()
	{

	}

	PCGTerrain::~PCGTerrain()
	{

	}

	void PCGTerrain::bindMethods()
	{
		CLASS_BIND_METHOD(PCGTerrain, getTerrainPath, DEF_METHOD("getTerrainPath"));
		CLASS_BIND_METHOD(PCGTerrain, setTerrainPath, DEF_METHOD("setTerrainPath"));
		CLASS_BIND_METHOD(PCGTerrain, isAutoCreate, DEF_METHOD("isAutoCreate"));
		CLASS_BIND_METHOD(PCGTerrain, setAutoCreate, DEF_METHOD("setAutoCreate"));

		CLASS_REGISTER_PROPERTY(PCGTerrain, "Terrain", Variant::Type::NodePath, "getTerrainPath", "setTerrainPath");
		CLASS_REGISTER_PROPERTY(PCGTerrain, "AutoCreate", Variant::Type::Bool, "isAutoCreate", "setAutoCreate");
	}

	void PCGTerrain::setTerrainPath(const NodePath& terrainPath)
	{
		m_terrainPath.setPath(terrainPath.getPath());
	}

	void PCGTerrain::run()
	{

	}
}
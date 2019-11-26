#include "terrain_module.h"
#include "terrain.h"
#include "editor/terrain_editor.h"

namespace Echo
{
    TerrainModule::TerrainModule()
    {
    }

	TerrainModule::~TerrainModule()
	{

	}

	TerrainModule* TerrainModule::instance()
	{
		static TerrainModule* inst = EchoNew(TerrainModule);
		return inst;
	}

	void TerrainModule::bindMethods()
	{

	}
    
    void TerrainModule::registerTypes()
    {
        Class::registerType<Terrain>();
        
        REGISTER_OBJECT_EDITOR(Terrain, TerrainEditor)
    }
}

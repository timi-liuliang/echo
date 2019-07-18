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
    
    void TerrainModule::registerTypes()
    {
        Class::registerType<Terrain>();
        
        REGISTER_OBJECT_EDITOR(Terrain, TerrainEditor)
    }
}

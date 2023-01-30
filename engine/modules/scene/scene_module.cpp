#include "scene_module.h"
#include "terrain/terrain.h"
#include "terrain/editor/terrain_editor.h"
#include "tilemap/tilemap.h"
#include "tilemap/editor/tilemap_editor.h"

namespace Echo
{
	DECLARE_MODULE(SceneModule, __FILE__)

    SceneModule::SceneModule()
    {
    }

	SceneModule::~SceneModule()
	{

	}

	SceneModule* SceneModule::instance()
	{
		static SceneModule* inst = EchoNew(SceneModule);
		return inst;
	}

	void SceneModule::bindMethods()
	{

	}
    
    void SceneModule::registerTypes()
    {
        Class::registerType<Terrain>();
        Class::registerType<TileMap>();
        
        CLASS_REGISTER_EDITOR(Terrain, TerrainEditor)
        CLASS_REGISTER_EDITOR(TileMap, TileMapEditor)
    }
}

#include "scene_module.h"
#include "terrain/terrain.h"
#include "terrain/editor/terrain_editor.h"
#include "tilemap/tilemap.h"
#include "tilemap/editor/tilemap_editor.h"
#include "planet/planet.h"

namespace Echo
{
	DECLARE_MODULE(SceneModule)

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
		Class::registerType<Planet>();
        
        CLASS_REGISTER_EDITOR(Terrain, TerrainEditor)
        CLASS_REGISTER_EDITOR(TileMap, TileMapEditor)
    }
}

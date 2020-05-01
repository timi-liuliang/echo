#include "terrain_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TerrainEditor::TerrainEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    TerrainEditor::~TerrainEditor()
    {
    }
    
    ImagePtr TerrainEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() +  "engine/modules/scene/terrain/editor/icon/terrain.png");
    }
#endif
}


#include "terrain_editor.h"
#include "engine/core/editor/editor.h"

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
    
    // get camera2d icon, used for editor
    const char* TerrainEditor::getEditorIcon() const
    {
        return "engine/modules/ui/editor/icon/terrain.png";
    }
#endif
}


#include "tilemap_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TileMapEditor::TileMapEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    TileMapEditor::~TileMapEditor()
    {
    }
    
    // get camera2d icon, used for editor
    const char* TileMapEditor::getEditorIcon() const
    {
        return "engine/modules/scene/tilemap/editor/icon/tilemap.png";
    }
#endif
}


#include "texture_atlas_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TextureAtlasEditor::TextureAtlasEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    TextureAtlasEditor::~TextureAtlasEditor()
    {
    }
    
    // get camera2d icon, used for editor
    const char* TextureAtlasEditor::getEditorIcon() const
    {
        return "engine/core/render/base/editor/icon/texture_atlas.png";
    }
#endif
}


#include "procedural_texture_editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    ProceduralTextureEditor::ProceduralTextureEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    ProceduralTextureEditor::~ProceduralTextureEditor()
    {
    }

    ImagePtr ProceduralTextureEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/procedural/editor/icon/procedural_texture.png");
    }

    void ProceduralTextureEditor::onEditorSelectThisNode()
    {
    }

    void ProceduralTextureEditor::editor_update_self()
    {
    }
#endif
}


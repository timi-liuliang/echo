#include "sprite_editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    SpriteEditor::SpriteEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    SpriteEditor::~SpriteEditor()
    {
    }

    ImagePtr SpriteEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/effect/editor/icon/sprite.png");
    }

    void SpriteEditor::onEditorSelectThisNode()
    {
    }

    void SpriteEditor::editor_update_self()
    {
    }
#endif
}


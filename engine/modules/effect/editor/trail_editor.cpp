#include "trail_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TrailEditor::TrailEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    TrailEditor::~TrailEditor()
    {
    }

    ImagePtr TrailEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/effect/editor/icon/trail.png");
    }

    void TrailEditor::onEditorSelectThisNode()
    {
    }

    void TrailEditor::editor_update_self()
    {
    }
#endif
}


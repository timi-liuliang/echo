#include "mesh_render_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    MeshRenderEditor::MeshRenderEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    MeshRenderEditor::~MeshRenderEditor()
    {
    }

    ImagePtr MeshRenderEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/model/editor/icon/mesh_render.png");
    }

    void MeshRenderEditor::onEditorSelectThisNode()
    {
    }

    void MeshRenderEditor::editor_update_self()
    {
    }
#endif
}


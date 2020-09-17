#include "procedural_geometry_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    ProceduralGeometryEditor::ProceduralGeometryEditor(Object* object)
    : ObjectEditor(object)
    {
        m_panel = EchoNew(ProceduralGeometryPanel(object));
    }
    
    ProceduralGeometryEditor::~ProceduralGeometryEditor()
    {
        EchoSafeDelete(m_panel, ProceduralGeometryPanel);
    }

    ImagePtr ProceduralGeometryEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/procedural/editor/icon/procedural_geometry.png");
    }

    void ProceduralGeometryEditor::onEditorSelectThisNode()
    {
        Editor::instance()->showCenterPanel(m_panel->getUi(), 0.3f);
    }

    void ProceduralGeometryEditor::editor_update_self()
    {
        if (m_panel)
        {
            m_panel->update();
        }
    }
#endif
}


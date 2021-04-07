#include "pcg_flow_graph_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    PCGFlowGraphEditor::PCGFlowGraphEditor(Object* object)
    : ObjectEditor(object)
    {
        m_panel = EchoNew(PCGFlowGraphPanel(object));
    }
    
    PCGFlowGraphEditor::~PCGFlowGraphEditor()
    {
        EchoSafeDelete(m_panel, PCGFlowGraphPanel);
    }

    ImagePtr PCGFlowGraphEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/pcg/editor/icon/pcg_flow_graph.png");
    }

    void PCGFlowGraphEditor::onEditorSelectThisNode()
    {
        Editor::instance()->showCenterPanel(m_panel, 0.3f);
    }

    void PCGFlowGraphEditor::editor_update_self()
    {
        if (m_panel)
        {
            m_panel->update();
        }
    }
#endif
}


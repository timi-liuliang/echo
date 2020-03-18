#include "render_pipeline_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    RenderPipelineEditor::RenderPipelineEditor(Object* object)
    : ObjectEditor(object)
    {
        m_panel = EchoNew(RenderpipelinePanel(object));
    }
    
    RenderPipelineEditor::~RenderPipelineEditor()
    {
        EchoSafeDelete(m_panel, RenderpipelinePanel);
    }
    
    const char* RenderPipelineEditor::getEditorIcon() const
    {
        return "engine/core/render/base/editor/icon/render_pipeline.png";
    }

	void RenderPipelineEditor::onEditorSelectThisNode()
	{
		Editor::instance()->showBottomPanel(m_panel);
	}

	void RenderPipelineEditor::editor_update_self()
	{
		if (m_panel)
		{
			m_panel->update();
		}
	}
#endif
}


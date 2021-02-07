#include "render_pipeline_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

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

	ImagePtr RenderPipelineEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/core/render/base/pipeline/editor/icon/render_pipeline.png");
	}

	void RenderPipelineEditor::onEditorSelectThisNode()
	{
		Editor::instance()->showBottomPanel(m_panel);
	}

	void RenderPipelineEditor::postEditorCreateObject()
	{
		RenderPipeline* pipeline = ECHO_DOWN_CAST<RenderPipeline*>(m_object);
		if (pipeline)
		{
			pipeline->setSrc(RenderPipeline::Default);
		}
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


#include "render_pipeline_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

#ifdef ECHO_EDITOR_MODE

const char* defaultPipelineTemplate =
R"(<?xml version="1.0"?>
<pipeline>
	<stage class="RenderStage" Name="GBuffer">
		<property name="FrameBuffer">
			<obj class="FrameBufferWindow" IsClearColor="true" IsClearDepth="true" />
		</property>
		<queue class="RenderQueue" Name="Opaque" Enable="true" Sort="false" />
		<queue class="RenderQueue" Name="Transparent" Enable="true" Sort="true" />
	</stage>
</pipeline>
)";

namespace Echo
{
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
			pipeline->setSrc(defaultPipelineTemplate);
		}
	}

	void RenderPipelineEditor::editor_update_self()
	{
		if (m_panel)
		{
			m_panel->update();
		}
	}
}

#endif
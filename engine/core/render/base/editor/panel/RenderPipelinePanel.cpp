#include "RenderPipelinePanel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	RenderpipelinePanel::RenderpipelinePanel(Object* obj)
	{
		m_pipeline = ECHO_DOWN_CAST<RenderPipeline*>(obj);

		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/panel/RenderPipelinePanel.ui");

		QWidget* splitter = EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			EditorApi.qSplitterSetStretchFactor(splitter, 0, 0);
			EditorApi.qSplitterSetStretchFactor(splitter, 1, 1);
		}

		// Tool button icons
		EditorApi.qToolButtonSetIcon(EditorApi.qFindChild(m_ui, "m_new"), "engine/core/render/base/editor/icon/import.png");

		// connect signal slots
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_new"), QSIGNAL(clicked()), this, createMethodBind(&RenderpipelinePanel::onNew));
	}

	void RenderpipelinePanel::update()
	{
	}

	void RenderpipelinePanel::onNew()
	{
		if (!m_importMenu)
		{
			m_importMenu = EditorApi.qMenuNew(m_ui);

			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionNewCamera"));
			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionNewImageFilter"));

			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionNewCamera"), QSIGNAL(triggered()), this, createMethodBind(&RenderpipelinePanel::onNewCamera));
			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionNewImageFilter"), QSIGNAL(triggered()), this, createMethodBind(&RenderpipelinePanel::onNewImageFilter));
		}

		EditorApi.qMenuExec(m_importMenu);
	}

	void RenderpipelinePanel::onNewCamera()
	{

	}

	void RenderpipelinePanel::onNewImageFilter()
	{

	}
#endif
}

#include "RenderPipelinePanel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	RenderpipelinePanel::RenderpipelinePanel(Object* obj)
	{
		m_pipeline = ECHO_DOWN_CAST<RenderPipeline*>(obj);

		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/pipeline/RenderPipelinePanel.ui");

		QSplitter* splitter = (QSplitter*)EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			splitter->setStretchFactor(0, 0);
			splitter->setStretchFactor(1, 1);
		}

		// Tool button icons
		((QToolButton*)EditorApi.qFindChild(m_ui, "m_new"))->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/import.png").c_str()));

		// connect signal slots
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_new"), QSIGNAL(clicked()), this, createMethodBind(&RenderpipelinePanel::onNew));

		// create QGraphicsScene
		m_graphicsView = m_ui->findChild<QGraphicsView*>("m_graphicsView");
		m_graphicsScene = EditorApi.qGraphicsSceneNew();
		m_graphicsView->setScene(m_graphicsScene);

		// top line
		float topSpace = 15.f;
		float leftSpace = 15.f;
		float defaultStageNodeWidth = 190.f;
		m_borderTopLine = m_graphicsScene->addLine(QLineF(-leftSpace - defaultStageNodeWidth * 0.5f, -topSpace, 100, -topSpace), QPen(QColor(0, 0, 0, 0)));

		// Fixed viewport top-left corner
		m_graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	}

	void RenderpipelinePanel::update()
	{
		drawStages();
		drawRenderQueues();
	}

	void RenderpipelinePanel::onNew()
	{
		if (!m_importMenu)
		{
			m_importMenu = EchoNew(QMenu(m_ui));

			m_importMenu->addAction( EditorApi.qFindChildAction(m_ui, "m_actionNewCamera"));
			m_importMenu->addAction( EditorApi.qFindChildAction(m_ui, "m_actionNewImageFilter"));

			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionNewCamera"), QSIGNAL(triggered()), this, createMethodBind(&RenderpipelinePanel::onNewCamera));
			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionNewImageFilter"), QSIGNAL(triggered()), this, createMethodBind(&RenderpipelinePanel::onNewImageFilter));
		}

		m_importMenu->exec(QCursor::pos());
	}

	void RenderpipelinePanel::onNewCamera()
	{

	}

	void RenderpipelinePanel::onNewImageFilter()
	{

	}

	void RenderpipelinePanel::drawStages()
	{
		const vector<RenderStage*>::type& stages = m_pipeline->getRenderStages();
		while (m_stageNodePainters.size() > stages.size())
		{
			EchoSafeDelete(m_stageNodePainters.back(), StatgeNodePainter);
			m_stageNodePainters.pop_back();
		}

		if (m_stageNodePainters.size() < stages.size())
		{
			for (size_t i = m_stageNodePainters.size(); i < stages.size(); ++i)
				m_stageNodePainters.emplace_back(EchoNew(Pipeline::StatgeNodePainter(m_graphicsView, m_graphicsScene, stages[i])));
		}

		for (size_t i = 0; i < stages.size(); i++)
		{
			if (!m_stageNodePainters[i] || m_stageNodePainters[i]->m_stage != stages[i])
			{
				EchoSafeDelete(m_stageNodePainters[i], StatgeNodePainter);
				m_stageNodePainters[i] = EchoNew(Pipeline::StatgeNodePainter(m_graphicsView, m_graphicsScene, stages[i]));
			}
		}

		for (size_t i = 0; i < stages.size(); i++)
		{
			m_stageNodePainters[i]->update(i, i + 1 == stages.size());
		}
	}

	void RenderpipelinePanel::drawRenderQueues()
	{
		vector<Vector2>::type		poses;
		vector<IRenderQueue*>::type renderqueues;
		for (size_t x=0; x<m_pipeline->getRenderStages().size(); x++)
		{
			RenderStage* stage = m_pipeline->getRenderStages()[x];
			for (size_t y = 0; y < stage->getRenderQueues().size(); y++)
			{
				renderqueues.push_back(stage->getRenderQueues()[y]);
				poses.push_back(Vector2(x, y));
			}
		}

		while (m_renderQueueNodePainters.size() > renderqueues.size())
		{
			EchoSafeDelete(m_renderQueueNodePainters.back(), RenderQueueNodePainter);
			m_renderQueueNodePainters.pop_back();
		}

		if (m_renderQueueNodePainters.size() < renderqueues.size())
		{
			for (size_t i = m_renderQueueNodePainters.size(); i < renderqueues.size(); ++i)
				m_renderQueueNodePainters.emplace_back(EchoNew(Pipeline::RenderQueueNodePainter(m_graphicsView, m_graphicsScene, renderqueues[i])));
		}

		for (size_t i = 0; i < renderqueues.size(); i++)
		{
			if (!m_renderQueueNodePainters[i] || m_renderQueueNodePainters[i]->m_renderQueue != renderqueues[i])
			{
				EchoSafeDelete(m_renderQueueNodePainters[i], RenderQueueNodePainter);
				m_renderQueueNodePainters[i] = EchoNew(Pipeline::RenderQueueNodePainter(m_graphicsView, m_graphicsScene, renderqueues[i]));
			}
		}

		for (size_t i = 0; i < renderqueues.size(); i++)
		{
			m_renderQueueNodePainters[i]->update(poses[i].x, poses[i].y);
		}
	}
#endif
}

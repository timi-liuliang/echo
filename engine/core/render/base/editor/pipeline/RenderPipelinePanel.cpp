#include "RenderPipelinePanel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/main/Engine.h"
#include "engine/core/main/GameSettings.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	RenderpipelinePanel::RenderpipelinePanel(Object* obj)
	{
		m_pipeline = ECHO_DOWN_CAST<RenderPipeline*>(obj);

		m_ui = (QDockWidget*)EditorApi.qLoadUi("engine/core/render/base/editor/pipeline/RenderPipelinePanel.ui");

		QSplitter* splitter = (QSplitter*)EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			splitter->setStretchFactor(0, 0);
			splitter->setStretchFactor(1, 1);
		}

		// Tool button icons
		m_applyButton = m_ui->findChild<QToolButton*>("m_apply");
		m_playIcon = QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/play.png").c_str());
		m_stopIcon = QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/stop.png").c_str());
		m_applyButton->setIcon(m_playIcon);

		// connect signal slots
		EditorApi.qConnectWidget(m_applyButton, QSIGNAL(clicked()), this, createMethodBind(&RenderpipelinePanel::onApply));

		// create QGraphicsScene
		m_graphicsView = m_ui->findChild<QGraphicsView*>("m_graphicsView");
		m_graphicsScene = new Pipeline::QGraphicsSceneEx();
		m_graphicsView->setScene(m_graphicsScene);
		m_graphicsView->setAttribute(Qt::WA_AlwaysShowToolTips);

		// event
		m_graphicsScene->setMousePressEventCb([this]()
		{
			EditorApi.showObjectProperty(m_pipeline);
		});

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
		drawStageAddButtons();

		updateApplyButtonIcon();
	}

	void RenderpipelinePanel::onApply()
	{
		Echo::String resPath = m_pipeline->getPath();
		Echo::String currentPath = Echo::GameSettings::instance()->getRenderPipeline().getPath();
		if (resPath != currentPath)
		{
			Echo::GameSettings::instance()->setRenderPipeline(resPath);
		}
		else
		{
			Echo::GameSettings::instance()->setRenderPipeline(Echo::StringUtil::BLANK);
		}
	}

	void RenderpipelinePanel::drawStages()
	{
		vector<RenderStage*>::type stages = m_pipeline->getRenderStages();
		while (m_stageNodePainters.size() > stages.size())
		{
			EchoSafeDelete(m_stageNodePainters.back(), StageNodePainter);
			m_stageNodePainters.pop_back();
		}

		if (m_stageNodePainters.size() < stages.size())
		{
			for (size_t i = m_stageNodePainters.size(); i < stages.size(); ++i)
				m_stageNodePainters.emplace_back(EchoNew(Pipeline::StageNodePainter(m_graphicsView, m_graphicsScene, stages[i])));
		}

		for (size_t i = 0; i < stages.size(); i++)
		{
			if (!m_stageNodePainters[i] || m_stageNodePainters[i]->m_stage != stages[i])
			{
				EchoSafeDelete(m_stageNodePainters[i], StageNodePainter);
				m_stageNodePainters[i] = EchoNew(Pipeline::StageNodePainter(m_graphicsView, m_graphicsScene, stages[i]));
			}
		}

		for (size_t i = 0; i < stages.size(); i++)
		{
			m_stageNodePainters[i]->update(i, i + 1 == stages.size());
		}
	}

	void RenderpipelinePanel::drawStageAddButtons()
	{
		Echo::i32 count = m_pipeline->getRenderStages().size()+1;
		while (m_stageAddButtons.size() > count)
		{
			EchoSafeDelete(m_stageAddButtons.back(), StatgeAddButton);
			m_stageAddButtons.pop_back();
		}

		if (m_stageAddButtons.size() <count)
		{
			for (size_t i = m_stageAddButtons.size(); i < count; ++i)
				m_stageAddButtons.emplace_back(EchoNew(Pipeline::StatgeAddButton(m_graphicsScene, m_pipeline)));
		}

		for (size_t i = 0; i < count; i++)
		{
			m_stageAddButtons[i]->update(i, i + 1 == count);
		}
	}

	void RenderpipelinePanel::updateApplyButtonIcon()
	{
		Echo::String resPath = m_pipeline->getPath();
		Echo::String currentPath = Echo::GameSettings::instance()->getRenderPipeline().getPath();
		if (resPath != currentPath)
		{
			m_applyButton->setIcon(m_playIcon);
			m_applyButton->setStatusTip("Start this render pipeline");
			m_applyButton->setToolTip("Start this render pipeline");
		}
		else
		{
			m_applyButton->setIcon(m_stopIcon);
			m_applyButton->setStatusTip("Stop this render pipeline");
			m_applyButton->setToolTip("Stop this render pipeline");
		}
	}

	void RenderpipelinePanel::save()
	{
		if (m_pipeline)
		{
			m_pipeline->save();
		}
	}

#endif
}

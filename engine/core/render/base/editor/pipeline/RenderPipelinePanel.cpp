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

		m_splitter = m_ui->findChild<QSplitter*>("m_splitter");
		if (m_splitter)
		{
			m_splitter->setStretchFactor(0, 1.5);
			m_splitter->setStretchFactor(1, 1);
		}

		// Tool button icons
		m_applyButton = m_ui->findChild<QToolButton*>("m_apply");
		m_playIcon = QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/play.png").c_str());
		m_stopIcon = QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/stop.png").c_str());
		m_applyButton->setIcon(m_playIcon);

		m_captureModeButton = m_ui->findChild<QToolButton*>("m_captureMode");
		m_captureEnableIcon = QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/capture-enable.png").c_str());
		m_captureDisableIcon = QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/capture-disable.png").c_str());
		m_captureModeButton->setIcon(m_captureDisableIcon);

		// connect signal slots
		EditorApi.qConnectWidget(m_applyButton, QSIGNAL(clicked()), this, createMethodBind(&RenderpipelinePanel::onApply));
		EditorApi.qConnectWidget(m_captureModeButton, QSIGNAL(clicked()), this, createMethodBind(&RenderpipelinePanel::onCaputeModeChanged));

		// create QGraphicsScene
		m_graphicsView = m_ui->findChild<QGraphicsView*>("m_graphicsView");
		m_graphicsScene = new Pipeline::QGraphicsSceneEx();
		m_graphicsView->setScene(m_graphicsScene);
		m_graphicsView->setAttribute(Qt::WA_AlwaysShowToolTips);

		m_graphicsViewFrameBuffer = m_ui->findChild<QGraphicsView*>("m_graphicsViewFrameBuffer");
		m_graphicsViewFrameBuffer->setVisible(false);

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

	void RenderpipelinePanel::onCaputeModeChanged()
	{
		m_captureEnable = !m_captureEnable;

		updateCaptureModeButtonIcon();
	}

	void RenderpipelinePanel::onCaptureFrame(FrameBuffer* fb)
	{
		if (m_captureEnable)
		{
			Echo::FrameBuffer::Pixels pixels;
			if (fb->readPixels(Echo::FrameBuffer::Attachment::Color0, pixels))
			{
				Echo::Image image(pixels.m_data.data(), pixels.m_width, pixels.m_height, 1, pixels.m_format);
				image.saveToFile("D:/test.bmp");
			}
		}
	}

	bool RenderpipelinePanel::isNeedUpdateStageNodePainters()
	{
		vector<RenderStage*>::type stages = m_pipeline->getRenderStages();
		if (m_stageNodePainters.size() != stages.size())
			return true;

		for (size_t i = 0; i < stages.size(); i++)
		{
			if (m_stageNodePainters[i]->m_stage != stages[i])
				return true;
		}

		return false;
	}

	void RenderpipelinePanel::drawStages()
	{
		vector<RenderStage*>::type stages = m_pipeline->getRenderStages();
		if (isNeedUpdateStageNodePainters())
		{
			Pipeline::StageNodePainters newStageNodePainters;
			for (RenderStage* stage : stages)
			{
				bool createNew = true;
				for (size_t i = 0; i < m_stageNodePainters.size(); i++)
				{
					if (m_stageNodePainters[i]->m_stage == stage)
					{
						newStageNodePainters.push_back(m_stageNodePainters[i]);
						m_stageNodePainters.erase(m_stageNodePainters.begin()+i);
						createNew = false;
						break;
					}
				}

				if (createNew)
				{
					Pipeline::StageNodePainter* painter = EchoNew(Pipeline::StageNodePainter(m_graphicsView, m_graphicsScene, stage));
					painter->setCaptureFrameCb([this](Echo::FrameBuffer* fb) 
					{
						onCaptureFrame(fb);
					});

					newStageNodePainters.emplace_back(painter);
				}
			}

			for (Pipeline::StageNodePainter* stagePainter : m_stageNodePainters)
			{
				if (stagePainter)
					EchoSafeDelete(stagePainter, StageNodePainter);
			}

			m_stageNodePainters = newStageNodePainters;
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
				m_stageAddButtons.emplace_back(EchoNew(Pipeline::StatgeAddButton(m_graphicsView, m_graphicsScene, m_pipeline)));
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
		if (resPath == currentPath)
		{
			m_applyButton->setIcon(m_playIcon);
			m_applyButton->setStatusTip("Render pipeline running");
			m_applyButton->setToolTip("Render pipeline running");
		}
		else
		{
			m_applyButton->setIcon(m_stopIcon);
			m_applyButton->setStatusTip("Render pipeline stopped");
			m_applyButton->setToolTip("Render pipeline stopped");
		}
	}

	void RenderpipelinePanel::updateCaptureModeButtonIcon()
	{
		if (m_captureEnable)
		{
			m_captureModeButton->setIcon(m_captureEnableIcon);
			m_captureModeButton->setStatusTip("Capture running");
			m_captureModeButton->setToolTip("Capture running");
		}
		else
		{
			m_captureModeButton->setIcon(m_captureDisableIcon);
			m_captureModeButton->setStatusTip("Capture stopped");
			m_captureModeButton->setToolTip("Capture stopped");
		}

		m_graphicsViewFrameBuffer->setVisible(m_captureEnable);
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

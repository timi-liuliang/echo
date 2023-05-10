#pragma once

#include "engine/core/util/Array.hpp"
#include "../../pipeline/render_pipeline.h"

#ifdef ECHO_EDITOR_MODE

#include <QSplitter>
#include "stage_node_painter.h"
#include "stage_add_button.h"
#include "renderqueue_node_painter.h"
#include "frame_buffer_painter.h"
#include "custom/qgraphics_scene_ex.h"
#include "ui_RenderPipelinePanel.h"

namespace Echo
{
	class RenderpipelinePanel : public QDockWidget, public Ui_RenderPipelinePanel
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		RenderpipelinePanel(Object* obj);

		// update
		void update();

	public:
		// import
		void onApply();
		void onCaputeModeChanged();

		// capture frame
		void onCaptureFrame(FrameBuffer* fb);

	protected:
		// need update
		bool isNeedUpdateStageNodePainters();

		// draw
		void drawStages();
		void drawStageAddButtons();

		// update icon
		void updateApplyButtonIcon();
		void updateCaptureModeButtonIcon();

	protected:
		// save
		//virtual void save() override;

	protected:
		RenderPipeline*						m_pipeline;
		QIcon								m_playIcon;
		QIcon								m_stopIcon;
		QIcon								m_captureEnableIcon;
		QIcon								m_captureDisableIcon;
		bool								m_captureEnable = false;
		Pipeline::QGraphicsSceneEx*			m_graphicsScene = nullptr;
		Pipeline::QGraphicsSceneEx*			m_graphicsSceneFrameBuffer = nullptr;
		QGraphicsLineItem*					m_borderTopLine = nullptr;
		Pipeline::StageNodePainters			m_stageNodePainters;
		Pipeline::StatgeAddButtons			m_stageAddButtons;
		Pipeline::FrameBufferPainter*		m_frameBufferPainter = nullptr;
	};
}

#endif
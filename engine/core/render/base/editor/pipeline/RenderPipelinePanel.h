#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "../../pipeline/render_pipeline.h"

#ifdef ECHO_EDITOR_MODE

#include <QSplitter>
#include "stage_node_painter.h"
#include "stage_add_button.h"
#include "renderqueue_node_painter.h"
#include "custom/qgraphics_scene_ex.h"

namespace Echo
{
	class RenderpipelinePanel : public EditorDockPanel
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
		virtual void save() override;

	protected:
		QSplitter*							m_splitter = nullptr;
		RenderPipeline*						m_pipeline;
		QIcon								m_playIcon;
		QIcon								m_stopIcon;
		QToolButton*						m_applyButton = nullptr;
		QIcon								m_captureEnableIcon;
		QIcon								m_captureDisableIcon;
		bool								m_captureEnable = false;
		QToolButton*						m_captureModeButton = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsView*						m_graphicsViewFrameBuffer = nullptr;
		Pipeline::QGraphicsSceneEx*			m_graphicsScene = nullptr;
		QGraphicsLineItem*					m_borderTopLine = nullptr;
		Pipeline::StageNodePainters			m_stageNodePainters;
		Pipeline::StatgeAddButtons			m_stageAddButtons;
	};
}

#endif
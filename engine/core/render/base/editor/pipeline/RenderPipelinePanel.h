#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "../../pipeline/render_pipeline.h"

#ifdef ECHO_EDITOR_MODE

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

	protected:
		// draw
		void drawStages();
		void drawStageAddButtons();

		// update icon
		void updateApplyButtonIcon();

	protected:
		// save
		virtual void save() override;

	protected:
		RenderPipeline*						m_pipeline;
		QIcon								m_playIcon;
		QIcon								m_stopIcon;
		QToolButton*						m_applyButton;
		QGraphicsView*						m_graphicsView = nullptr;
		Pipeline::QGraphicsSceneEx*			m_graphicsScene = nullptr;
		QGraphicsLineItem*					m_borderTopLine = nullptr;
		Pipeline::StageNodePainters			m_stageNodePainters;
		Pipeline::StatgeAddButtons			m_stageAddButtons;
	};
}

#endif
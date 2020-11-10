#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"
#include "../../pipeline/render_pipeline.h"

#ifdef ECHO_EDITOR_MODE

#include "stage_node_painter.h"
#include "renderqueue_node_painter.h"
#include "custom/qgraphics_scene_ex.h"

namespace Echo
{
	class RenderpipelinePanel : public PanelTab
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		RenderpipelinePanel(Object* obj);

		// update
		void update();

	public:
		// import
		void onNew();
		void onNewCamera();
		void onNewImageFilter();

	protected:
		// draw
		void drawStages();
		void drawRenderQueues();

	protected:
		// save
		virtual void save() override;

	protected:
		RenderPipeline*						m_pipeline;
		QGraphicsView*						m_graphicsView = nullptr;
		Pipeline::QGraphicsSceneEx*			m_graphicsScene = nullptr;
		QMenu*								m_importMenu = nullptr;
		QGraphicsLineItem*					m_borderTopLine = nullptr;
		Pipeline::StageNodePainters			m_stageNodePainters;
		Pipeline::RenderQueueNodePainters	m_renderQueueNodePainters;
	};
}

#endif
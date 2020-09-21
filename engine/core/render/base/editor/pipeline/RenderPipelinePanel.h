#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"
#include "../../pipeline/RenderPipeline.h"

#ifdef ECHO_EDITOR_MODE

#include "node_painter.h"

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

	protected:
		RenderPipeline*				m_pipeline;
		QGraphicsView*				m_graphicsView = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		QMenu*						m_importMenu = nullptr;
		Pipeline::StageNodePainters	m_stageNodePainters;
	};
}

#endif
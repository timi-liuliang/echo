#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "engine/modules/pcg/pcg_flow_graph.h"
#include "painter/background_grid_painter.h"
#include "painter/node_painter.h"
#include "painter/connect_painter.h"

#ifdef ECHO_EDITOR_MODE

#include "painter/custom/qgraphics_flow_scene.h"

namespace Echo
{
	class PCGFlowGraphPanel : public QObject
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		// Background style
		struct BackgroundStyle
		{
			Color m_backgroundColor;
			Color m_fineGridColor;
			Color m_coarseGridColor;
		};

	public:
		PCGFlowGraphPanel(Object* obj);
		virtual ~PCGFlowGraphPanel();

		// ui
		QDockWidget* getUi() { return m_ui; }

		// update
		void update();

	public:
		// new
		void onPlay();

		// import
		void onRightClickGraphicsView();
		void onNewPCGNode();

		// delete 
		void onDeletePGNodes();

		// refresh list
		void refreshUiDisplay();
		void drawNodes();
		void drawConnects();

	private:
		// draw
		void drawBackground();

	protected:
		PCGFlowGraph*						m_flowGraph = nullptr;
		QDockWidget*						m_ui;
		QMenu*								m_menuNew = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		Procedural::QGraphicsFlowScene*		m_graphicsScene = nullptr;
		BackgroundStyle						m_backgroundStyle;
		Procedural::QBackgroundGridPainter	m_backgroundGridSmall;
		Procedural::QBackgroundGridPainter	m_backgroundGridBig;
		Procedural::PCGNodePainters			m_pgNodePainters;
		Procedural::PCGConnectPainters		m_pcgConnectPainters;
		Echo::Vector2						m_newPGNodePosition = Vector2::ZERO;
	};
}

#endif

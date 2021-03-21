#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "pcg/pcg_flow_graph.h"
#include "painter/background_grid_painter.h"
#include "painter/node_painter.h"

#ifdef ECHO_EDITOR_MODE

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
		void onNewPGNode();

		// delete 
		void onDeletePGNodes();

		// refresh list
		void refreshUiDisplay();
		void drawNodes();
		void refreshImageDisplay();

		// clear
		void clearImageItemAndBorder();

	private:
		// draw
		void drawBackground();

	protected:
		PCGFlowGraph*					m_proceduralGeometry = nullptr;
		QDockWidget*						m_ui;
		QMenu*								m_menuNew = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		BackgroundStyle						m_backgroundStyle;
		Procedural::QBackgroundGridPainter	m_backgroundGridSmall;
		Procedural::QBackgroundGridPainter	m_backgroundGridBig;
		Procedural::PGNodePainters			m_pgNodePainters;
		Echo::Vector2						m_newPGNodePosition = Vector2::ZERO;
	};
}

#endif

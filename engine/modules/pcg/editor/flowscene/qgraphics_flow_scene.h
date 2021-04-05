#pragma once

#include <engine/core/editor/qt/QWidget.h>

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/pcg/pcg_flow_graph.h"
#include "item/qgraphics_connect_point_item.h"
#include "item/qgraphics_connect_item.h"
#include "flowscene/painter/qgraphics_grid_painter.h"
#include "flowscene/painter/qgraphics_node_painter.h"
#include "flowscene/painter/qgraphics_connect_painter.h"

namespace Procedural
{
	class QGraphicsFlowScene : public QGraphicsScene
	{
	public:
		QGraphicsFlowScene(QGraphicsView* graphicsView, Echo::PCGFlowGraph* flowGraph);
		virtual ~QGraphicsFlowScene();

		// connection
		void beginConnect(QGraphicsConnectPointItem* startPoint);
		void endConnect(QGraphicsConnectPointItem* startPoint);

		// is can connect
		bool isCanConnect(QGraphicsConnectPointItem* endPoint);

		// update
		void update();

	protected:
		// refresh list
		void refreshUiDisplay();
		void drawBackground();
		void drawNodes();
		void drawConnects();

	protected:
		// mouse event
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

	protected:
		QGraphicsView*						m_graphicsView = nullptr;
		Echo::PCGFlowGraph*					m_flowGraph = nullptr;
		QGraphicsConnectPointItem*			m_editingConnectionStartPoint = nullptr;
		QGraphicsConnectItem*				m_editingConnectItem = nullptr;
		QGraphicsConnectPointItem*			m_endConnectItemCandidate = nullptr;
		struct BackgroundStyle
		{
			Echo::Color m_backgroundColor;
			Echo::Color m_fineGridColor;
			Echo::Color m_coarseGridColor;
		}									m_backgroundStyle;
		Procedural::QBackgroundGridPainter	m_backgroundGridSmall;
		Procedural::QBackgroundGridPainter	m_backgroundGridBig;
		Procedural::PCGNodePainters			m_pgNodePainters;
		Procedural::PCGConnectPainters		m_pcgConnectPainters;
	};
}

#endif

#pragma once

#include <engine/core/editor/qt/QWidget.h>

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/pcg/pcg_flow_graph.h"
#include "item/qgraphics_connect_point_item.h"
#include "item/qgraphics_connect_item.h"

namespace Procedural
{
	class QGraphicsFlowScene : public QGraphicsScene
	{
	public:
		QGraphicsFlowScene(Echo::PCGFlowGraph* flowGraph);
		virtual ~QGraphicsFlowScene();

		// connection
		void beginConnect(QGraphicsConnectPointItem* startPoint);
		void endConnect(QGraphicsConnectPointItem* startPoint);

	protected:
		// mouse event
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

	protected:
		Echo::PCGFlowGraph*			m_flowGraph = nullptr;
		QGraphicsConnectPointItem*  m_editingConnectionStartPoint = nullptr;
		QGraphicsConnectItem*		m_editingConnectItem = nullptr;
	};
}

#endif

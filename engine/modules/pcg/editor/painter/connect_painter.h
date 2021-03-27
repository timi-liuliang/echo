#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/pcg/connect/pcg_connect.h"
#include "engine/modules/pcg/connect/pcg_connect_point.h"
#include "engine/modules/pcg/pcg_flow_graph.h"
#include "custom/qgraphics_connect_item.h"

namespace Procedural
{
	class PCGConnectPainter
	{
	public:
		PCGConnectPainter(QGraphicsView* view, QGraphicsScene* scene, Echo::PCGFlowGraph* flowGraph, Echo::PCGConnect* connect);
		~PCGConnectPainter();

		// pcg connect
		Echo::PCGConnect* getPCGConnect() { return m_pcgConnect; }

		// update
		void update();

	private:
		QGraphicsView*			m_graphicsView = nullptr;
		QGraphicsScene*			m_graphicsScene = nullptr;
		Echo::PCGFlowGraph*		m_pcgFlowGraph = nullptr;
		Echo::PCGConnect*		m_pcgConnect = nullptr;
		QGraphicsConnectItem*	m_connectItem = nullptr;
	};
	typedef Echo::vector<PCGConnectPainter*>::type PCGConnectPainters;
}

#endif
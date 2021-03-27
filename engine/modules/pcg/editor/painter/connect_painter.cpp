#include "connect_painter.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	PCGConnectPainter::PCGConnectPainter(QGraphicsView* view, QGraphicsScene* scene, Echo::PCGFlowGraph* flowGraph, Echo::PCGConnect* connect)
	{
		m_graphicsView = view;
		m_graphicsScene = scene;
		m_pcgFlowGraph = flowGraph;
		m_pcgConnect = connect;
	}

	PCGConnectPainter::~PCGConnectPainter()
	{

	}
}

#endif
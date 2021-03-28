#include "qgraphics_connect_painter.h"

#ifdef ECHO_EDITOR_MODE

#include "../item/qgraphics_connect_point_item.h"

namespace Procedural
{
	PCGConnectPainter::PCGConnectPainter(QGraphicsView* view, QGraphicsScene* scene, Echo::PCGFlowGraph* flowGraph, Echo::PCGConnect* connect)
	{
		m_graphicsView = view;
		m_graphicsScene = scene;
		m_pcgFlowGraph = flowGraph;
		m_pcgConnect = connect;

		m_connectItem = new QGraphicsConnectItem();
		m_connectItem->setPen(QPen(QColor(0, 139, 139), 1.5f));

		m_graphicsScene->addItem(m_connectItem);
	}

	PCGConnectPainter::~PCGConnectPainter()
	{
		if (m_connectItem)
			m_graphicsScene->removeItem(m_connectItem);
	}

	void PCGConnectPainter::update()
	{
		if (m_connectItem && m_pcgConnect)
		{
			Echo::PCGConnectPoint* from = m_pcgConnect->getFrom();
			Echo::PCGConnectPoint* to = m_pcgConnect->getTo();
			if (from && to)
			{
				QGraphicsConnectPointItem* fromItem = QGraphicsConnectPointItem::getByPCGConnectPoint(from);
				QGraphicsConnectPointItem* toItem = QGraphicsConnectPointItem::getByPCGConnectPoint(to);
				if (fromItem && toItem)
				{
					QPointF fromLocalCenter = fromItem->boundingRect().center();
					QPointF toLocalCenter = toItem->boundingRect().center();

					m_connectItem->set(fromItem->scenePos() + fromLocalCenter, toItem->scenePos() + toLocalCenter);
				}
			}
		}
	}
}

#endif
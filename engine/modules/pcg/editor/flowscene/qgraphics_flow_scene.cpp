#include "qgraphics_flow_scene.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/pcg/connect/pcg_connect.h"

namespace Procedural
{
	QGraphicsFlowScene::QGraphicsFlowScene(Echo::PCGFlowGraph* flowGraph)
		: m_flowGraph(flowGraph)
	{

	}

	QGraphicsFlowScene::~QGraphicsFlowScene()
	{

	}

	void QGraphicsFlowScene::beginConnect(QGraphicsConnectPointItem* startPoint)
	{
		if (!m_editingConnectItem)
		{
			m_editingConnectionStartPoint = startPoint;

			m_editingConnectItem = new QGraphicsConnectItem;
			m_editingConnectItem->set(m_editingConnectionStartPoint->scenePos(), m_editingConnectionStartPoint->scenePos());

			addItem(m_editingConnectItem);
		}
	}

	void QGraphicsFlowScene::endConnect(QGraphicsConnectPointItem* endPoint)
	{
		if (m_editingConnectItem)
		{
			if (endPoint)
			{
				Echo::PCGConnectPoint* pcgStartPoint = m_editingConnectionStartPoint->getPCGConnectPoint();
				Echo::PCGConnectPoint* pcgEndPoint = endPoint->getPCGConnectPoint();
				if (pcgStartPoint && pcgEndPoint)
				{
					using namespace Echo;

					PCGConnect* pcgConnect = EchoNew(PCGConnect(pcgStartPoint, pcgEndPoint));
					m_flowGraph->addConnect(pcgConnect);
				}
			}
			else
			{
				removeItem(m_editingConnectItem);
				m_editingConnectionStartPoint = nullptr;
				m_editingConnectItem = nullptr;
			}
		}
	}

	void QGraphicsFlowScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mousePressEvent(event);

		QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
		if (item)
		{
			QGraphicsConnectPointItem* connectPointItem = dynamic_cast<QGraphicsConnectPointItem*>(item);
			if(connectPointItem)
				beginConnect(connectPointItem);
		}
	}

	void QGraphicsFlowScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mouseMoveEvent(event);

		if (m_editingConnectItem)
		{
			m_editingConnectItem->set(m_editingConnectionStartPoint->scenePos(), event->scenePos());
		}
	}

	void QGraphicsFlowScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mouseReleaseEvent(event);

		QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
		if (item)
		{
			QGraphicsConnectPointItem* connectPointItem = dynamic_cast<QGraphicsConnectPointItem*>(item);
			endConnect(connectPointItem);
		}

		endConnect(nullptr);
	}
}

#endif
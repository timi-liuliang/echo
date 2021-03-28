#include "qgraphics_flow_scene.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/pcg/connect/pcg_connect.h"
#include "engine/modules/pcg/node/pcg_node.h"

namespace Procedural
{
	QGraphicsFlowScene::QGraphicsFlowScene(QGraphicsView* graphicsView, Echo::PCGFlowGraph* flowGraph)
		: m_graphicsView(graphicsView)
		, m_flowGraph(flowGraph)
	{
		// background
		m_backgroundGridSmall.set(m_graphicsView, this);
		m_backgroundGridBig.set(m_graphicsView, this);
	}

	QGraphicsFlowScene::~QGraphicsFlowScene()
	{

	}

	void QGraphicsFlowScene::update()
	{
		refreshUiDisplay();
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

	void QGraphicsFlowScene::refreshUiDisplay()
	{
		drawBackground();
		drawNodes();
		drawConnects();
	}

	void QGraphicsFlowScene::drawBackground()
	{
		m_backgroundStyle.m_backgroundColor.setRGBA(77, 77, 77, 255);
		m_backgroundStyle.m_fineGridColor.setRGBA(84, 84, 84, 255);
		m_backgroundStyle.m_coarseGridColor.setRGBA(64, 64, 64, 255);

		QBrush brush(QColor::fromRgbF(m_backgroundStyle.m_backgroundColor.r, m_backgroundStyle.m_backgroundColor.g, m_backgroundStyle.m_backgroundColor.b, m_backgroundStyle.m_backgroundColor.a));
		m_graphicsView->setBackgroundBrush(brush);

		m_backgroundGridSmall.update(15, m_backgroundStyle.m_fineGridColor);
		m_backgroundGridBig.update(150, m_backgroundStyle.m_coarseGridColor);
	}

	void QGraphicsFlowScene::drawNodes()
	{
		const Echo::vector<Echo::PCGNode*>::type& pcgNodes = m_flowGraph->getNodes();
		while (m_pgNodePainters.size() > pcgNodes.size())
		{
			EchoSafeDelete(m_pgNodePainters.back(), PCGNodePainter);
			m_pgNodePainters.pop_back();
		}

		if (m_pgNodePainters.size() < pcgNodes.size())
		{
			for (size_t i = m_pgNodePainters.size(); i < pcgNodes.size(); ++i)
				m_pgNodePainters.emplace_back(EchoNew(Procedural::PCGNodePainter(m_graphicsView, this, m_flowGraph, pcgNodes[i])));
		}

		for (size_t i = 0; i < pcgNodes.size(); i++)
		{
			if (!m_pgNodePainters[i] || m_pgNodePainters[i]->getPCGNode() != pcgNodes[i])
			{
				EchoSafeDelete(m_pgNodePainters[i], PCGNodePainter);
				m_pgNodePainters[i] = EchoNew(Procedural::PCGNodePainter(m_graphicsView, this, m_flowGraph, pcgNodes[i]));
			}
		}

		for (size_t i = 0; i < pcgNodes.size(); i++)
		{
			m_pgNodePainters[i]->update();
		}
	}

	void QGraphicsFlowScene::drawConnects()
	{
		const Echo::vector<Echo::PCGConnect*>::type& pcgConnects = m_flowGraph->getConnects();
		while (m_pcgConnectPainters.size() > pcgConnects.size())
		{
			EchoSafeDelete(m_pcgConnectPainters.back(), PCGConnectPainter);
			m_pcgConnectPainters.pop_back();
		}

		if (m_pcgConnectPainters.size() < pcgConnects.size())
		{
			for (size_t i = m_pcgConnectPainters.size(); i < pcgConnects.size(); ++i)
				m_pcgConnectPainters.emplace_back(EchoNew(Procedural::PCGConnectPainter(m_graphicsView, this, m_flowGraph, pcgConnects[i])));
		}

		for (size_t i = 0; i < pcgConnects.size(); i++)
		{
			if (!m_pcgConnectPainters[i] || m_pcgConnectPainters[i]->getPCGConnect() != pcgConnects[i])
			{
				EchoSafeDelete(m_pcgConnectPainters[i], PCGConnectPainter);
				m_pcgConnectPainters[i] = EchoNew(Procedural::PCGConnectPainter(m_graphicsView, this, m_flowGraph, pcgConnects[i]));
			}
		}

		for (size_t i = 0; i < pcgConnects.size(); i++)
		{
			m_pcgConnectPainters[i]->update();
		}
	}
}

#endif
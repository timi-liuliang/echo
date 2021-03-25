#include "node_painter.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	PCGNodePainter::PCGNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::PCGFlowGraph* flowGraph, Echo::PCGNode* pgNode)
	{
		m_pcgFlowGraph = flowGraph;
		m_pcgNode = pgNode;
		m_graphicsView = view;
		m_graphicsScene = scene;

		if (!m_rect)
		{
			float halfWidth = m_width * 0.5f;
			float halfHeight = m_height * 0.5f;

			m_rect = new QGraphicsRoundRectItem(nullptr, m_pcgNode);
			m_rect->setRect(QRect(-halfWidth, -halfHeight, m_width, m_height));
			m_rect->setRadius(0.f);
			m_rect->setPen(QPen(m_style.m_normalBoundaryColor, m_style.m_penWidth));
			QLinearGradient gradient(QPointF(0.0, -halfHeight), QPointF(0.0, halfHeight));
			gradient.setColorAt(0.0, m_style.m_gradientColor0);
			gradient.setColorAt(0.03, m_style.m_gradientColor1);
			gradient.setColorAt(0.97, m_style.m_gradientColor2);
			gradient.setColorAt(1.0, m_style.m_gradientColor3);
			m_rect->setBrush(gradient);
			m_rect->setFlag(QGraphicsItem::ItemIsMovable, true);
			m_rect->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
			m_rect->setFlag(QGraphicsItem::ItemIsFocusable, true);
			m_rect->setPos(QPointF(pgNode->getPosition().x, pgNode->getPosition().y));
			m_graphicsScene->addItem(m_rect);

			m_rectFinal = new QGraphicsRoundRectItemFinal(nullptr, m_pcgNode);
			m_rectFinal->setRect(QRect(halfWidth - m_rectFinalWidth - 1, -halfHeight + 1, m_rectFinalWidth, m_height - 2));
			m_rectFinal->setRadius(0.f);
			m_rectFinal->setPen(QPen(m_style.m_finalBoundaryColor, 0.f));
			m_rectFinal->setBrush(QBrush(m_style.finalColor));
			m_rectFinal->setZValue(-5.f);
			m_rectFinal->setParentItem(m_rect);

			m_text = m_graphicsScene->addSimpleText(Echo::StringUtil::Replace(pgNode->getClassName(), "PCG", "").c_str());
			m_text->setBrush(QBrush(m_style.m_fontColor));
			m_text->setParentItem(m_rect);

			QRectF textRect = m_text->sceneBoundingRect();
			m_text->setPos((m_width - textRect.width()) * 0.5f - halfWidth, (m_height - textRect.height()) * 0.5f - halfHeight);

			float halfConnectPointRadius = m_connectPointRadius * 0.5f;
			m_inputConnectionPoints.push_back(EditorApi.qGraphicsSceneAddEclipse(m_graphicsScene, 0.f, 0.f, m_connectPointRadius, m_connectPointRadius, m_connectPointColor));
			for (QGraphicsItem* item : m_inputConnectionPoints)
			{
				item->setParentItem(m_rect);
				item->setPos(0.f - halfConnectPointRadius, -halfHeight - halfConnectPointRadius * 3.f);
			}

			m_outputConnectionPoints.push_back(EditorApi.qGraphicsSceneAddEclipse(m_graphicsScene, 0.f, 0.f, m_connectPointRadius, m_connectPointRadius, m_connectPointColor));
			for (QGraphicsItem* item : m_outputConnectionPoints)
			{
				item->setParentItem(m_rect);
				item->setPos(0.f - halfConnectPointRadius, halfHeight + halfConnectPointRadius);
			}
		}
	}

	PCGNodePainter::~PCGNodePainter()
	{
		reset();
	}

	void PCGNodePainter::reset()
	{
		if (m_rect)
			m_graphicsScene->removeItem(m_rect);

		if (m_rectFinal)
			m_graphicsScene->removeItem(m_rectFinal);

		m_inputConnectionPoints.clear();
		m_outputConnectionPoints.clear();

		m_pcgNode = nullptr;
		m_graphicsView = nullptr;
		m_graphicsScene = nullptr;
		m_rect = nullptr;
		m_rectFinal = nullptr;
		m_text = nullptr;
	}

	void PCGNodePainter::set()
	{

	}

	void PCGNodePainter::update()
	{
		if (m_pcgNode && m_rectFinal)
		{	
			m_rectFinal->setBrush(QBrush(m_pcgFlowGraph->getOutputNode() == m_pcgNode ? m_style.finalColor : Qt::transparent));
		}

		if (m_pcgNode && m_rect)
		{
			//m_rect->setPen(QPen(m_pcgNode->isSelected() ? m_style.m_selectedBoundaryColor : m_style.m_normalBoundaryColor, m_style.m_penWidth));
		}
	}
}

#endif
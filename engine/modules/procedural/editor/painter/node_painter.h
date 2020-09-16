#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/procedural/procedural_geometry.h"
#include <QPen>
#include <QGRaphicsScene>
#include <QtWidgets/QGraphicsItem>
#include "qgraphics_round_item.h"

namespace Procedural
{
	struct PGNodePainter
	{
		Echo::PGNode*						m_pgNode = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		QGraphicsRoundRectItem*				m_rect = nullptr;
		Echo::vector<QGraphicsItem*>::type	m_inputConnectionPoints;
		Echo::vector<QGraphicsItem*>::type	m_outputConnectionPoints;
		Echo::Color							m_normalBoundaryColor;
		Echo::Color							m_selectedBoundaryColor;
		Echo::Color							m_regionColor;
		float								m_width = 120;
		float								m_height = 40;
		float								m_connectPointRadius = 8.f;
		Echo::Color							m_connectPointColor = Echo::Color::DARKCYAN;
		QGraphicsSimpleTextItem*			m_text = nullptr;
		Echo::Color							m_textColor;

		PGNodePainter()
		{

		}

		~PGNodePainter()
		{
			reset();
		}

		// reset
		void reset()
		{
			if (m_text)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_text);
				m_text = nullptr;
			}

			for (QGraphicsItem* connectionPoint : m_inputConnectionPoints)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, connectionPoint);
			}

			for (QGraphicsItem* connectionPoint : m_outputConnectionPoints)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, connectionPoint);
			}

			m_inputConnectionPoints.clear();
			m_outputConnectionPoints.clear();

			if (m_rect)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_rect);
				m_rect = nullptr;
			}
		}

		// update
		void update(QGraphicsView* view, QGraphicsScene* scene, Echo::PGNode* pgNode)
		{
			if (m_pgNode != pgNode)
			{
				reset();
				m_pgNode = pgNode;
			}

			if (!m_rect)
			{
				m_graphicsView = view;
				m_graphicsScene = scene;

				m_selectedBoundaryColor.setRGBA(255, 165, 0, 255);
				m_regionColor.setRGBA(80, 80, 80, 255);
				m_textColor.setRGBA(178, 178, 178, 255);

				float halfWidth = m_width * 0.5f;
				float halfHeight = m_height * 0.5f;

				m_rect = new QGraphicsRoundRectItem();
				m_rect->setRect(QRect(-halfWidth, -halfHeight, m_width, m_height));
				m_rect->setRadius(4.f);
				m_rect->setPen(QPen(QColor::fromRgbF(m_normalBoundaryColor.r, m_normalBoundaryColor.g, m_normalBoundaryColor.b, m_normalBoundaryColor.a)));
				m_rect->setBrush(QBrush(QColor::fromRgbF(m_regionColor.r, m_regionColor.g, m_regionColor.b, m_regionColor.a)));
				m_rect->setFlag(QGraphicsItem::ItemIsMovable, true);
				m_rect->setPos(QPointF(pgNode->getPosition().x, pgNode->getPosition().y));
				m_graphicsScene->addItem(m_rect);

				m_text = m_graphicsScene->addSimpleText("Sphere");
				m_text->setBrush(QBrush(QColor::fromRgbF(m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a)));
				m_text->setParentItem(m_rect);

				Echo::Rect textRect;
				EditorApi.qGraphicsItemSceneRect(m_text, textRect);
				m_text->setPos((m_width - textRect.getWidth()) * 0.5f - halfWidth, (m_height - textRect.getHeight()) * 0.5f - halfHeight);

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

				QPen pen;
				pen.setColor(QColor::fromRgbF(1.f, 1.f, 1.f, 1.f));
			}
		}
	};
	typedef Echo::vector<PGNodePainter>::type PGNodePainters;
}

#endif
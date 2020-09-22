#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/modules/procedural/procedural_geometry.h"
#include <QPen>
#include <QGraphicsScene>
#include <QtWidgets/QGraphicsItem>
#include "qgraphics_round_item.h"

namespace Procedural
{
	struct PGNodePainter
	{
		struct 
		{
			QColor m_normalBoundaryColor = QColor(137, 137, 137);
			QColor m_selectedBoundaryColor = QColor(255, 165, 0);
			QColor m_gradientColor0 = Qt::gray;
			QColor m_gradientColor1 = QColor(80, 80, 80);
			QColor m_gradientColor2 = QColor(64, 64, 64);
			QColor m_gradientColor3 = QColor(58, 58, 58);
			QColor m_shadowColor	= QColor(20, 20, 20);
			QColor m_fontColor		= Qt::gray;
			QColor m_fontColorFaded = Qt::gray;
			QColor m_connectionPointColor = QColor(169, 169, 169);
			QColor m_filledConnectionPointColor = Qt::cyan;
			QColor m_warningColor = QColor(128, 128, 0);
			QColor m_errorColor	  = Qt::red;
			QColor finalColor	  = QColor(54, 108, 179, 255);
			float  m_penWidth = 1.5f;
		}									m_style;
		Echo::PGNode*						m_pgNode = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		QGraphicsRoundRectItem*				m_rect = nullptr;
		QGraphicsRoundRectItem*				m_rectFinal = nullptr;
		float								m_rectFinalWidth = 15;
		Echo::vector<QGraphicsItem*>::type	m_inputConnectionPoints;
		Echo::vector<QGraphicsItem*>::type	m_outputConnectionPoints;
		float								m_width = 120;
		float								m_height = 40;
		float								m_connectPointRadius = 8.f;
		Echo::Color							m_connectPointColor = Echo::Color::DARKCYAN;
		QGraphicsSimpleTextItem*			m_text = nullptr;

		PGNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::PGNode* pgNode)
		{
			m_pgNode = pgNode;
			m_graphicsView = view;
			m_graphicsScene = scene;

			if (!m_rect)
			{
				float halfWidth = m_width * 0.5f;
				float halfHeight = m_height * 0.5f;

				m_rect = new QGraphicsRoundRectItem(nullptr);
				m_rect->setRect(QRect(-halfWidth, -halfHeight, m_width, m_height));
				m_rect->setRadius(0.f);
				m_rect->setPen(QPen(m_style.m_normalBoundaryColor, m_style.m_penWidth));
				QLinearGradient gradient(QPointF(0.0, -halfHeight), QPointF(0.0, halfHeight));
				gradient.setColorAt(0.0,  m_style.m_gradientColor0);
				gradient.setColorAt(0.03, m_style.m_gradientColor1);
				gradient.setColorAt(0.97, m_style.m_gradientColor2);
				gradient.setColorAt(1.0,  m_style.m_gradientColor3);
				m_rect->setBrush(gradient);
				m_rect->setFlag(QGraphicsItem::ItemIsMovable, true);
				m_rect->setPos(QPointF(pgNode->getPosition().x, pgNode->getPosition().y));
				m_graphicsScene->addItem(m_rect);

				m_rectFinal = new QGraphicsRoundRectItem(nullptr, m_pgNode);
				m_rectFinal->setRect(QRect(halfWidth - m_rectFinalWidth, -halfHeight, m_rectFinalWidth, m_height));
				m_rectFinal->setRadius(3.f);
				m_rectFinal->setPen(QPen(m_style.m_normalBoundaryColor, 1.f));
				m_rectFinal->setBrush(QBrush(m_style.finalColor));
				m_rectFinal->setZValue(-5.f);
				m_rectFinal->setParentItem(m_rect);

				m_text = m_graphicsScene->addSimpleText(Echo::StringUtil::Replace(pgNode->getClassName(), "PG", "").c_str());
				m_text->setBrush(QBrush(m_style.m_fontColor));
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
			}
		}

		~PGNodePainter()
		{
			reset();
		}

		// reset
		void reset()
		{
			if (m_rect)
				m_graphicsScene->removeItem(m_rect);

			if (m_rectFinal)
				m_graphicsScene->removeItem(m_rectFinal);

			m_inputConnectionPoints.clear();
			m_outputConnectionPoints.clear();

			m_pgNode = nullptr;
			m_graphicsView = nullptr;
			m_graphicsScene = nullptr;
			m_rect = nullptr;
			m_rectFinal = nullptr;
			m_text = nullptr;
		}

		// set
		void set()
		{

		}

		// update
		void update()
		{
			if (m_pgNode && m_rectFinal)
			{
				m_rectFinal->setBrush(QBrush( m_pgNode->isFinal() ? m_style.finalColor : Qt::transparent));
			}
		}
	};
	typedef Echo::vector<PGNodePainter*>::type PGNodePainters;
}

#endif
#pragma once

#ifdef ECHO_EDITOR_MODE

#include "engine/core/editor/editor.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include <QPen>
#include <QGraphicsScene>
#include <QtWidgets/QGraphicsItem>

namespace Procedural
{
	struct PGNodesPainter
	{
		QWidget*							m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		QGraphicsItem*						m_rect = nullptr;
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

		~PGNodesPainter()
		{
		}

		// set
		void set(QWidget* view, QObject* scene)
		{
			m_graphicsView = view;
			m_graphicsScene = (QGraphicsScene*)scene;

			m_selectedBoundaryColor.setRGBA(255, 165, 0, 255);
			m_regionColor.setRGBA(80, 80, 80,255);
			m_textColor.setRGBA(178, 178, 178, 255);
		}

		// reset
		void reset()
		{
			if (m_rect)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_rect);
				m_rect = nullptr;
			}

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
		}

		// update
		void update(Echo::ProceduralGeometry* pg)
		{
			if (!m_rect)
			{
				m_rect = EditorApi.qGraphicsSceneAddRect(m_graphicsScene, 0.f, 0.f, m_width, m_height, m_normalBoundaryColor, m_regionColor);
				EditorApi.qGraphicsItemSetMoveable(m_rect, true);

				m_text = m_graphicsScene->addSimpleText("Sphere");
				m_text->setBrush(QBrush(QColor::fromRgbF(m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a)));
				m_text->setParentItem(m_rect);

				Echo::Rect textRect;
				EditorApi.qGraphicsItemSceneRect(m_text, textRect);
				EditorApi.qGraphicsItemSetPos(m_text, (m_width - textRect.getWidth()) * 0.5f, (m_height - textRect.getHeight()) * 0.5f);

				m_inputConnectionPoints.push_back(EditorApi.qGraphicsSceneAddEclipse(m_graphicsScene, (m_width - m_connectPointRadius) * 0.5f, - m_connectPointRadius * 0.5f, m_connectPointRadius, m_connectPointRadius, m_connectPointColor));
				for (QGraphicsItem* item : m_inputConnectionPoints)
				{
					item->setParentItem(m_rect);
				}

				m_outputConnectionPoints.push_back(EditorApi.qGraphicsSceneAddEclipse(m_graphicsScene, (m_width - m_connectPointRadius) * 0.5f, m_height-m_connectPointRadius * 0.5f, m_connectPointRadius, m_connectPointRadius, m_connectPointColor));
				for (QGraphicsItem* item : m_outputConnectionPoints)
				{
					item->setParentItem(m_rect);
				}

				QPen pen;
				pen.setColor(QColor::fromRgbF(1.f, 1.f, 1.f, 1.f));
			}

			for (Echo::PGNode* pgNode : pg->getPGNodes())
			{

			}
		}
	};
}

#endif
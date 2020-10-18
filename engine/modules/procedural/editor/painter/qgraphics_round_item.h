#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>

namespace Procedural
{
	class QGraphicsRoundRectItem : public QGraphicsRectItem
	{
	public:
		QGraphicsRoundRectItem(QGraphicsItem* parent = nullptr, Echo::PGNode* pgNode = nullptr)
			: QGraphicsRectItem(parent)
			, m_pgNode(pgNode)
		{

		}

		// custom paint
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override
		{
			painter->setPen(pen());
			painter->setBrush(brush());
			painter->drawRoundedRect(rect(), m_radius, m_radius);
		}

		// set radius
		void setRadius(float radius) { m_radius = radius; }

		// mouse event
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override
		{
			QGraphicsRectItem::mousePressEvent(event);

			if (m_pgNode)
			{
				m_pgNode->setFinal(!m_pgNode->isFinal());

				EditorApi.showObjectProperty(m_pgNode);
			}
		}

	protected:
		float			m_radius = 3.f;
		Echo::PGNode*	m_pgNode = nullptr;
	};
}

#endif

#pragma once

#include "engine/core/editor/editor.h"
#include "engine/modules/procedural/geometry/node/pg_node.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>

namespace Procedural
{
	class QGraphicsRoundRectItemFinal : public QGraphicsRectItem
	{
	public:
		QGraphicsRoundRectItemFinal(QGraphicsItem* parent = nullptr, Echo::PGNode* pgNode = nullptr)
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

		// key press
		virtual void keyPressEvent(QKeyEvent* event)
		{
			if (event->key() == Qt::Key_Delete)
			{
				if (m_pgNode)
				{
					m_pgNode->queueFree();
				}
			}
		}

	protected:
		float			m_radius = 3.f;
		Echo::PGNode*	m_pgNode = nullptr;
	};
}

#endif

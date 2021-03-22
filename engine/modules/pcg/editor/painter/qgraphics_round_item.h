#pragma once

#include "engine/core/editor/editor.h"
#include "engine/modules/pcg/node/pcg_node.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>

namespace Procedural
{
	class QGraphicsRoundRectItem : public QGraphicsRectItem
	{
	public:
		QGraphicsRoundRectItem(QGraphicsItem* parent = nullptr, Echo::PCGNode* pgNode = nullptr)
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
				EditorApi.showObjectProperty(m_pgNode);
				//m_pgNode->setSelected(true);
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
		// position changed etc...
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value)
		{
			if (change == ItemPositionChange && m_pgNode)
			{
				m_pgNode->setPosition(Echo::Vector2(pos().x(), pos().y()));
			}

			return QGraphicsRectItem::itemChange(change, value);
		}

	protected:
		bool			m_isFinalRect = false;
		float			m_radius = 3.f;
		Echo::PCGNode*	m_pgNode = nullptr;
	};
}

#endif

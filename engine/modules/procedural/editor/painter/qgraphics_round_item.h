#pragma once

#include "engine/core/editor/editor.h"
#include "engine/modules/procedural/pg/node/pg_node.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>

namespace Procedural
{
	class QGraphicsRoundRectItem : public QGraphicsRectItem
	{
	public:
		QGraphicsRoundRectItem(QGraphicsItem* parent = nullptr, Echo::PGNode* pgNode = nullptr, bool isFinalRect=false)
			: QGraphicsRectItem(parent)
			, m_pgNode(pgNode)
			, m_isFinalRect(isFinalRect)
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
				if(m_isFinalRect)
					m_pgNode->setFinal(!m_pgNode->isFinal());

				EditorApi.showObjectProperty(m_pgNode);
			}
		}

	protected:
		// position changed etc...
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

	protected:
		bool			m_isFinalRect = false;
		float			m_radius = 3.f;
		Echo::PGNode*	m_pgNode = nullptr;
	};
}

#endif

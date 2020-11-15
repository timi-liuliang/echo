#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>
#include <QGraphicsSceneEvent>

namespace Pipeline
{
	class QGraphicsPixmapItemCustom : public QGraphicsPixmapItem
	{
	public:
		QGraphicsPixmapItemCustom(QGraphicsItem* parent = nullptr)
			: QGraphicsPixmapItem(parent)
		{}

		// hover event callback
		void setHoverEnterEventCb(std::function<void(QGraphicsPixmapItem*)> cb) { m_hoverEnterEventCb = cb;}
		void setHoverEnterLeaveCb(std::function<void(QGraphicsPixmapItem*)> cb) { m_hoverLeaveEventCb = cb; }

		// mouse event cb
		void setMousePressEventCb(std::function<void(QGraphicsPixmapItem*)> cb) { m_mousePressEventCb = cb; }

	protected:
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsPixmapItem::hoverEnterEvent(event);

			if (m_hoverEnterEventCb)
				m_hoverEnterEventCb(this);
		}

		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsPixmapItem::hoverLeaveEvent(event);

			if (m_hoverLeaveEventCb)
				m_hoverLeaveEventCb(this);
		}

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event)
		{
			QGraphicsItem* item = scene()->itemAt(event->scenePos(), QTransform());
			if (item == this)
			{
				if (m_mousePressEventCb)
					m_mousePressEventCb(this);
			}
		}

	protected:
		// position changed etc...
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value)
		{
			return QGraphicsPixmapItem::itemChange(change, value);
		}

	protected:
		std::function<void(QGraphicsPixmapItem*)> m_hoverEnterEventCb;
		std::function<void(QGraphicsPixmapItem*)> m_hoverLeaveEventCb;
		std::function<void(QGraphicsPixmapItem*)> m_mousePressEventCb;
	};
}

#endif

#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>

namespace Pipeline
{
	class QGraphicsPixmapItemCustom : public QGraphicsPixmapItem
	{
	public:
		QGraphicsPixmapItemCustom(QGraphicsItem* parent = nullptr)
			: QGraphicsPixmapItem(parent)
		{

		}

		void setHoverEnterEventCb(std::function<void(QGraphicsPixmapItem*)> cb)
		{
			m_hoverEnterEvent = cb;
		}

		void setHoverEnterLeaveCb(std::function<void(QGraphicsPixmapItem*)> cb)
		{
			m_hoverLeaveEvent = cb;
		}

		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsPixmapItem::hoverEnterEvent(event);

			if (m_hoverEnterEvent)
				m_hoverEnterEvent(this);
		}

		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsPixmapItem::hoverLeaveEvent(event);

			if (m_hoverLeaveEvent)
				m_hoverLeaveEvent(this);
		}

	protected:
		// position changed etc...
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value)
		{
			return QGraphicsPixmapItem::itemChange(change, value);
		}

	protected:
		std::function<void(QGraphicsPixmapItem*)> m_hoverEnterEvent;
		std::function<void(QGraphicsPixmapItem*)> m_hoverLeaveEvent;
	};
}

#endif

#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>
#include <QGraphicsSceneEvent>

namespace Pipeline
{
	class QGraphicsSimpleTextItemCustom : public QGraphicsSimpleTextItem
	{
	public:
		QGraphicsSimpleTextItemCustom(QGraphicsItem* parent = nullptr)
			: QGraphicsSimpleTextItem(parent)
		{}

		// hover event callback
		void setHoverEnterEventCb(std::function<void(QGraphicsSimpleTextItem*)> cb) { m_hoverEnterEventCb = cb; }
		void setHoverEnterLeaveCb(std::function<void(QGraphicsSimpleTextItem*)> cb) { m_hoverLeaveEventCb = cb; }

		// mouse event cb
		void setMousePressEventCb(std::function<void(QGraphicsSimpleTextItem*)> cb) { m_mousePressEventCb = cb; }

		// key event cb
		void setKeyPressEventCb(std::function<void(QKeyEvent*)> cb) { m_keyEventCb = cb; }

	protected:
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsSimpleTextItem::hoverEnterEvent(event);

			if (m_hoverEnterEventCb)
				m_hoverEnterEventCb(this);
		}

		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsSimpleTextItem::hoverLeaveEvent(event);

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
		std::function<void(QGraphicsSimpleTextItem*)> m_hoverEnterEventCb;
		std::function<void(QGraphicsSimpleTextItem*)> m_hoverLeaveEventCb;
		std::function<void(QGraphicsSimpleTextItem*)> m_mousePressEventCb;
		std::function<void(QKeyEvent*)>				  m_keyEventCb;
	};
}

#endif

#include "qgraphics_connect_item.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	QGraphicsConnectItem::QGraphicsConnectItem()
		: QGraphicsPathItem()
	{}

	QGraphicsConnectItem::~QGraphicsConnectItem()
	{

	}

	void QGraphicsConnectItem::set(const Echo::Vector2& startPos, const Echo::Vector2& endPos)
	{
		if (m_startPos != startPos || m_endPos != endPos)
		{
			QPainterPath path;
			path.moveTo(QPointF(startPos.x, startPos.y));
			path.lineTo(QPointF(endPos.x, endPos.y));

			setPath(path);

			m_startPos = startPos;
			m_endPos = endPos;
		}
	}

	void QGraphicsConnectItem::focusInEvent(QFocusEvent* event)
	{
		m_focused = true;
	}

	void QGraphicsConnectItem::focusOutEvent(QFocusEvent* event)
	{
		m_focused = false;
	}

	void QGraphicsConnectItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
	{
		QGraphicsPathItem::hoverEnterEvent(event);
	}

	void QGraphicsConnectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
	{
		QGraphicsPathItem::hoverLeaveEvent(event);
	}

	void QGraphicsConnectItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsItem* item = scene()->itemAt(event->scenePos(), QTransform());
		if (item == this)
		{
		}
	}

	void QGraphicsConnectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsPathItem::mouseReleaseEvent(event);
	}

	void QGraphicsConnectItem::keyPressEvent(QKeyEvent* event)
	{
	}

	void QGraphicsConnectItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsPathItem::mouseMoveEvent(event);
	}

	bool QGraphicsConnectItem::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
	{
		if (watched->type() == QGraphicsLineItem::Type)
		{
			if (event->type() == QEvent::MouseButtonPress)
			{
				mousePressEvent(dynamic_cast<QGraphicsSceneMouseEvent*>(event));
			}
			else if (event->type() == QEvent::KeyPress)
			{
				keyPressEvent(dynamic_cast<QKeyEvent*>(event));
			}
		}

		return false;
	}
}

#endif
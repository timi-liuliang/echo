#include "qgraphics_connect_item.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	QGraphicsConnectItem::QGraphicsConnectItem()
		: QGraphicsPathItem()
	{
		setZValue(-10.f);
	}

	QGraphicsConnectItem::~QGraphicsConnectItem()
	{

	}

	void QGraphicsConnectItem::set(const QPointF& startPos, const QPointF& endPos)
	{
		if (m_startPos != startPos || m_endPos != endPos)
		{
			float xdiff = endPos.x() - startPos.x();
			float ydiff = endPos.y() - startPos.y();

			QPointF ctrlPt1 = startPos + QPointF(-xdiff * 0.f, ydiff * 0.9f);
			QPointF ctrlPt2 = endPos - QPointF(-xdiff * 0.f, ydiff * 0.9f);

			QPainterPath path;
			path.moveTo(startPos);
			path.cubicTo(ctrlPt1, ctrlPt2, endPos);

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
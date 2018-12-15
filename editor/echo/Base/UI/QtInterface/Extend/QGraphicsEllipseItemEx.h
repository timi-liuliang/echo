#pragma once

#include <QGraphicsItem>

namespace Echo
{
	/**
	 * QGraphicsEllipseItemEx
	 * used to Convert QGraphicsEllipseItem events to signals
	 */
	class QGraphicsEllipseItemEx : public QGraphicsEllipseItem
	{
	protected:
		virtual void focusInEvent(QFocusEvent *event) override
		{
			QGraphicsEllipseItem::focusInEvent(event);
		}

		virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsEllipseItem::mousePressEvent(event);
		}

		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsEllipseItem::mouseMoveEvent(event);
		}

		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsEllipseItem::mouseReleaseEvent(event);
		}

		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsEllipseItem::mouseDoubleClickEvent(event);
		}
	};
}


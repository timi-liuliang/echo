#pragma once

#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <engine/core/editor/qt/QWidget.h>
#include "QMessageHandler.h"

namespace Echo
{
	/**
	 * QGraphicsEllipseItemEx
	 * used to Convert QGraphicsEllipseItem events to signals
	 */
	class QGraphicsEllipseItemEx : public QGraphicsEllipseItem
	{
	protected:
		virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override
		{
			QGraphicsEllipseItem::dragMoveEvent(event);
			QMessageHandler::instance()->onReceiveQGraphicsItemMessage(this, QSIGNAL(dragMoveEvent(QGraphicsSceneDragDropEvent*)));
		}

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

			//QMessageHandler::instance()->getEvent(this).graphicsSceneMouseEvent.scenePos = Vector2(event->scenePos().x(), event->scenePos().y());
			//QMessageHandler::instance()->onReceiveQGraphicsItemMessage(this, QSIGNAL(mouseMoveEvent(QGraphicsSceneMouseEvent*)));
		}

		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsEllipseItem::mouseReleaseEvent(event);
		}

		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsEllipseItem::mouseDoubleClickEvent(event);

			QMessageHandler::instance()->getEvent(this).graphicsSceneMouseEvent.scenePos = Vector2(event->scenePos().x(), event->scenePos().y());
			QMessageHandler::instance()->onReceiveQGraphicsItemMessage( this, QSIGNAL(mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)));
		}
	};
}


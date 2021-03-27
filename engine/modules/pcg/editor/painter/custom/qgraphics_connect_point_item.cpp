#include "qgraphics_connect_point_item.h"

#ifdef ECHO_EDITOR_MODE

#include "qgraphics_flow_scene.h"

namespace Procedural
{
	QGraphicsConnectPointItem::QGraphicsConnectPointItem()
		: QGraphicsEllipseItem()
	{}

	QGraphicsConnectPointItem::~QGraphicsConnectPointItem()
	{

	}

	void QGraphicsConnectPointItem::focusInEvent(QFocusEvent* event)
	{
		m_focused = true;
	}

	void QGraphicsConnectPointItem::focusOutEvent(QFocusEvent* event)
	{
		m_focused = false;
	}

	void QGraphicsConnectPointItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
	{
		QGraphicsEllipseItem::hoverEnterEvent(event);
	}

	void QGraphicsConnectPointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
	{
		QGraphicsEllipseItem::hoverLeaveEvent(event);
	}

	void QGraphicsConnectPointItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsItem* item = scene()->itemAt(event->scenePos(), QTransform());
		if (item == this)
		{
			// begin connection
			QGraphicsFlowScene* flowScene = dynamic_cast<QGraphicsFlowScene*>(scene());
			if(flowScene)
				flowScene->beginConnect();
		}
	}

	void QGraphicsConnectPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsEllipseItem::mouseReleaseEvent(event);
	}

	void QGraphicsConnectPointItem::keyPressEvent(QKeyEvent* event)
	{
	}

	void QGraphicsConnectPointItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsEllipseItem::mouseMoveEvent(event);

		//if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() > QApplication::startDragDistance())
		//{
		//	QMimeData* mimeData = new  QMimeData;
		//	mimeData->setData("drag/render-queue", QByteArray::number(m_objectId));
		//	QDrag* drag = new QDrag(event->widget());
		//	drag->setMimeData(mimeData);
		//	drag->setPixmap(Echo::QGraphicsItemToPixmap(this, 0.7f));
		//	QPoint hotSpot = (event->buttonDownScenePos(Qt::LeftButton).toPoint() - sceneBoundingRect().topLeft().toPoint());
		//	//drag->setHotSpot(hotSpot);
		//	drag->exec();
		//}
	}

	bool QGraphicsConnectPointItem::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
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
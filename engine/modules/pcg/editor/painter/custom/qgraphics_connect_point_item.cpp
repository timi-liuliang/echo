#include "qgraphics_connect_point_item.h"

#ifdef ECHO_EDITOR_MODE

#include "qgraphics_flow_scene.h"

namespace Procedural
{
	QGraphicsConnectPointItem::QGraphicsConnectPointItem(Echo::PCGConnectPoint* connectPoint)
		: QGraphicsEllipseItem()
		, m_connectPoint(connectPoint)
	{
		setZValue(10.f);
	}

	QGraphicsConnectPointItem::~QGraphicsConnectPointItem()
	{

	}

	void QGraphicsConnectPointItem::focusInEvent(QFocusEvent* event)
	{
	}

	void QGraphicsConnectPointItem::focusOutEvent(QFocusEvent* event)
	{
	}

	void QGraphicsConnectPointItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
	{
		QGraphicsEllipseItem::hoverEnterEvent(event);
	}

	void QGraphicsConnectPointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
	{
		QGraphicsEllipseItem::hoverLeaveEvent(event);
	}
}

#endif
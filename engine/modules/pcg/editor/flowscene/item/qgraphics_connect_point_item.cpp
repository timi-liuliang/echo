#include "qgraphics_connect_point_item.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	static std::unordered_map<Echo::PCGConnectPoint*, QGraphicsConnectPointItem*> g_connectPointItems;

	QGraphicsConnectPointItem::QGraphicsConnectPointItem(Echo::PCGConnectPoint* connectPoint)
		: QGraphicsEllipseItem()
		, m_connectPoint(connectPoint)
	{
		setZValue(10.f);

		g_connectPointItems[connectPoint] = this;
	}

	QGraphicsConnectPointItem::~QGraphicsConnectPointItem()
	{
		auto it = g_connectPointItems.find(m_connectPoint);
		if (it != g_connectPointItems.end())
		{
			g_connectPointItems.erase(it);
		}
	}

	QGraphicsConnectPointItem* QGraphicsConnectPointItem::getByPCGConnectPoint(Echo::PCGConnectPoint* connectPoint)
	{
		auto it = g_connectPointItems.find(connectPoint);
		return it != g_connectPointItems.end() ? it->second : nullptr;
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
#include "qgraphics_connect_point_item.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	static std::unordered_map<Echo::PCGConnectPoint*, QGraphicsConnectPointItem*> g_connectPointItems;

	QGraphicsConnectPointItem::QGraphicsConnectPointItem(Echo::PCGConnectPoint* connectPoint)
		: QGraphicsEllipseItem()
		, m_connectPoint(connectPoint)
	{
		QPen pen(m_connectionPointBorderColor, m_borderWidth);
		QBrush brush(m_filledConnectionPointColorActive);

		setZValue(10.f);
		setPen(pen);
		setBrush(brush);
		setFlag(QGraphicsItem::ItemIsFocusable);
		setFiltersChildEvents(true);

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

	void QGraphicsConnectPointItem::setRadius(float radius)
	{
		m_radius = radius;
		float halfRadius = 0.5f * m_radius;

		setRect(-halfRadius, -halfRadius, m_radius, m_radius);
	}

	void QGraphicsConnectPointItem::setState(State state)
	{ 
		if (m_state != state)
		{
			m_state = state;
			m_stateDirty = true;
		}
	}

	void QGraphicsConnectPointItem::update()
	{
		QBrush brush(m_connectPoint->isHaveConnect() ? m_filledConnectionPointColorActive : m_filledConnectionPointColor);
		setBrush(brush);

		if (m_stateDirty)
		{
			if (m_state == State::Normal)
			{
				float radius = m_radius;
				float halfRadius = 0.5f * radius;
				setRect(-halfRadius, -halfRadius, radius, radius);

				QPen pen(m_connectionPointBorderColor, m_borderWidth, Qt::SolidLine);
				setPen(pen);
			}
			else if (m_state == State::Big)
			{
				float radius = m_radius * 1.2f;
				float halfRadius = 0.5f * radius;
				setRect(-halfRadius, -halfRadius, radius, radius);

				QPen pen(m_connectionPointBorderColor, m_borderWidth, Qt::DotLine);
				setPen(pen);
			}
			else if (m_state == State::Hide)
			{
				float radius = m_radius;
				float halfRadius = 0.5f * radius;
				setRect(-halfRadius, -halfRadius, radius, radius);

				QPen pen(m_filledConnectionPointColor, m_borderWidth, Qt::SolidLine);
				setPen(pen);
			}

			m_stateDirty = false;
		}
	}
}

#endif
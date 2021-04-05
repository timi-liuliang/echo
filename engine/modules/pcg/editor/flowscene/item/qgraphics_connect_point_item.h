#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QMimeData>
#include <QDrag>
#include "engine/modules/pcg/connect/pcg_connect_point.h"

namespace Procedural
{
	class QGraphicsConnectPointItem : public QGraphicsEllipseItem
	{
	public:
		QGraphicsConnectPointItem(Echo::PCGConnectPoint* connectPoint);
		~QGraphicsConnectPointItem();

		// get item
		static QGraphicsConnectPointItem* getByPCGConnectPoint(Echo::PCGConnectPoint* connectPoint);

		// connect point
		Echo::PCGConnectPoint* getPCGConnectPoint() { return m_connectPoint; }

		// radius
		float getRadius() const { return m_radius; }
		void setRadius(float radius);

		// update
		void update();

	protected:
		// foucs event
		virtual void focusInEvent(QFocusEvent* event);
		virtual void focusOutEvent(QFocusEvent* event);

		// hover event
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

	protected:
		Echo::PCGConnectPoint*		m_connectPoint = nullptr;
		QColor						m_connectionPointBorderColor = QColor(132, 132, 132);
		QColor						m_filledConnectionPointColor = QColor(77, 77, 77, 0);
		QColor						m_filledConnectionPointColorActive = Qt::darkCyan;
		float						m_borderWidth = 2.f;
		float						m_radius = 7.f;
	};
}

#endif

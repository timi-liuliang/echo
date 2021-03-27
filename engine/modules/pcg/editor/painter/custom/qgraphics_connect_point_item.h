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

	protected:
		// foucs event
		virtual void focusInEvent(QFocusEvent* event);
		virtual void focusOutEvent(QFocusEvent* event);

		// hover event
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

	protected:
		Echo::PCGConnectPoint*				m_connectPoint = nullptr;
	};
}

#endif

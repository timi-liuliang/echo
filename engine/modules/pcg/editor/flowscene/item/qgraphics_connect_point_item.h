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
		// state
		enum State
		{
			Normal,
			Big,
			Hide,
		};

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

		// state
		void setState(State state);

		// update
		void update();

	protected:
		Echo::PCGConnectPoint*		m_connectPoint = nullptr;
		QColor						m_connectionPointBorderColor = QColor(132, 132, 132);
		QColor						m_filledConnectionPointColor = QColor(77, 77, 77, 0);
		QColor						m_filledConnectionPointColorActive = Qt::darkCyan;
		float						m_borderWidth = 1.6f;
		float						m_radius = 6.f;
		State						m_state = State::Normal;
		bool						m_stateDirty = false;
	};
}

#endif

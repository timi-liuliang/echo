#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QApplication>
#include <QGraphicsPathItem>
#include <QMimeData>
#include <QDrag>

namespace Procedural
{
	class QGraphicsConnectItem : public QGraphicsPathItem
	{
	public:
		QGraphicsConnectItem();
		~QGraphicsConnectItem();

		// set
		void set(const QPointF& startPos, const QPointF& endPos);

	protected:
		// foucs event
		virtual void focusInEvent(QFocusEvent* event);
		virtual void focusOutEvent(QFocusEvent* event);

		// hover event
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

		// mouse press event
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

		// key press event
		virtual void keyPressEvent(QKeyEvent* event) override;

		// mouse move event
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

		// scene event filter
		virtual bool sceneEventFilter(QGraphicsItem* watched, QEvent* event);

	protected:
		bool			m_focused = false;
		QPointF			m_startPos = QPointF(1e30f, 1e30f);
		QPointF			m_endPos = QPointF(1e30f, 1e30f);
	};
}

#endif

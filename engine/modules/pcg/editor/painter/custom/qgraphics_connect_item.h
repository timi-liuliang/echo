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
		void set(const Echo::Vector2& startPos, const Echo::Vector2& endPos);

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
		bool				m_focused = false;
		Echo::Vector2		m_startPos = Echo::Vector2::ZERO;
		Echo::Vector2		m_endPos = Echo::Vector2::ZERO;
	};
}

#endif

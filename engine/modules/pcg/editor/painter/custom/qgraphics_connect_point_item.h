#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QMimeData>
#include <QDrag>

namespace Procedural
{
	class QGraphicsConnectPointItem : public QGraphicsEllipseItem
	{
	public:
		QGraphicsConnectPointItem();
		~QGraphicsConnectPointItem();

		// is focused
		bool isFocused() const { return m_focused; }

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
		Echo::ui32							m_objectId = 0;
		bool								m_focused = false;
	};
}

#endif
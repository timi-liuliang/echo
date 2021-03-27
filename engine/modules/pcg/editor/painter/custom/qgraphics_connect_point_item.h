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

		// hover event callback
		void setHoverEnterEventCb(std::function<void(QGraphicsItem*)> cb) { m_hoverEnterEventCb = cb; }
		void setHoverEnterLeaveCb(std::function<void(QGraphicsItem*)> cb) { m_hoverLeaveEventCb = cb; }

		// mouse event cb
		void setMousePressEventCb(std::function<void(QGraphicsItem*)> cb) { m_mousePressEventCb = cb; }

		// key event cb
		void setKeyPressEventCb(std::function<void(QKeyEvent*)> cb) { m_keyEventCb = cb; }

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
		std::function<void(QGraphicsItem*)> m_hoverEnterEventCb;
		std::function<void(QGraphicsItem*)> m_hoverLeaveEventCb;
		std::function<void(QGraphicsItem*)> m_mousePressEventCb;
		std::function<void(QKeyEvent*)>		m_keyEventCb;
	};
}

#endif

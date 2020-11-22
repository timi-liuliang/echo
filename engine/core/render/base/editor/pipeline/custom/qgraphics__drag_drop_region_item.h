#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QApplication>
#include <QMimeData>
#include <QDrag>

namespace Pipeline
{
	class QGraphicsDragDropRegionItem : public QGraphicsPathItem
	{
	public:
		QGraphicsDragDropRegionItem(QGraphicsItem* parent = nullptr)
			: QGraphicsPathItem(parent)
		{}

		// is drop enter
		bool isDropEnter() const { return m_dropEnter; }

		// hover event callback
		void setHoverEnterEventCb(std::function<void(QGraphicsItem*)> cb) { m_hoverEnterEventCb = cb;}
		void setHoverEnterLeaveCb(std::function<void(QGraphicsItem*)> cb) { m_hoverLeaveEventCb = cb; }

		// mouse event cb
		void setMousePressEventCb(std::function<void(QGraphicsItem*)> cb) { m_mousePressEventCb = cb; }

		// key event cb
		void setKeyPressEventCb(std::function<void(QKeyEvent*)> cb) { m_keyEventCb = cb; }

	protected:
		virtual void focusInEvent(QFocusEvent* event)
		{
			m_focused = true;
		}

		virtual void focusOutEvent(QFocusEvent* event)
		{
			m_focused = false;
		}

		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsPathItem::hoverEnterEvent(event);

			if (m_hoverEnterEventCb)
				m_hoverEnterEventCb(this);
		}

		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
		{
			QGraphicsPathItem::hoverLeaveEvent(event);

			if (m_hoverLeaveEventCb)
				m_hoverLeaveEventCb(this);
		}

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override
		{
			QGraphicsItem* item = scene()->itemAt(event->scenePos(), QTransform());
			if (item == this)
			{
				if (m_mousePressEventCb)
					m_mousePressEventCb(this);
			}
		}

		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
		{
			QGraphicsPathItem::mouseReleaseEvent(event);

		}

		virtual void keyPressEvent(QKeyEvent* event) override
		{
			if (m_keyEventCb)
				m_keyEventCb(event);
		}

		virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override
		{
			if (event->mimeData()->hasFormat("drag/render-stage"))
			{
				m_dropEnter = true;
			}
		}

		virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override
		{
			m_dropEnter = false;
		}

		virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override
		{
			if (event->mimeData()->hasFormat("drag/render-stage"))
			{

			}
		}

		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
		{
			QGraphicsPathItem::mouseMoveEvent(event);

			if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() > QApplication::startDragDistance())
			{
				QMimeData* mimeData = new  QMimeData;
				mimeData->setData("drag/render-stage", QByteArray());
				QDrag* drag = new QDrag(mimeData);
				if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
				{
				}
			}
		}

	protected:
		// position changed etc...
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value)
		{
			return QGraphicsPathItem::itemChange(change, value);
		}

	protected:
		bool								m_dropEnter = false;
		bool								m_focused = false;
		std::function<void(QGraphicsItem*)> m_hoverEnterEventCb;
		std::function<void(QGraphicsItem*)> m_hoverLeaveEventCb;
		std::function<void(QGraphicsItem*)> m_mousePressEventCb;
		std::function<void(QKeyEvent*)>		m_keyEventCb;
	};
}

#endif

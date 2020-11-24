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
		QGraphicsDragDropRegionItem(QGraphicsItem* parent)
			: QGraphicsPathItem(parent)
		{}

		// hover event callback
		void setDragEnterCb(std::function<bool(QGraphicsSceneDragDropEvent*)> cb) { m_dragEnterCb = cb; }
		void setDragDropCb(std::function<void(QGraphicsSceneDragDropEvent*)> cb) { m_dragDropCb = cb; }

		// is drop enter
		bool isDropEnter() const { return m_dropEnter; }

	protected:
		// drag enter
		virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override
		{
			m_dropEnter = m_dragEnterCb(event);
		}

		// drag leave
		virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override
		{
			m_dropEnter = false;
		}

		// drop
		virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override
		{
			m_dragDropCb(event);
			m_dropEnter = false;
		}

	protected:
		bool												m_dropEnter = false;
		std::function<bool(QGraphicsSceneDragDropEvent*)>	m_dragEnterCb;
		std::function<void(QGraphicsSceneDragDropEvent*)>	m_dragDropCb;
	};
}

#endif

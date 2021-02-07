#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <engine/core/editor/qt/QWidget.h>

namespace Pipeline
{
	class QGraphicsSceneEx : public QGraphicsScene
	{
	public:
		// mouse event cb
		void setMousePressEventCb(std::function<void()> cb) { m_mousePressEventCb = cb; }

	protected:
		// mouse release event
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
		{
			QGraphicsItem* item = this->itemAt(event->scenePos(), QTransform());
			if (!item)
			{
				if (m_mousePressEventCb)
					m_mousePressEventCb();
			}
		}

		// wheel event
		virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override
		{
			QGraphicsScene::wheelEvent(event);
		}

	protected:
		std::function<void()> m_mousePressEventCb;
	};
}
#endif

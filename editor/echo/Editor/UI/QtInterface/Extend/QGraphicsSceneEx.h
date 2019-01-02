#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <engine/core/editor/qt/QWidget.h>
#include "QMessageHandler.h"

namespace Echo
{
	class QGraphicsSceneEx : public QGraphicsScene
	{
	protected:
		virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override
		{
			QGraphicsScene::wheelEvent(event);

			QMessageHandler::instance()->getEvent(this).graphicsSceneWheelEvent.delta = event->delta();
			QMessageHandler::instance()->onReceiveQObjectMessage(this, QSIGNAL(wheelEvent(QGraphicsSceneWheelEvent*)));
		}
	};
}

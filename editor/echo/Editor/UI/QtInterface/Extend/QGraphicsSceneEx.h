#pragma once

#include <QGraphicsScene>
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

			QMessageHandler::instance()->onReceiveQObjectMessage(this, QSIGNAL(wheelEvent(QGraphicsSceneWheelEvent*)));
		}
	};
}

#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a QGraphicsScene
	QObject* qGraphicsSceneNew();

	// add line
	QGraphicsLineItem* qGraphicsSceneAddLine(QObject* scene, float startX, float startY, float endX, float endY);
}

#endif
#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a QGraphicsScene
	QObject* qGraphicsSceneNew();

	// add line
	QGraphicsLineItem* qGraphicsSceneAddLine(QObject* scene, float startX, float startY, float endX, float endY, const Color& color);

	// add simple text
	QGraphicsItem* qGraphicsSceneAddSimpleText(QObject* scene, const char* txt, const Color& color);
}

#endif
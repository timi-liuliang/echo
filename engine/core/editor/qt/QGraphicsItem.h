#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get item width
	float qGraphicsItemWidth(QGraphicsItem* item);

	// set pos
	void qGraphicsItemSetPos(QGraphicsItem* item, float posX, float posY);

	// set visible
	void qGraphicsItemSetVisible(QGraphicsItem* item, bool visible);
}

#endif
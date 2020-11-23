#pragma once

#include "QWidget.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get sender item
	typedef QGraphicsItem* (*qSenderItemFun)();

	// to pixmap
	QPixmap QGraphicsItemToPixmap(QGraphicsItem* item, float alpha = 1.f);
}

#endif

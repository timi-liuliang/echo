#pragma once

#include "QWidget.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get sender item
	typedef QGraphicsItem* (*qSenderItemFun)();

	// get rect
	typedef bool (*qGraphicsItemSceneRectFun)(QGraphicsItem* item, Rect& rect);

	// get item width
	typedef float (*qGraphicsItemWidthFun)(QGraphicsItem* item);

	// pos
	typedef Vector2 (*qGraphicsItemPosFun)(QGraphicsItem* item);
	typedef void (*qGraphicsItemSetPosFun)(QGraphicsItem* item, float posX, float posY);

	// set pos
	typedef void (*qGraphicsProxyWidgetSetPosFun)(QGraphicsProxyWidget* item, float posX, float posY);

	// set zvalue
	typedef void (*qGraphicsProxyWidgetSetZValueFun)(QGraphicsProxyWidget* item, float zValue);
	typedef void (*qGraphicsItemSetZValueFun)(QGraphicsItem* item, float zValue);

	// set visible
	typedef void (*qGraphicsItemSetVisibleFun)(QGraphicsItem* item, bool visible);

	// user data
	typedef String (*qGraphicsItemUserDataFun)(QGraphicsItem* item);

	// set user data
	typedef void (*qGraphicsItemSetUserDataFun)(QGraphicsItem* item, const char* userData);

	// set tool tip
	typedef void (*qGraphicsItemSetToolTipFun)(QGraphicsItem* item, const char* toolTip);

	// set moveable
	typedef void (*qGraphicsItemSetMoveableFun)(QGraphicsItem* item, bool isMoveable);
}

#endif

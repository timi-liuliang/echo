#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get sender item
	QGraphicsItem* qSenderItem();

	// get item width
	float qGraphicsItemWidth(QGraphicsItem* item);

	// pos
	Vector2 qGraphicsItemPos(QGraphicsItem* item);
	void qGraphicsItemSetPos(QGraphicsItem* item, float posX, float posY);

	// set pos
	void qGraphicsProxyWidgetSetPos(QGraphicsProxyWidget* item, float posX, float posY);

	// set zvalue
	void qGraphicsProxyWidgetSetZValue(QGraphicsProxyWidget* item, float zValue);

	// set visible
	void qGraphicsItemSetVisible(QGraphicsItem* item, bool visible);

	// user data
	String qGraphicsItemUserData(QGraphicsItem* item);

	// set user data
	void qGraphicsItemSetUserData(QGraphicsItem* item, const char* userData);

	// set tool tip
	void qGraphicsItemSetToolTip(QGraphicsItem* item, const char* toolTip);

	// set moveable
	void qGraphicsItemSetMoveable(QGraphicsItem* item, bool isMoveable);
}

#endif
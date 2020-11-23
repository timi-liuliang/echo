#pragma once

#include "QWidget.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get sender item
	typedef QGraphicsItem* (*qSenderItemFun)();

	// set parent
	typedef void (*qGrphicsItemSetParentItemFun)(QGraphicsItem* current, QGraphicsItem* parent);

	// get rect
	typedef bool (*qGraphicsItemSceneRectFun)(QGraphicsItem* item, Rect& rect);

	// get item width
	typedef float (*qGraphicsItemWidthFun)(QGraphicsItem* item);

	// set pos
	typedef void (*qGraphicsProxyWidgetSetPosFun)(QGraphicsProxyWidget* item, float posX, float posY);

	// set zvalue
	typedef void (*qGraphicsProxyWidgetSetZValueFun)(QGraphicsProxyWidget* item, float zValue);
	typedef void (*qGraphicsItemSetZValueFun)(QGraphicsItem* item, float zValue);

	// user data
	typedef String (*qGraphicsItemUserDataFun)(QGraphicsItem* item);

	// set user data
	typedef void (*qGraphicsItemSetUserDataFun)(QGraphicsItem* item, const char* userData);

	// set tool tip
	typedef void (*qGraphicsItemSetToolTipFun)(QGraphicsItem* item, const char* toolTip);

	// to pixmap
	static inline QPixmap QGraphicsItemToPixmap(QGraphicsItem* item, float alpha=1.f)
	{
		QRectF boundingRect = item->sceneBoundingRect();
		QPixmap pixmap(boundingRect.size().toSize());
		pixmap.fill(Qt::transparent);
		if (item->scene())
		{
			QPainter painter(&pixmap);
			painter.setOpacity(alpha);
			painter.setRenderHint(QPainter::Antialiasing);
			item->scene()->render(&painter, QRectF(), boundingRect);
			painter.end();
		}

		return pixmap;
	}
}

#endif

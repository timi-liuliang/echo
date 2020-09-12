#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a QGraphicsScene
	typedef QObject* (*qGraphicsSceneNewFun)();

	// add widget to QGraphicsScene
	typedef QGraphicsProxyWidget* (*qGraphicsSceneAddWidgetFun)(QObject* scene, QWidget* widget);

	// add line
	typedef QGraphicsItem* (*qGraphicsSceneAddLineFun)(QObject* scene, float startX, float startY, float endX, float endY, const Color& color);

	// add rect
	typedef QGraphicsItem* (*qGraphicsSceneAddRectFun)(QObject* scene, float left, float top, float width, float height, const Color& pen, const Color& brush);

	// add path
	typedef QGraphicsItem* (*qGraphicsSceneAddPathFun)(QObject* scene, const vector<Vector2>::type paths, float widthf, const Color& color);

	// add simple text
	typedef QGraphicsItem* (*qGraphicsSceneAddSimpleTextFun)(QObject* scene, const char* txt, const Color& color);

	// add eclipse
	typedef QGraphicsItem* (*qGraphicsSceneAddEclipseFun)(QObject* scene, float left, float top, float width, float height, const Color& color);

	// add pixmap
	typedef QGraphicsItem* (*qGraphicsSceneAddPixmapFun)(QObject* scene, const char* fileName);

	// remove item
	typedef void (*qGraphicsSceneDeleteWidgetFun)(QObject* scene, QGraphicsProxyWidget* widget);

	// delete item
	typedef void (*qGraphicsSceneDeleteItemFun)(QObject* scene, QGraphicsItem* item);
}

#endif

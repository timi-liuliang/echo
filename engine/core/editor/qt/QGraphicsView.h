#pragma once

#include "QWidget.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// disable viewport update
	typedef void (*qGraphicsViewDisableViewportAutoUpdateFun)(QWidget* graphicsView);

	// get view port
	typedef void (*qGraphicsViewSceneRectFun)(QWidget* view, Rect& rect);

	// create a QGraphicsScene
	typedef void (*qGraphicsViewSetSceneFun)(QWidget* graphicsView, QObject* graphicsScene);

	// scale
	typedef void (*qGraphicsViewSetScaleFun)(QWidget* view, float scaleX, float scaleY);

	// set background brush
	typedef void (*qGraphicsViewSetBackgroundBrushFun)(QWidget* view, const Color& color);
}

#endif

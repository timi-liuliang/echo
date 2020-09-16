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

	// set background brush
	typedef void (*qGraphicsViewSetBackgroundBrushFun)(QWidget* view, const Color& color);

	// fit in view
	typedef void (*qGraphicsViewFitInViewFun)(QWidget* view, Rect& rect);
}

#endif

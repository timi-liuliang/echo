#pragma once

#include "QWidget.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get view port
	typedef void (*qGraphicsViewSceneRectFun)(QWidget* view, Rect& rect);

	// fit in view
	typedef void (*qGraphicsViewFitInViewFun)(QWidget* view, Rect& rect);
}

#endif

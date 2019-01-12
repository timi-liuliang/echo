#pragma once

#include "QWidget.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// disable viewport update
	void qGraphicsViewDisableViewportAutoUpdate(QWidget* graphicsView);

	// get view port
	void qGraphicsViewSceneRect(QWidget* view, Rect& rect);

	// create a QGraphicsScene
	void qGraphicsViewSetScene(QWidget* graphicsView, QObject* graphicsScene);

	// scale
	void qGraphicsViewSetScale(QWidget* view, float scaleX, float scaleY);
}

#endif
#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// disable viewport update
	void qGraphicsViewDisableViewportAutoUpdate(QWidget* graphicsView);

	// get view port
	void qGraphicsViewport(QWidget* view, float& left, float& top, float& width, float& height);

	// create a QGraphicsScene
	void qGraphicsViewSetScene(QWidget* graphicsView, QObject* graphicsScene);

	// scale
	void qGraphicsViewSetScale(QWidget* view, float scaleX, float scaleY);
}

#endif
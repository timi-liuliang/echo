#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a QGraphicsScene
	void qGraphicsViewSetScene(QWidget* graphicsView, QObject* graphicsScene);
}

#endif
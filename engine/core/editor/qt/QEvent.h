#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	struct qEventAll
	{
		// QGraphicsWheelEvent
		struct qGraphicsSceneWheelEvent 
		{
			int	delta = 0;
		}	graphicsSceneWheelEvent;

		// Mouse Event
		struct qGraphicsSceneMouseEvent
		{
			Vector2 scenePos = Vector2::ZERO;
		} graphicsSceneMouseEvent;
	};

	// query widget's qEventAll
	typedef const qEventAll& (*qObjectGetEventAllFun)(QObject* sender);
	typedef const qEventAll& (*qGraphicsItemGetEventAllFun)(QGraphicsItem* sender);
}

#endif

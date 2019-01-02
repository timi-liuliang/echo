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
	const qEventAll& qGetEventAll(QObject* sender);
	const qEventAll& qGetEventAll(QGraphicsItem* sender);
}

#endif
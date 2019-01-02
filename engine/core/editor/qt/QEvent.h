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
	};

	// query widget's qEventAll
	const qEventAll& qGetEventAll(QObject* sender);
}

#endif
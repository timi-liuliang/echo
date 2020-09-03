#pragma once

#include "gesture_recognizer.h"

namespace Echo
{
	class UiPanGestureRecognizer : public UiGestureRecognizer
	{
		ECHO_CLASS(UiGestureRecognizer, UiGestureRecognizer)

	public:
		UiPanGestureRecognizer();
		virtual ~UiPanGestureRecognizer();
	};
}
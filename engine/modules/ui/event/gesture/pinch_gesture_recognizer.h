#pragma once

#include "gesture_recognizer.h"

namespace Echo
{
	class UiPinchGestureRecognizer : public UiGestureRecognizer
	{
		ECHO_CLASS(UiPinchGestureRecognizer, UiGestureRecognizer)

	public:
		UiPinchGestureRecognizer();
		virtual ~UiPinchGestureRecognizer();
	};
}
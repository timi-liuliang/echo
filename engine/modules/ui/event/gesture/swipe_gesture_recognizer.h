#pragma once

#include "gesture_recognizer.h"

namespace Echo
{
	class UiSwipeGestureRecognizer : public UiGestureRecognizer
	{
		ECHO_CLASS(UiSwipeGestureRecognizer, UiGestureRecognizer)

	public:
		UiSwipeGestureRecognizer();
		virtual ~UiSwipeGestureRecognizer();
	};
}
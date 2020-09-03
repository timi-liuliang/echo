#pragma once

#include "gesture_recognizer.h"

namespace Echo
{
	class UiLongPressGestureRecognizer : public UiGestureRecognizer
	{
		ECHO_CLASS(UiGestureRecognizer, UiGestureRecognizer)

	public:
		UiLongPressGestureRecognizer();
		virtual ~UiLongPressGestureRecognizer();
	};
}
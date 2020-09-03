#pragma once

#include "gesture_recognizer.h"

namespace Echo
{
	class UiRotationGestureRecognizer : public UiGestureRecognizer
	{
		ECHO_CLASS(UiRotationGestureRecognizer, UiGestureRecognizer)

	public:
		UiRotationGestureRecognizer();
		virtual ~UiRotationGestureRecognizer();
	};
}
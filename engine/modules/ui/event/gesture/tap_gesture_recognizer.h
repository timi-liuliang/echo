#pragma once

#include "gesture_recognizer.h"

namespace Echo
{
	class UiTapGestureRecognizer : public UiGestureRecognizer
	{
		ECHO_CLASS(UiTapGestureRecognizer, UiGestureRecognizer)

	public:
		UiTapGestureRecognizer();
		virtual ~UiTapGestureRecognizer();
	};
}
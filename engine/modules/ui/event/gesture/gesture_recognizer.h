#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class UiGestureRecognizer : public Object
	{
		ECHO_CLASS(UiGestureRecognizer, Object)

	public:
		UiGestureRecognizer();
		virtual ~UiGestureRecognizer();
	};
}
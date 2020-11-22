#pragma once

#include "engine/core/base/object.h"
#include "engine/core/util/Array.hpp"

namespace Echo
{
	struct DragEvent : public Object
	{
		ECHO_CLASS(DragEvent, Object)

	public:
		DragEvent();
		~DragEvent();

	private:
	};
}
#pragma once

#include "engine/core/base/echo_def.h"

namespace Echo
{
	class IVideoCapture
	{
	public:
		// Start
		virtual void start() {}

		// Lock
		virtual void lockFrame(void*& buffer, i32& bufferLen) {}

		// Unlock
		virtual void unlockFrame() {}
	};
}
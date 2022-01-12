#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/render/base/image/pixel_format.h"

namespace Echo
{
	class IVideoCapture
	{
	public:
		// Start
		virtual void start() {}

		// Lock
		virtual bool lockFrame(void*& buffer, i32& width, i32& height, PixelFormat& format, i32& bufferLen) { return false; }

		// Unlock
		virtual void unlockFrame() {}
	};
}
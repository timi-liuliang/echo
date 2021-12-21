#pragma once

#include "video_capture_interface.h"

#ifdef ECHO_PLATFORM_WINDOWS

namespace Echo
{
	class VideCaptureDShow : public IVideoCapture
	{
	public:
		VideCaptureDShow();
		virtual ~VideCaptureDShow();

		// Start
		void Start();

	private:
		// Device
		void ChooseDevice();
	};
}

#endif
#include "video_capture.h"
#include "engine/core/main/Engine.h"
#include "engine/core/io/IO.h"
#include "capture/video_capture_dshow.h"

namespace Echo
{
	VideoCapture::VideoCapture()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		m_implement = EchoNew(VideCaptureDShow);
#endif
	}

	VideoCapture::~VideoCapture()
	{
	}

	void VideoCapture::bindMethods()
	{
	}
}

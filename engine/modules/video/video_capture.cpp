#include "video_capture.h"
#include "engine/core/main/Engine.h"
#include "engine/core/io/IO.h"
#include "capture/dshow/video_capture_dshow.h"

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
		CLASS_BIND_METHOD(VideoCapture, openDevice);
		CLASS_BIND_METHOD(VideoCapture, getRenderTarget);
		CLASS_BIND_METHOD(VideoCapture, setRenderTarget);

		CLASS_REGISTER_PROPERTY(VideoCapture, "RenderTarget", Variant::Type::ResourcePath, getRenderTarget, setRenderTarget);
	}

	void VideoCapture::openDevice()
	{
		if (m_implement)
		{
			m_implement->start();
		}
	}

	void VideoCapture::setRenderTarget(const ResourcePath& res)
	{
		if (m_renderTarget.setPath(res.getPath()))
		{

		}
	}

	void VideoCapture::updateInternal(float elapsedTime)
	{
		if (m_implement)
		{
			void* buffer = nullptr;
			i32   bufferLen = 0;

			m_implement->lockFrame(buffer, bufferLen);
			m_implement->unlockFrame();
		}
	}
}

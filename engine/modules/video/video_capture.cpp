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
		if (m_renderTargetResPath.setPath(res.getPath()))
		{
			m_renderTarget = ECHO_DOWN_CAST<TextureRenderTarget2D*>(Res::get(res));
		}
	}

	void VideoCapture::updateInternal(float elapsedTime)
	{
		if (m_implement && m_renderTarget)
		{
			void*		buffer = nullptr;
			i32			bufferLen = 0;
			i32			width = 0;
			i32			height = 0;
			PixelFormat format;

			if (m_implement->lockFrame(buffer, width, height, format, bufferLen))
			{
				m_renderTarget->updateTexture2D(format, Texture::TU_GPU_READ, width, height, buffer, bufferLen);
			}
			m_implement->unlockFrame();
		}
	}
}

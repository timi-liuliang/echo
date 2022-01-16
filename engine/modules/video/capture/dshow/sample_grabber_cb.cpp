#include "sample_grabber_cb.h"
#include "engine/core/render/base/image/pixel_util.h"

#ifdef ECHO_PLATFORM_WINDOWS

namespace Echo
{
	SampleGrabberCallback::SampleGrabberCallback()
	{
	}

	HRESULT SampleGrabberCallback::Initialize(i32 Width, i32 Height, ui8 bytesPerPixel, AM_MEDIA_TYPE mediaType)
	{
		m_width = Width;
		m_height = Height;
		m_bytesPerPixel = bytesPerPixel;

		m_buffer.resize(m_width * m_height * m_bytesPerPixel);

		return S_OK;
	}

	bool SampleGrabberCallback::lockFrame(void*& buffer, i32& width, i32& height, PixelFormat& format, i32& bufferLen)
	{
		m_mutex.lock();

		buffer = (void*)m_buffer.data();
		bufferLen = (i32)m_buffer.size();
		width = m_width;
		height = m_height;
		format = PF_RGBA8_UNORM;

		return bufferLen == width * height * PixelUtil::GetPixelBytes(format);
	}

	void SampleGrabberCallback::unlockFrame()
	{
		m_mutex.unlock();
	}

	HRESULT STDMETHODCALLTYPE SampleGrabberCallback::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (NULL == ppvObject) 
			return E_POINTER;

		if (riid == __uuidof(IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(this);
			return S_OK;
		}

		if (riid == IID_ISampleGrabberCB)
		{
			*ppvObject = static_cast<ISampleGrabberCB*>(this);
			return S_OK;
		}

		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE SampleGrabberCallback::SampleCB(double Time, IMediaSample* pSample)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE SampleGrabberCallback::BufferCB(double Time, BYTE* pBuffer, long BufferLen)
	{
		if (!pBuffer)
			return E_POINTER;

		if (m_buffer.size() == BufferLen)
		{
			EE_LOCK_MUTEX(m_mutex);

			if (m_upsideDown)
			{
				// https://microsoft.public.win32.programmer.directx.video.narkive.com/fySNYIqF/output-video-upside-down
				i32 bufferLenPerRow = BufferLen / m_height;
				for (i32 i = 0; i < m_height; i++)
				{
					ui8* srcPtr = pBuffer + (m_height-i-1) * bufferLenPerRow;
					ui8* dstPtr = m_buffer.data() + i * bufferLenPerRow;

					if (false)
					{
						//for (i32 j = 0; j < m_width; j++)
						//{
						//	*(dstPtr + j) = *(srcPtr + m_width - j - 1);
						//}
					}
					else
					{
						std::memcpy(dstPtr, srcPtr, bufferLenPerRow);
					}
				}
			}
			else
			{
				std::memcpy(m_buffer.data(), pBuffer, BufferLen);
			}
		}

		return S_OK;
	}
}

#endif
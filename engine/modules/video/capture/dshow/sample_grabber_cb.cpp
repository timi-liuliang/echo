#include "sample_grabber_cb.h"

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

	void SampleGrabberCallback::lockFrame(void*& buffer, i32& bufferLen)
	{
		m_mutex.lock();

		buffer = (void*)m_buffer.data();
		bufferLen = (i32)m_buffer.size();
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
			EE_LOCK_MUTEX(m_mutex)

			std::memcpy(m_buffer.data(), pBuffer, BufferLen);
		}

		return S_OK;
	}
}

#endif
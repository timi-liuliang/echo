#include "sample_grabber_cb.h"

#ifdef ECHO_PLATFORM_WINDOWS

namespace Echo
{
	SampleGrabberCallback::SampleGrabberCallback()
	{
		m_bGetPicture = FALSE;
	}

	ULONG STDMETHODCALLTYPE SampleGrabberCallback::AddRef()
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE SampleGrabberCallback::Release()
	{
		return 2;
	}

	HRESULT STDMETHODCALLTYPE SampleGrabberCallback::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (NULL == ppvObject) return E_POINTER;
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
		if (FALSE == m_bGetPicture)
			return S_FALSE;
		if (!pBuffer)
			return E_POINTER;

		//SaveBitmap(pBuffer, BufferLen);

		m_bGetPicture = FALSE;
		return S_OK;
	}

	BOOL SampleGrabberCallback::SaveBitmap(BYTE* pBuffer, long lBufferSize)
	{
		return TRUE;
	}
}

#endif
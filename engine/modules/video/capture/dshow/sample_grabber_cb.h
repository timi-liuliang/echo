#pragma once

#include "../video_capture_interface.h"
#include "engine/core/thread/Threading.h"

#ifdef ECHO_PLATFORM_WINDOWS

#include <dshow.h>
#include "qedit.h"

namespace Echo
{
	class SampleGrabberCallback : public ISampleGrabberCB
	{
	public:
		SampleGrabberCallback();

		// Initialize
		HRESULT Initialize(i32 Width, i32 Height, ui8 bytesPerPixel, AM_MEDIA_TYPE mediaType);

		// Lock
		bool lockFrame(void*& buffer, i32& width, i32& height, PixelFormat& format, i32& bufferLen);

		// Unlock
		void unlockFrame();

	public:
		ULONG STDMETHODCALLTYPE AddRef() { return 1; }
		ULONG STDMETHODCALLTYPE Release() { return 2; }
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

		HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample* pSample);
		HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE* pBuffer, long BufferLen);

	public:
		EE_MUTEX			(m_mutex);
		bool				m_upsideDown = true;
		i32					m_width;
		i32					m_height;
		ui8					m_bytesPerPixel;
		AM_MEDIA_TYPE		m_mediaType;
		std::vector<ui8>	m_buffer;
	};
}

#endif
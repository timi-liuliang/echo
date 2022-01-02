#pragma once

#include "video_capture_interface.h"

#ifdef ECHO_PLATFORM_WINDOWS

#include <dshow.h>
#include "qedit.h"

class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample* pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE* pBuffer, long BufferLen);

	SampleGrabberCallback();
	BOOL SaveBitmap(BYTE* pBuffer, long lBufferSize);
public:
	BOOL m_bGetPicture;
	long m_lWidth;
	long m_lHeight;
	int  m_iBitCount;
	TCHAR m_chTempPath[MAX_PATH];
	TCHAR m_chSwapStr[MAX_PATH];
	TCHAR m_chDirName[MAX_PATH];
};

#endif
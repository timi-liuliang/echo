#include "video_capture_dshow.h"
#include "engine/core/log/Log.h"

#ifdef ECHO_PLATFORM_WINDOWS

#include <dshow.h>
#include "qedit.h"

#pragma comment(lib, "Strmiids.lib")

namespace Echo
{
	VideCaptureDShow::DeviceInfo::~DeviceInfo()
	{
		if (m_deviceFilter) m_deviceFilter->Release();
		if (m_propertyBag) m_propertyBag->Release();
		if (m_moniker) m_moniker->Release();
	}

	void VideCaptureDShow::DeviceInfo::initialzie()
	{
		auto readProperty = [this](const wchar_t* name, std::string& value)
		{
			VARIANT propValue;
			VariantInit(&propValue);
			m_propertyBag->Read(name, &propValue, 0);
			if (propValue.bstrVal)
				value = StringUtil::WCS2MBS((const wchar_t*)propValue.bstrVal);

			VariantClear(&propValue);
		};

		m_moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&m_propertyBag);
		m_moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_deviceFilter);

		readProperty(L"FriendlyName", m_friendlyName);
		readProperty(L"Description", m_description);
		readProperty(L"DevicePath", m_devicePath);

		m_isVFWCard = isVFWCard(m_deviceFilter);
		m_isWDMCard = isWDMCard(m_deviceFilter);
	}

	bool VideCaptureDShow::DeviceInfo::isVFWCard(IBaseFilter* deviceFilter)
	{
		if (deviceFilter)
		{
			IAMVfwCaptureDialogs* vfwCaptureDialogs = nullptr;
			if (SUCCEEDED(deviceFilter->QueryInterface(IID_IAMVfwCaptureDialogs, (void**)&vfwCaptureDialogs)))
			{
				vfwCaptureDialogs->Release();
				return true;
			}
		}

		return false;
	}

	bool VideCaptureDShow::DeviceInfo::isWDMCard(IBaseFilter* deviceFilter)
	{
		if (deviceFilter)
		{
			IAMAnalogVideoDecoder* analogVideoDecoder = nullptr;
			if (SUCCEEDED(deviceFilter->QueryInterface(IID_IAMAnalogVideoDecoder, (void**)&analogVideoDecoder)))
			{
				analogVideoDecoder->Release();
				return true;
			}
		}

		return false;
	}

	VideCaptureDShow::VideCaptureDShow()
	{
	}

	VideCaptureDShow::~VideCaptureDShow()
	{

	}

	void VideCaptureDShow::start()
	{
		IBaseFilter* baseFilter = nullptr;
		enumCaptureDevices(baseFilter);

		initCaptureGraphBuilder();

		m_grabberCb = EchoNew(SampleGrabberCallback);

		openDevice(0);
	}

	void VideCaptureDShow::enumCaptureDevices(IBaseFilter*& baseFilter)
	{
		HRESULT hr;

		ICreateDevEnum* deviceEnum = nullptr;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&deviceEnum);
		if (SUCCEEDED(hr))
		{
			// An enumerator for the video capture category
			IEnumMoniker* enumMoniker = nullptr;
			hr = deviceEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
			if (S_OK == hr)
			{
				ULONG cFected;
				DeviceInfo* deviceInfo = EchoNew(DeviceInfo);
				enumMoniker->Reset();

				while (enumMoniker->Next(1, &deviceInfo->m_moniker, &cFected) == S_OK)
				{
					deviceInfo->initialzie();
					m_devices.emplace_back(deviceInfo);
				}

				enumMoniker->Release();
			}
			else
			{
				EchoLogError("Enum video capture device failed");
			}

			deviceEnum->Release();
		}
	}

	void VideCaptureDShow::initCaptureGraphBuilder()
	{
		HRESULT hr;

		if (SUCCEEDED(CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&m_capture)))
		{
			if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_graph)))
			{
				// Obtain interfaces for media control and Video Window
				m_graph->QueryInterface(IID_IMediaControl, (LPVOID*)&m_mediaControl);
				m_graph->QueryInterface(IID_IVideoWindow, (LPVOID*)&m_videoWindow);
				m_graph->QueryInterface(IID_IMediaEventEx, (LPVOID*)&m_mediaEvent);

				hr = m_capture->SetFiltergraph(m_graph);
				if (FAILED(hr))
					return;
			}
			else
			{
				m_capture->Release();
			}
		}
	}

	void VideCaptureDShow::openDevice(int deviceID)
	{
		HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&m_grabberFilter);
		if (FAILED(hr))	
			return;

		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_destFilter);
		if (FAILED(hr))
			return;

		IBaseFilter* deviceFilter = m_devices[deviceID]->m_deviceFilter;

		hr = m_graph->AddFilter(deviceFilter, L"Video Filter");
		if (FAILED(hr))
			return;

		hr = m_graph->AddFilter(m_grabberFilter, L"Sample Grabber");
		if (FAILED(hr))
			return;

		hr = m_graph->AddFilter(m_destFilter, L"Null Renderer");
		if (FAILED(hr))
			return;

		hr = m_grabberFilter->QueryInterface(IID_ISampleGrabber, (LPVOID*)&m_grabber);
		if (FAILED(hr))
			return;

		HDC hdc = GetDC(NULL);
		int bitDepth = GetDeviceCaps(hdc, BITSPIXEL);
		ReleaseDC(nullptr, hdc);

		AM_MEDIA_TYPE mediaType;
		ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));

		mediaType.majortype = MEDIATYPE_Video;
		switch (bitDepth)
		{
		case  8:	mediaType.subtype = MEDIASUBTYPE_RGB8;	break;
		case 16:	mediaType.subtype = MEDIASUBTYPE_RGB555;break;
		case 24:	mediaType.subtype = MEDIASUBTYPE_RGB24;	break;
		case 32:	mediaType.subtype = MEDIASUBTYPE_ARGB32;	break;
		default:	break;
		}
		mediaType.formattype = FORMAT_VideoInfo;
		
		hr = m_grabber->SetMediaType(&mediaType);
		if (FAILED(hr))
			return;

		// https://docs.microsoft.com/en-us/previous-versions/ms784859(v=vs.85)
		hr = m_capture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, deviceFilter, m_grabberFilter, m_destFilter);
		if (FAILED(hr))
			return;

		hr = m_grabber->GetConnectedMediaType(&mediaType);
		if (FAILED(hr))
			return;

		VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mediaType.pbFormat;
		hr = m_grabberCb->Initialize(vih->bmiHeader.biWidth, vih->bmiHeader.biHeight, vih->bmiHeader.biBitCount / 8, mediaType);
		if (FAILED(hr))
			return;

		hr = m_grabber->SetCallback(m_grabberCb, 1);
		if (FAILED(hr))
			return;

		hr = m_mediaControl->Run();
		if (FAILED(hr))
			return;

		hr = m_grabber->SetOneShot(FALSE);
		if (FAILED(hr))
			return;
	}

	bool VideCaptureDShow::lockFrame(void*& buffer, i32& width, i32& height, PixelFormat& format, i32& bufferLen)
	{
		if (m_grabberCb)
		{
			return m_grabberCb->lockFrame(buffer, width, height, format, bufferLen);
		}
		else
		{
			buffer = nullptr;
			bufferLen = 0;

			return false;
		}
	}

	void VideCaptureDShow::unlockFrame()
	{
		if(m_grabberCb)
			m_grabberCb->unlockFrame();
	}
}

#endif
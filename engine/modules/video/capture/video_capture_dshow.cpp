#include "video_capture_dshow.h"
#include "engine/core/log/Log.h"

#ifdef ECHO_PLATFORM_WINDOWS

#include <dshow.h>

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
		// Test
		start();
	}

	VideCaptureDShow::~VideCaptureDShow()
	{

	}

	void VideCaptureDShow::start()
	{
		IBaseFilter* baseFilter = nullptr;
		enumCaptureDevices(baseFilter);

		initCaptureGraphBuilder();
	}

	void VideCaptureDShow::enumCaptureDevices(IBaseFilter*& baseFilter)
	{
		HRESULT hr;

		ICreateDevEnum* sysDevEnum = nullptr;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sysDevEnum);
		if (hr == S_OK)
		{
			// An enumerator for the video capture category
			IEnumMoniker* enumMoniker = nullptr;
			hr = sysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
			if (S_OK == hr)
			{
				ULONG cFected;
				DeviceInfo deviceInfo;

				while (enumMoniker->Next(1, &deviceInfo.m_moniker, &cFected) == S_OK)
				{
					deviceInfo.initialzie();
					m_devices.emplace_back(deviceInfo);
				}

				enumMoniker->Release();
			}
			else
			{
				EchoLogError("Enum video capture device failed");
			}

			sysDevEnum->Release();
		}
	}

	void VideCaptureDShow::initCaptureGraphBuilder()
	{
		IGraphBuilder* graphBuilder = nullptr;
		ICaptureGraphBuilder2* captureGraphBuilder2 = nullptr;

		if (SUCCEEDED(CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&captureGraphBuilder2)))
		{
			if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&graphBuilder)))
			{
				captureGraphBuilder2->SetFiltergraph(graphBuilder);
			}
			else
			{
				captureGraphBuilder2->Release();
			}
		}
	}
}

#endif
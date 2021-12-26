#include "video_capture_dshow.h"
#include "engine/core/log/Log.h"

#ifdef ECHO_PLATFORM_WINDOWS

#include <dshow.h>

#pragma comment(lib, "Strmiids.lib")

namespace Echo
{
	VideCaptureDShow::VideCaptureDShow()
	{
		// Test
		Start();
	}

	VideCaptureDShow::~VideCaptureDShow()
	{

	}

	void VideCaptureDShow::Start()
	{
		IBaseFilter* baseFilter = nullptr;
		ChooseDevice(baseFilter);

		int a = 10;
	}

	void VideCaptureDShow::ChooseDevice(IBaseFilter*& baseFilter)
	{
		HRESULT hr;

		ICreateDevEnum* sysDevEnum = nullptr;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sysDevEnum);
		if (hr == S_OK)
		{
			IEnumMoniker* enumMoniker = nullptr;
			hr = sysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
			if (S_OK == hr)
			{
				IMoniker* moniker = nullptr;
				ULONG cFected;

				while (enumMoniker->Next(1, &moniker, &cFected) == S_OK)
				{
					IPropertyBag* propertyBag = nullptr;
					moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&propertyBag);

					hr = moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&baseFilter);
					if (S_OK == hr)
					{
						moniker->Release();
						break;
					}
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
}

#endif
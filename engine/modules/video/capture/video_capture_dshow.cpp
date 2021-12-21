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
		ChooseDevice();
	}

	void VideCaptureDShow::ChooseDevice()
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
					IPropertyBag* propBag = nullptr;
				}
			}
			else
			{
				EchoLogError("Enum video capture device failed");
			}
		}
	}
}

#endif
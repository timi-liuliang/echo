#pragma once

#include "video_capture_interface.h"

#ifdef ECHO_PLATFORM_WINDOWS

#include <dshow.h>

namespace Echo
{
	class VideCaptureDShow : public IVideoCapture
	{
	public:
		// Device info
		struct DeviceInfo
		{
			IMoniker*		m_moniker = nullptr;
			IPropertyBag*	m_propertyBag = nullptr;
			IBaseFilter*	m_filter = nullptr;
			std::string		m_friendlyName;
			std::string		m_description;
			std::string		m_devicePath;

			DeviceInfo() {}
			~DeviceInfo();

			// Read properties
			void initialzie();
		};

	public:
		VideCaptureDShow();
		virtual ~VideCaptureDShow();

		// Start
		void Start();

	private:
		// Device
		void ChooseDevice(class IBaseFilter*& baseFilter);

	private:
		std::vector<DeviceInfo>	m_devices;
	};
}

#endif
#pragma once

#include "sample_grabber_cb.h"

#ifdef ECHO_PLATFORM_WINDOWS

namespace Echo
{
	// https://blog.csdn.net/lijian2017/article/details/80346580?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-0.pc_relevant_paycolumn_v2&spm=1001.2101.3001.4242.1&utm_relevant_index=3
	class VideCaptureDShow : public IVideoCapture
	{
	public:
		// Device info
		struct DeviceInfo
		{
			IMoniker*		m_moniker = nullptr;
			IPropertyBag*	m_propertyBag = nullptr;
			IBaseFilter*	m_deviceFilter = nullptr;
			std::string		m_friendlyName;
			std::string		m_description;
			std::string		m_devicePath;
			bool			m_isVFWCard = false;
			bool			m_isWDMCard = false;

			DeviceInfo() {}
			~DeviceInfo();

			// Read properties
			void initialzie();

			// Check card type
			bool isVFWCard(IBaseFilter* deviceFilter);
			bool isWDMCard(IBaseFilter* deviceFilter);
		};

	public:
		VideCaptureDShow();
		virtual ~VideCaptureDShow();

		// Start
		virtual void start() override;

		// Query Frame
		ui8* queryFrame();

	private:
		// Device
		void enumCaptureDevices(class IBaseFilter*& baseFilter);

		// Init Graph Builder
		void initCaptureGraphBuilder();

		// Open device
		void openDevice(int deviceID);

	private:
		std::vector<DeviceInfo*>m_devices;
		IGraphBuilder*			m_graph = nullptr;
		ICaptureGraphBuilder2*	m_capture = nullptr;
		IMediaControl*			m_mediaControl = nullptr;
		IVideoWindow*			m_videoWindow = nullptr;
		IMediaEventEx*			m_mediaEvent = nullptr;
		IBaseFilter*			m_grabberFilter = nullptr;
		ISampleGrabber*			m_grabber = nullptr;
		SampleGrabberCallback*	m_grabberCb = nullptr;
		IBaseFilter*			m_destFilter = nullptr;
	};
}

#endif
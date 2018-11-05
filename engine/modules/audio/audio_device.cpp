#include "audio_device.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	AudioDevice::AudioDevice()
	{
		m_device = alcOpenDevice(nullptr);
		if (!m_device)
		{
			EchoLogError("open openal device failed.");
		}

		// check device ability
		m_isSupportEnumeration = alcIsExtensionPresent(m_device, "ALC_ENUMERATION_EXT");
	}

	AudioDevice::~AudioDevice()
	{

	}

	void AudioDevice::bindMethods()
	{
	}

	AudioDevice* AudioDevice::instance()
	{
		static AudioDevice* inst = EchoNew(AudioDevice);
		return inst;
	}
}
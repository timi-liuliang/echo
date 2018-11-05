#include "audio_device.h"

namespace Echo
{
	AudioDevice::AudioDevice()
	{

	}

	AudioDevice::~AudioDevice()
	{

	}

	void AudioDevice::bindMethods()
	{
	}

	// instance
	AudioDevice* AudioDevice::instance()
	{
		static AudioDevice* inst = EchoNew(AudioDevice);
		return inst;
	}
}
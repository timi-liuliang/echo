#include "audio_device.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
	AudioDevice::AudioDevice()
		: m_currentListener(nullptr)
	{
		m_device = alcOpenDevice(nullptr);
		if (!m_device)
		{
			EchoLogError("open openal device failed.");
		}

		// check device ability
		m_isSupportEnumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
		if (m_isSupportEnumeration)
		{
			listAudioDevices();
		}

		// context creation and initialization
		m_context = alcCreateContext(m_device, nullptr);
		if (!alcMakeContextCurrent(m_context))
		{
			EchoLogError("make openal context failed.");
		}

	}

	AudioDevice::~AudioDevice()
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_context);
		alcCloseDevice(m_device);
	}

	void AudioDevice::bindMethods()
	{
	}

	AudioDevice* AudioDevice::instance()
	{
		static AudioDevice* inst = EchoNew(AudioDevice);
		return inst;
	}

	// step
	void AudioDevice::step(float elapsedTime)
	{
		if (m_currentListener)
		{

		}
		else
		{
			Camera* camera = NodeTree::instance()->get3dCamera();
			if (camera)
			{
				static Vector3 lastPosition = camera->getPosition();
				const  Vector3& position = camera->getPosition();
				Vector3         velocity = elapsedTime > 1e-5f ? (position - lastPosition) / elapsedTime : Vector3::ZERO;
				const Vector3&  up = camera->getUp();
				const Vector3&  dir = camera->getDirection();

				alListener3f(AL_POSITION, position.x, position.y, position.z);
				alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);

				ALfloat listenOri[] = { dir.x, dir.y, dir.z, up.x, up.y, up.z};
				alListenerfv(AL_ORIENTATION, listenOri);

				lastPosition = position;
			}
		}
	}

	void AudioDevice::listAudioDevices()
	{
		// Passsing nullptr to alcGetString() indicates that we do not 
		// want the device specifier of a particular device, but all of them.
		m_audioDevices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
	}
}
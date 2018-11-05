#pragma once

#include "engine/core/scene/node.h"
#include <al.h>
#include <alc.h>

namespace Echo
{
	class AudioListener;
	class AudioDevice : public Object
	{
		ECHO_SINGLETON_CLASS(AudioDevice, Object)

	public:
		AudioDevice();
		virtual ~AudioDevice();

		// instance
		static AudioDevice* instance();

		// step
		void step(float elapsedTime);

	private:
		// list audio devices
		void listAudioDevices();

	private:
		ALCdevice*			m_device;
		ALCcontext*			m_context;
		bool				m_isSupportEnumeration;
		String				m_audioDevices;
		AudioListener*		m_currentListener;
	};
}
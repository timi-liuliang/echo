#pragma once

#include "engine/core/scene/node.h"
#include <al.h>
#include <alc.h>

namespace Echo
{
	class AudioDevice : public Object
	{
		ECHO_SINGLETON_CLASS(AudioDevice, Object)

	public:
		AudioDevice();
		virtual ~AudioDevice();

		// instance
		static AudioDevice* instance();

	private:
		ALCdevice*			m_device;
		bool				m_isSupportEnumeration;
	};
}
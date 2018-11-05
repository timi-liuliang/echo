#pragma once

#include "engine/core/scene/node.h"

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
	};
}
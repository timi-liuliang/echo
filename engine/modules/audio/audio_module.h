#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class AudioModule : public Module
	{
	public:
		AudioModule();

		// register all types of the module
		virtual void registerTypes();

		// update box2d world
		virtual void update(float elapsedTime);
	};
}

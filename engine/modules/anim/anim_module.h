#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class AnimModule : public Module
	{
	public:
		AnimModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

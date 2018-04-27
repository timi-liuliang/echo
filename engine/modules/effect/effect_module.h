#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class EffectModule : public Module
	{
	public:
		EffectModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

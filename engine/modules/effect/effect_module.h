#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class EffectModule : public Module
	{
		ECHO_SINGLETON_CLASS(EffectModule, Module)

	public:
		EffectModule();

		// instance
		static EffectModule* instance();

		// register all types of the module
		virtual void registerTypes();
	};
}

#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class LightModule : public Module
	{
		ECHO_SINGLETON_CLASS(LightModule, Module)

	public:
		LightModule();
        ~LightModule();

		// instance
		static LightModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

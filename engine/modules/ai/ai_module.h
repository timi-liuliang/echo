#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class AIModule : public Module
	{
		ECHO_SINGLETON_CLASS(AIModule, Module)

	public:
		AIModule();

		// instance
		static AIModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

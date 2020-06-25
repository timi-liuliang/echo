#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class SpineModule : public Module
	{
		ECHO_SINGLETON_CLASS(SpineModule, Module)

	public:
		SpineModule();

		// instance
		static SpineModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

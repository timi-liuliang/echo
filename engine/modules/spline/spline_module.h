#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class SplineModule : public Module
	{
		ECHO_SINGLETON_CLASS(SplineModule, Module)

	public:
		SplineModule();

		// instance
		static SplineModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

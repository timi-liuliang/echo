#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class RecastModule : public Module
	{
		ECHO_SINGLETON_CLASS(RecastModule, Module)

	public:
		RecastModule();

		// instance
		static RecastModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

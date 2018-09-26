#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	// scriptable render pipeline
	class SRPModule : public Module
	{
	public:
		SRPModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

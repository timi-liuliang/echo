#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	// 1. used for light map
	// 2. real time painter
	class SRayModule : public Module
	{
	public:
		SRayModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class SRayModule : public Module
	{
	public:
		SRayModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

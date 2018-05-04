#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class GltfModule : public Module
	{
	public:
		GltfModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

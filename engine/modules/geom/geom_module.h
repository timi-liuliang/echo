#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class GeomModule : public Module
	{
	public:
		GeomModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

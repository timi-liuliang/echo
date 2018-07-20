#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class Box2DModule : public Module
	{
	public:
		Box2DModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

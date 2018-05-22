#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class SpineModule : public Module
	{
	public:
		SpineModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

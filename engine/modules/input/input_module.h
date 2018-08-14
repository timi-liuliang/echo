#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class InputModule : public Module
	{
	public:
		InputModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

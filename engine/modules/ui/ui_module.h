#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class UIModule : public Module
	{
	public:
		UIModule();

		// register all types of the module
		virtual void registerTypes();
	};
}

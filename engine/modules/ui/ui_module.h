#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class UiModule : public Module
	{
	public:
		UiModule();

		// register all types of the module
		virtual void registerTypes() override;

		// get font
	};
}

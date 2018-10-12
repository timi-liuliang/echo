#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class LightModule : public Module
	{
	public:
		LightModule();

		// register all types of the module
		virtual void registerTypes() override;
	};
}
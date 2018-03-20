#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class Live2DModule : public Module
	{
	public:
		Live2DModule();

		virtual void registerTypes();
	};
}

#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class Live2DModule : public Module
	{
		ECHO_SINGLETON_CLASS(Live2DModule, Module)

	public:
		Live2DModule();

		// instance
		static Live2DModule* instance();

		// register all types of the module
		virtual void registerTypes();
	};
}

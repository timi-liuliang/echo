#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class AnimModule : public Module
	{
		ECHO_SINGLETON_CLASS(AnimModule, Module)

	public:
		AnimModule();

		// instance
		static AnimModule* instance();

		// register all types of the module
		virtual void registerTypes();
	};
}

#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class GltfModule : public Module
	{
		ECHO_SINGLETON_CLASS(GltfModule, Module)

	public:
		GltfModule();

		// instance
		static GltfModule* instance();

		// register all types of the module
		virtual void registerTypes();
	};
}

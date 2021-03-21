#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class ProceduralModule : public Module
	{
		ECHO_SINGLETON_CLASS(ProceduralModule, Module)

	public:
		ProceduralModule();

		// instance
		static ProceduralModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

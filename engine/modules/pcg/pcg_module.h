#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class PCGModule : public Module
	{
		ECHO_SINGLETON_CLASS(PCGModule, Module)

	public:
		PCGModule();

		// instance
		static PCGModule* instance();

		// register all types of the module
		virtual void registerTypes();
	};
}

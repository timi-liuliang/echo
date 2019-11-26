#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class GeomModule : public Module
	{
		ECHO_SINGLETON_CLASS(GeomModule, Module)

	public:
		GeomModule();

		// instance
		static GeomModule* instance();

		// register all types of the module
		virtual void registerTypes();
	};
}

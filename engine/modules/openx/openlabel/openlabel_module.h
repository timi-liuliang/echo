#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class OpenLabelModule : public Module
	{
		ECHO_SINGLETON_CLASS(OpenLabelModule, Module)

	public:
		OpenLabelModule();

		// instance
		static OpenLabelModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

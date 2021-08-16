#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class OpenDriveModule : public Module
	{
		ECHO_SINGLETON_CLASS(OpenDriveModule, Module)

	public:
		OpenDriveModule();

		// instance
		static OpenDriveModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}

#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class CameraModule : public Module
	{
		ECHO_SINGLETON_CLASS(CameraModule, Module)

	public:
		CameraModule();

		// instance
		static CameraModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}
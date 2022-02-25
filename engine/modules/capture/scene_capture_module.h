#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class SceneCaptureModule : public Module
	{
		ECHO_SINGLETON_CLASS(SceneCaptureModule, Module)

	public:
		SceneCaptureModule();

		// instance
		static SceneCaptureModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}
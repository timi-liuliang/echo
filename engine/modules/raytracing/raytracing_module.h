#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	// 1. used for light map
	// 2. real time painter
	class RaytracingModule : public Module
	{
		ECHO_SINGLETON_CLASS(RaytracingModule, Module)

	public:
		RaytracingModule();
        ~RaytracingModule();

		// instance
		static RaytracingModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update box2d world
		virtual void update(float elapsedTime) override;
	};
}

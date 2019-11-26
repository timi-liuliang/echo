#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class Box2DModule : public Module
	{
		ECHO_SINGLETON_CLASS(Box2DModule, Module)

	public:
		Box2DModule();
		~Box2DModule();

		// instance
		static Box2DModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update box2d world
		virtual void update(float elapsedTime) override;
	};
}

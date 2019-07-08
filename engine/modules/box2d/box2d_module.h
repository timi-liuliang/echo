#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class Box2DModule : public Module
	{
	public:
		Box2DModule();
		~Box2DModule();

		// register all types of the module
		virtual void registerTypes() override;

		// update box2d world
		virtual void update(float elapsedTime) override;
	};
}

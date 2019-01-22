#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class PhysxModule : public Module 
	{
	public:
		PhysxModule();

		// resister all types of the module
		virtual void registerTypes() override;

		// update physx world
		virtual  void update(float elapsedTime) override;
	};
}
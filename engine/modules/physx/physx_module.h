#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class PhysxModule : public Module 
	{
		ECHO_SINGLETON_CLASS(PhysxModule, Module)

	public:
		PhysxModule();
        virtual ~PhysxModule();

		// instance
		static PhysxModule* instance();

		// resister all types of the module
		virtual void registerTypes() override;

		// update physx world
		virtual  void update(float elapsedTime) override;
	};
}

#include "physx_module.h"
#include "physx_world.h"

namespace Echo
{
	PhysxModule::PhysxModule()
	{

	}

	void PhysxModule::registerTypes()
	{
		Class::registerType<PhysxWorld>();
	}

	void PhysxModule::update(float elapsedTime)
	{
		PhysxWorld::instance()->step(elapsedTime);
	}
}
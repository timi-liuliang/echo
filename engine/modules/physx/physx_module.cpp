#include "physx_module.h"
#include "physx_world.h"
#include "physx_shape.h"
#include "physx_body.h"

namespace Echo
{
	PhysxModule::PhysxModule()
	{

	}

	void PhysxModule::registerTypes()
	{
		Class::registerType<PhysxWorld>();
		Class::registerType<PhysxBody>();
	}

	void PhysxModule::update(float elapsedTime)
	{
		PhysxWorld::instance()->step(elapsedTime);
	}
}
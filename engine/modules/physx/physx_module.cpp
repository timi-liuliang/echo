#include "physx_module.h"
#include "physx_world.h"
#include "shape/physx_shape.h"
#include "shape/physx_shape_sphere.h"
#include "shape/physx_shape_capsule.h"
#include "shape/physx_shape_plane.h"
#include "physx_body.h"

namespace Echo
{
	PhysxModule::PhysxModule()
	{

	}
    
    PhysxModule::~PhysxModule()
    {
        EchoSafeDeleteInstance(PhysxWorld);
    }

	void PhysxModule::registerTypes()
	{
		Class::registerType<PhysxWorld>();
		Class::registerType<PhysxBody>();
		Class::registerType<PhysxShape>();
		Class::registerType<PhysxShapeSphere>();
		Class::registerType<PhysxShapeCapsule>();
		Class::registerType<PhysxShapePlane>();
	}

	void PhysxModule::update(float elapsedTime)
	{
		PhysxWorld::instance()->step(elapsedTime);
	}
}

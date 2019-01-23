#include "physx_shape_sphere.h"
#include "../physx_world.h"

namespace Echo
{
	PhysxShapeSphere::PhysxShapeSphere()
	{

	}

	PhysxShapeSphere::~PhysxShapeSphere()
	{

	}

	void PhysxShapeSphere::bindMethods()
	{

	}

	physx::PxShape* PhysxShapeSphere::createPxShape()
	{
		using namespace physx;

		PxPhysics* physics = PhysxWorld::instance()->getPxPhysics();
		if (physics)
		{
			PxShape* shape = physics->createShape(PxSphereGeometry(m_radius), *m_pxMaterial);
			return shape;
		}

		return nullptr;
	}
}
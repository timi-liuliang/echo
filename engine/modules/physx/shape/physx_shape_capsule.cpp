#include "physx_shape_capsule.h"
#include "../physx_world.h"

namespace Echo
{
	PhysxShapeCapsule::PhysxShapeCapsule()
	{

	}

	PhysxShapeCapsule::~PhysxShapeCapsule()
	{

	}

	void PhysxShapeCapsule::bindMethods()
	{

	}

	physx::PxShape* PhysxShapeCapsule::createPxShape()
	{
		using namespace physx;

		PxPhysics* physics = PhysxWorld::instance()->getPxPhysics();
		if (physics)
		{
			PxShape* shape = physics->createShape(PxCapsuleGeometry(m_radius, m_halfHeight), *m_pxMaterial);
			return shape;
		}

		return nullptr;
	}
}
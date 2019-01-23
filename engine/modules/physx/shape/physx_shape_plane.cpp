#include "physx_shape_plane.h"
#include "../physx_world.h"

namespace Echo
{
	PhysxShapePlane::PhysxShapePlane()
	{

	}

	PhysxShapePlane::~PhysxShapePlane()
	{

	}

	void PhysxShapePlane::bindMethods()
	{

	}

	physx::PxShape* PhysxShapePlane::createPxShape()
	{
		using namespace physx;

		PxPhysics* physics = PhysxWorld::instance()->getPxPhysics();
		if (physics)
		{
			PxShape* shape = physics->createShape(PxPlaneGeometry(), *m_pxMaterial);
			return shape;
		}

		return nullptr;
	}
}
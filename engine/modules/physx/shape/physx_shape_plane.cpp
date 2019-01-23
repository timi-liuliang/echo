#include "physx_shape_plane.h"
#include "../physx_world.h"

namespace Echo
{
	PhysxShapePlane::PhysxShapePlane()
	{
		// set default rotation
		Quaternion quat; quat.fromAxisAngle(Vector3::UNIT_Z, Math::PI_DIV2);
		setLocalOrientation(quat);
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
			physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
			PxShape* shape = physics->createShape(PxPlaneGeometry(), *m_pxMaterial);
			shape->setLocalPose( pxTransform);

			return shape;
		}

		return nullptr;
	}
}
#include "physx_shape_heightfield.h"
#include "../physx_world.h"

namespace Echo
{
	PhysxShapeHeightfield::PhysxShapeHeightfield()
	{
		// set default rotation
		Quaternion quat; quat.fromAxisAngle(Vector3::UNIT_Z, Math::PI_DIV2);
		setLocalOrientation(quat);
	}

	PhysxShapeHeightfield::~PhysxShapeHeightfield()
	{

	}

	void PhysxShapeHeightfield::bindMethods()
	{

	}

	physx::PxShape* PhysxShapeHeightfield::createPxShape()
	{
		using namespace physx;

		PxPhysics* physics = PhysxWorld::instance()->getPxPhysics();
		if (physics)
		{
			physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
			PxShape* shape = physics->createShape(PxHeightFieldGeometry(m_pxHeightField, PxMeshGeometryFlags(), 1.f, 1.f, 1.f), *m_pxMaterial);
			shape->setLocalPose(pxTransform);

			return shape;
		}

		return nullptr;
	}
}
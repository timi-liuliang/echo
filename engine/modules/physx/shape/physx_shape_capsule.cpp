#include "physx_shape_capsule.h"
#include "../physx_module.h"

namespace Echo
{
	PhysxShapeCapsule::PhysxShapeCapsule()
	{
		// set default rotation
		Quaternion quat; quat.fromAxisAngle(Vector3::UNIT_Z, Math::PI_DIV2);
		setLocalOrientation(quat);
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

		PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (physics)
		{
			physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
			PxShape* shape = physics->createShape(PxCapsuleGeometry(m_radius, m_halfHeight), *m_pxMaterial);
			shape->setLocalPose(pxTransform);

			return shape;
		}

		return nullptr;
	}
}
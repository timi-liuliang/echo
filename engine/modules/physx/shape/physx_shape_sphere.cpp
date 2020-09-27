#include "physx_shape_sphere.h"
#include "../physx_module.h"

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

		PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (physics)
		{
			physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
			PxShape* shape = physics->createShape(PxSphereGeometry(m_radius), *m_pxMaterial);
			shape->setLocalPose(pxTransform);

			return shape;
		}

		return nullptr;
	}
}
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
		CLASS_BIND_METHOD(PhysxShapeSphere, getRadius);
		CLASS_BIND_METHOD(PhysxShapeSphere, setRadius);

		CLASS_REGISTER_PROPERTY(PhysxShapeSphere, "Radius", Variant::Type::Real, getRadius, setRadius);
	}

	void PhysxShapeSphere::setRadius(float radius)
	{
		if (m_radius != radius)
		{
			m_radius = radius;

			physx::PxSphereGeometry sphereGeometry;
			if (m_pxShape && m_pxShape->getSphereGeometry(sphereGeometry))
			{
				sphereGeometry.radius = radius;
				m_pxShape->setGeometry(sphereGeometry);
			}
		}
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
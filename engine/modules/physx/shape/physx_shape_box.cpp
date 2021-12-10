#include "physx_shape_box.h"
#include "../physx_module.h"

namespace Echo
{
	PhysxShapeBox::PhysxShapeBox()
	{
		// set default rotation
		Quaternion quat; quat.fromAxisAngle(Vector3::UNIT_Z, Math::PI_DIV2);
		setLocalOrientation(quat);
	}

	PhysxShapeBox::~PhysxShapeBox()
	{

	}

	void PhysxShapeBox::bindMethods()
	{
		CLASS_BIND_METHOD(PhysxShapeBox, getHalfExtents);
		CLASS_BIND_METHOD(PhysxShapeBox, setHalfExtents);

		CLASS_REGISTER_PROPERTY(PhysxShapeBox, "HalfExtents", Variant::Type::Vector3, getHalfExtents, setHalfExtents);
	}

	void PhysxShapeBox::setHalfExtents(const Vector3& halfExtents)
	{
		if (m_halfExtents != halfExtents)
		{
			m_halfExtents = halfExtents;

			physx::PxBoxGeometry geometry;
			if (m_pxShape && m_pxShape->getBoxGeometry(geometry))
			{
				geometry.halfExtents = (physx::PxVec3&)m_halfExtents;
				m_pxShape->setGeometry(geometry);
			}
		}
	}

	physx::PxShape* PhysxShapeBox::createPxShape()
	{
		using namespace physx;

		PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (physics)
		{
			physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
			PxShape* shape = physics->createShape(PxBoxGeometry((physx::PxVec3&)m_halfExtents), *m_pxMaterial);
			shape->setLocalPose(pxTransform);

			return shape;
		}

		return nullptr;
	}
}
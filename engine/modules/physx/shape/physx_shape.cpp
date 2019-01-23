#include "physx_shape.h"
#include "../physx_world.h"
#include "../physx_body.h"

namespace Echo
{
	PhysxShape::PhysxShape()
	{
		m_pxMaterial = PhysxWorld::instance()->getPxPhysics()->createMaterial(0.5f, 0.5f, 0.5f);
	}

	PhysxShape::~PhysxShape()
	{

	}

	void PhysxShape::bindMethods()
	{

	}

	void PhysxShape::update_self()
	{
		if (m_isEnable && !m_pxShape)
		{
			PhysxBody* body = ECHO_DOWN_CAST<PhysxBody*>(getParent());
			if (body && body->getPxBody())
			{
				m_pxShape = createPxShape();
				if (m_pxShape)
				{
					physx::PxVec3 localPos(getLocalPosition().x, getLocalPosition().y, getLocalPosition().z);
					physx::PxTransform localTm(localPos);
					m_pxShape->setLocalPose(localTm);
					body->getPxBody()->attachShape(*m_pxShape);
				}
			}
		}
	}
}
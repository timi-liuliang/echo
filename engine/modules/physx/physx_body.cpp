#include "physx_body.h"
#include "physx_world.h"

namespace Echo
{
	PhysxBody::PhysxBody()
	{

	}

	PhysxBody::~PhysxBody()
	{

	}

	void PhysxBody::bindMethods()
	{

	}

	void PhysxBody::update_self()
	{
		if (m_isEnable && !m_pxBody)
		{
			physx::PxPhysics* physics =	PhysxWorld::instance()->getPxPhysics();
			if (physics)
			{
				physx::PxVec3 position( getWorldPosition().x, getWorldPosition().y, getWorldPosition().z);
				m_pxBody = physics->createRigidStatic(physx::PxTransform(position));

				PhysxWorld::instance()->getPxScene()->addActor(*m_pxBody);
			}
		}
	}
}
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
				m_pxMaterial = PhysxWorld::instance()->getPxPhysics()->createMaterial(0.5f, 0.5f, 0.5f);
				m_pxBody = physics->createRigidStatic(physx::PxTransform(position));

				// box shape
				physx::PxShape* aBoxShape = physx::PxRigidActorExt::createExclusiveShape(*m_pxBody, physx::PxBoxGeometry(1.f / 2, 1.f / 2, 1.f / 2), *m_pxMaterial);
				//physx::PxRigidBodyExt::updateMassAndInertia(*m_pxBody, 1.f);

				PhysxWorld::instance()->getPxScene()->addActor(*m_pxBody);
			}
		}
	}
}
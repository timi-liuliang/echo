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
		if (m_isEnable && !m_pxActor)
		{
			physx::PxPhysics* physics =	PhysxWorld::instance()->getPxPhysics();
			if (physics)
			{
				m_pxMaterial = PhysxWorld::instance()->getPxPhysics()->createMaterial(0.5f, 0.5f, 0.5f);
				m_pxActor = PxCreatePlane(*physics, physx::PxPlane( 0, 1, 0, 0), *m_pxMaterial);

				PhysxWorld::instance()->getPxScene()->addActor(*m_pxActor);
			}
		}
	}
}
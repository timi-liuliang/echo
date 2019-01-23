#include "physx_body.h"
#include "physx_world.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	PhysxBody::PhysxBody()
		: m_type("Static", { "Static", "Kinematic", "Dynamic" })
	{

	}

	PhysxBody::~PhysxBody()
	{

	}

	void PhysxBody::bindMethods()
	{
		CLASS_BIND_METHOD(PhysxBody, getType, DEF_METHOD("getType"));
		CLASS_BIND_METHOD(PhysxBody, setType, DEF_METHOD("setType"));

		CLASS_REGISTER_PROPERTY(PhysxBody, "Type", Variant::Type::StringOption, "getType", "setType");
	}

	void PhysxBody::update_self()
	{
		if (m_isEnable && !m_pxBody)
		{
			physx::PxPhysics* physics =	PhysxWorld::instance()->getPxPhysics();
			if (physics)
			{
				physx::PxVec3 position( getWorldPosition().x, getWorldPosition().y, getWorldPosition().z);
				if (m_type.getIdx() == 0)
				{
					m_pxBody = physics->createRigidStatic(physx::PxTransform(position));
				}
				else
				{
					physx::PxRigidDynamic* dyb = physics->createRigidDynamic(physx::PxTransform(position));
					if(m_type.getIdx()==1)
						dyb->setMass(0.f);
					
					m_pxBody = dyb;
				}

				PhysxWorld::instance()->getPxScene()->addActor(*m_pxBody);
			}
		}

		if (m_pxBody)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				physx::PxTransform pxTransform = m_pxBody->getGlobalPose();
				this->setWorldPosition( (Vector3&)pxTransform.p);
				this->setWorldOrientation((Quaternion&)pxTransform.q);
			}
			else
			{
				physx::PxTransform pxTransform((physx::PxVec3&)getWorldPosition(), (physx::PxQuat&)getWorldOrientation());
				m_pxBody->setGlobalPose( pxTransform);
			}
		}
	}
}
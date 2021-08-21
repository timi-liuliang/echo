#include "physx_body.h"
#include "physx_module.h"
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
		CLASS_BIND_METHOD(PhysxBody, setLinearVelocity, DEF_METHOD("setLinearVelocity"));
		CLASS_BIND_METHOD(PhysxBody, getLinearVelocity, DEF_METHOD("getLinearVelocity"));
		CLASS_BIND_METHOD(PhysxBody, addForce, DEF_METHOD("addForce"));

		CLASS_REGISTER_PROPERTY(PhysxBody, "Type", Variant::Type::StringOption, "getType", "setType");
	}

	void PhysxBody::updateInternal(float elapsedTime)
	{
		const Vector3& shift = PhysxModule::instance()->getShift();
		if (m_isEnable && !m_pxBody)
		{
			physx::PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
			if (physics)
			{
                Vector3 finalPosition = getWorldPosition() + shift;
				physx::PxTransform pxTransform((physx::PxVec3&)finalPosition, (physx::PxQuat&)getWorldOrientation());
				if (m_type.getIdx() == 0)
				{
					m_pxBody = physics->createRigidStatic(pxTransform);
				}
				else
				{
					physx::PxRigidDynamic* dyb = physics->createRigidDynamic(pxTransform);
					if(m_type.getIdx()==1)
						dyb->setMass(0.f);
					
					m_pxBody = dyb;
				}

				PhysxModule::instance()->getPxScene()->addActor(*m_pxBody);
			}
		}

		if (m_pxBody)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				physx::PxTransform pxTransform = m_pxBody->getGlobalPose();
				this->setWorldPosition( (Vector3&)pxTransform.p - shift);
				this->setWorldOrientation((Quaternion&)pxTransform.q);
			}
			else
			{
                Vector3 finalPosition = getWorldPosition() + shift;
				physx::PxTransform pxTransform((physx::PxVec3&)finalPosition, (physx::PxQuat&)getWorldOrientation());
				m_pxBody->setGlobalPose( pxTransform);
			}
		}
	}

	void PhysxBody::setLinearVelocity(const Vector3& velocity)
	{
		if (m_pxBody && m_type.getIdx() == 2)
		{
			physx::PxRigidDynamic* dyb = dynamic_cast<physx::PxRigidDynamic*>(m_pxBody);
			if (dyb)
				dyb->setLinearVelocity((physx::PxVec3&)velocity);
		}
	}

	Vector3 PhysxBody::getLinearVelocity()
	{
		if (m_pxBody && m_type.getIdx() == 2)
		{
			physx::PxRigidDynamic* dyb = dynamic_cast<physx::PxRigidDynamic*>(m_pxBody);
			if(dyb)
				return (Vector3&)dyb->getLinearVelocity();
		}

		return Vector3::ZERO;
	}

	void PhysxBody::addForce(const Vector3& force)
	{
		if (m_pxBody && m_type.getIdx() == 2)
		{
			physx::PxRigidDynamic* dyb = dynamic_cast<physx::PxRigidDynamic*>(m_pxBody);
			if (dyb)
				dyb->addForce((physx::PxVec3&)force);
		}
	}
}

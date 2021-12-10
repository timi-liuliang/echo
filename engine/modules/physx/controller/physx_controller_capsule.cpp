#include "physx_controller_capsule.h"
#include "../physx_module.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	PhysxControllerCapsule::PhysxControllerCapsule()
	{

	}

	PhysxControllerCapsule::~PhysxControllerCapsule()
	{

	}

	void PhysxControllerCapsule::bindMethods()
	{
		CLASS_BIND_METHOD(PhysxControllerCapsule, getRadius);
		CLASS_BIND_METHOD(PhysxControllerCapsule, setRadius);
		CLASS_BIND_METHOD(PhysxControllerCapsule, getHeight);
		CLASS_BIND_METHOD(PhysxControllerCapsule, setHeight);

		CLASS_REGISTER_PROPERTY(PhysxControllerCapsule, "Radius", Variant::Type::Real, getRadius, setRadius);
		CLASS_REGISTER_PROPERTY(PhysxControllerCapsule, "Height", Variant::Type::Real, getHeight, setHeight);
	}

	void PhysxControllerCapsule::setRadius(float radius)
	{
		if (m_radius != radius)
		{
			m_radius = radius;

			if (m_pxController)
			{
				physx::PxCapsuleController* controller = ECHO_DOWN_CAST<physx::PxCapsuleController*>(m_pxController);
				if (controller)
				{
					controller->setRadius(m_radius);
				}
			}
		}
	}

	void PhysxControllerCapsule::setHeight(float height)
	{
		if (m_height != height)
		{
			m_height = height;

			if (m_pxController)
			{
				physx::PxCapsuleController* controller = ECHO_DOWN_CAST<physx::PxCapsuleController*>(m_pxController);
				if (controller)
				{
					controller->resize(m_height);
				}
			}
		}
	}

	physx::PxController* PhysxControllerCapsule::createController()
	{
		physx::PxControllerManager* mgr = PhysxModule::instance()->getPxControllerManager();
		if (mgr)
		{
			physx::PxCapsuleControllerDesc desc;
			desc.radius = m_radius;
			desc.height = m_height;
			desc.material = PhysxModule::instance()->getPxPhysics()->createMaterial(0.5f, 0.5f, 0.1f);

			return mgr->createController(desc);
		}

		return nullptr;
	}

	bool PhysxControllerCapsule::sweep(const Vector3& unitDir, float distance)
	{
		physx::PxScene* pxScene = PhysxModule::instance()->getPxScene();
		if (pxScene)
		{
			physx::PxSweepBuffer hitCb;
			bool result = pxScene->sweep(
				physx::PxCapsuleGeometry(m_radius, m_height * 0.5f),
				m_pxController->getActor()->getGlobalPose(),
				(const physx::PxVec3&)Vector3::NEG_UNIT_Y,
				distance,
				hitCb,
				physx::PxHitFlag::eDEFAULT,
				physx::PxQueryFilterData(physx::PxQueryFlag::eSTATIC));

			return result;
		}

		return false;
	}

	bool PhysxControllerCapsule::overlap()
	{
		physx::PxScene* pxScene = PhysxModule::instance()->getPxScene();
		if (pxScene)
		{
			physx::PxOverlapBuffer hitCb;
			pxScene->overlap(
				physx::PxCapsuleGeometry(m_radius, m_height * 0.5f),
				m_pxController->getActor()->getGlobalPose(),
				hitCb,
				physx::PxQueryFilterData(physx::PxQueryFlag::eSTATIC));

			for (i32 i = 0; i < hitCb.getNbTouches(); i++)
			{
				const physx::PxOverlapHit& overlapHit = hitCb.getTouch(i);

				// Ignore self
				if (overlapHit.actor != m_pxController->getActor())
					return true;
			}
		}

		return false;
	}
}

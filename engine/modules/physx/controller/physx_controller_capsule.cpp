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
		CLASS_BIND_METHOD(PhysxControllerCapsule, getRadius, DEF_METHOD("getRadius"));
		CLASS_BIND_METHOD(PhysxControllerCapsule, setRadius, DEF_METHOD("setRadius"));
		CLASS_BIND_METHOD(PhysxControllerCapsule, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(PhysxControllerCapsule, setHeight, DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(PhysxControllerCapsule, "Radius", Variant::Type::Real, "getRadius", "setRadius");
		CLASS_REGISTER_PROPERTY(PhysxControllerCapsule, "Height", Variant::Type::Real, "getHeight", "setHeight");
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
}

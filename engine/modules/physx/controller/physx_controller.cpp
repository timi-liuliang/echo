#include "physx_controller.h"
#include "../physx_module.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	PhysxController::PhysxController()
	{

	}

	PhysxController::~PhysxController()
	{
		if (m_pxController)
		{
			m_pxController->release();
		}
	}

	void PhysxController::bindMethods()
	{
		CLASS_BIND_METHOD(PhysxController, move, DEF_METHOD("move"));
	}

	void PhysxController::update_self()
	{
		const Vector3& shift = PhysxModule::instance()->getShift();

		if (m_isEnable && !m_pxController)
		{
			m_pxController = createController();
			if (m_pxController)
			{
				Vector3 finalPosition = getWorldPosition() + shift;
				m_pxController->setFootPosition(physx::PxExtendedVec3(finalPosition.x, finalPosition.y, finalPosition.z));
			}
		}

		if (m_pxController)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				physx::PxExtendedVec3 position = m_pxController->getFootPosition();
				this->setWorldPosition(Vector3(position.x, position.y, position.z) - shift);
			}
			else
			{
				Vector3 finalPosition = getWorldPosition() + shift;
				m_pxController->setFootPosition(physx::PxExtendedVec3(finalPosition.x, finalPosition.y, finalPosition.z));
			}
		}

	}

	void PhysxController::move(const Vector3& disp)
	{
		if (m_pxController)
		{
			physx::PxControllerFilters filters;
			m_pxController->move((physx::PxVec3&)disp, 0.002f, 0.01f, filters, nullptr);
		}
	}
}

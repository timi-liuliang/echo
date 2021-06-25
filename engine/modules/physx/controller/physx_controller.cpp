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
		CLASS_BIND_METHOD(PhysxController, sweep, DEF_METHOD("sweep"));
		CLASS_BIND_METHOD(PhysxController, overlap, DEF_METHOD("overlap"));
		CLASS_BIND_METHOD(PhysxController, getContactOffset, DEF_METHOD("getContactOffset"));
		CLASS_BIND_METHOD(PhysxController, setContactOffset, DEF_METHOD("setContactOffset"));
		CLASS_BIND_METHOD(PhysxController, getFootPosition, DEF_METHOD("getFootPosition"));

		CLASS_REGISTER_PROPERTY(PhysxController, "ContactOffset", Variant::Type::Real, "getContactOffset", "setContactOffset");
	}

	float PhysxController::getContactOffset() const
	{
		if (m_pxController)
			return m_pxController->getContactOffset();

		return 0.f;
	}

	void PhysxController::setContactOffset(float offset)
	{
		if (m_pxController)
			m_pxController->setContactOffset(offset);
	}

	Vector3 PhysxController::getFootPosition()
	{
		if (m_pxController)
		{
			physx::PxExtendedVec3 fp = m_pxController->getFootPosition();
			return Vector3(fp.x, fp.y, fp.z);
		}

		return Vector3::ZERO;
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
				m_pxController->setPosition(physx::PxExtendedVec3(finalPosition.x, finalPosition.y, finalPosition.z));
			}
		}

		if (m_pxController)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				physx::PxExtendedVec3 position = m_pxController->getPosition();
				this->setWorldPosition(Vector3(position.x, position.y, position.z) - shift);
			}
			else
			{
				Vector3 finalPosition = getWorldPosition() + shift;
				m_pxController->setPosition(physx::PxExtendedVec3(finalPosition.x, finalPosition.y, finalPosition.z));
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

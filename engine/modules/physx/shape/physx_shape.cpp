#include "physx_shape.h"
#include "../physx_module.h"
#include "../physx_body.h"
#include <engine/core/main/engine.h>
#include <engine/core/log/Log.h>

namespace Echo
{
	PhysxShape::PhysxShape()
	{
		m_pxMaterial = PhysxModule::instance()->getPxPhysics()->createMaterial(0.5f, 0.5f, 0.5f);
	}

	PhysxShape::~PhysxShape()
	{
		if (m_pxShape)
		{
			PhysxBody* body = ECHO_DOWN_CAST<PhysxBody*>(getParent());
			if (body && body->getPxBody())
			{
				body->getPxBody()->detachShape(*m_pxShape);
				m_pxShape->release();
				m_pxShape = nullptr;
			}
		}
	}

	void PhysxShape::bindMethods()
	{

	}

	void PhysxShape::updateInternal(float elapsedTime)
	{
		if (m_isEnable && !m_pxShape)
		{
			PhysxBody* body = ECHO_DOWN_CAST<PhysxBody*>(getParent());
			if (body && body->getPxBody())
			{
				if (body->getPxBody())
				{
					m_pxShape = createPxShape();
					if (m_pxShape)
					{
						physx::PxTransform localTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
						m_pxShape->setLocalPose(localTransform);

						physx::PxFilterData filterData;
						filterData.word3 = 0xffff0000;
						m_pxShape->setQueryFilterData(filterData);

						body->getPxBody()->attachShape(*m_pxShape);
					}
				}
			}
			else
			{
				EchoLogError("Physx shape [%s]'s parent should be a PhysxBody", getNodePath().c_str());
			}
		}

		if (m_pxShape)
		{
			if (!Engine::instance()->getConfig().m_isGame)
			{
				physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
				m_pxShape->setLocalPose(pxTransform);
			}
		}
	}
}
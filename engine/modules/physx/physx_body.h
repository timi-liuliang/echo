#pragma once

#include "engine/core/scene/node.h"
#include "physx_base.h"

namespace Echo
{
	class PhysxBody : public Node 
	{
		ECHO_CLASS(PhysxBody, Node)

	public:
		PhysxBody();
		virtual ~PhysxBody();

		// get physx body
		physx::PxRigidActor* getPxBody() { return m_pxBody; }

	private:
		// update
		virtual void update_self();

	private:
		physx::PxRigidActor*m_pxBody = nullptr;
	};
}

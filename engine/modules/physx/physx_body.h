#pragma once

#include "engine/core/scene/node.h"
#include <PxPhysicsAPI.h>

namespace Echo
{
	class PhysxBody : public Node 
	{
		ECHO_CLASS(PhysxBody, Node)

	public:
		PhysxBody();
		virtual ~PhysxBody();

	private:
		// update
		virtual void update_self();

	private:
		physx::PxMaterial*	m_pxMaterial = nullptr;
		physx::PxActor*		m_pxActor = nullptr;
	};
}
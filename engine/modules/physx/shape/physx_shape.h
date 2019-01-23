#pragma once

#include <engine/core/scene/node.h>
#include "../physx_base.h"

namespace Echo
{
	class PhysxShape : public Node
	{
		ECHO_VIRTUAL_CLASS(PhysxShape, Node)

	public:
		PhysxShape();
		~PhysxShape();

	protected:
		// update self
		virtual void update_self() override;

		// create shape
		virtual physx::PxShape* createPxShape() { return nullptr; }

	protected:
		physx::PxMaterial*	m_pxMaterial = nullptr;
		physx::PxShape*		m_pxShape = nullptr;
	};
}
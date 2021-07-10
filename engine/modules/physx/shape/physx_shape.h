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

		// Sweep
		virtual bool sweep(const Vector3& unitDir, float distance) { return false; }

	protected:
		// Update self
		virtual void updateInternal() override;

		// Create shape
		virtual physx::PxShape* createPxShape() { return nullptr; }

	protected:
		physx::PxMaterial*	m_pxMaterial = nullptr;
		physx::PxShape*		m_pxShape = nullptr;
	};
}
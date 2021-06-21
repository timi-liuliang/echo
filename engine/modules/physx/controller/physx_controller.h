#pragma once

#include "engine/core/scene/node.h"
#include "../physx_base.h"

namespace Echo
{
	class PhysxController : public Node
	{
		ECHO_VIRTUAL_CLASS(PhysxController, Node)

	public:
		PhysxController();
		virtual ~PhysxController();

		// Move
		void move(const Vector3& disp);

		// Sweep
		virtual bool sweep(const Vector3& unitDir, float distance) { return false; }

		// Overlap
		virtual bool overlap() { return false; }

	protected:
		// Create controller
		virtual physx::PxController* createController() { return nullptr; }

	private:
		// Update
		virtual void update_self() override;

	protected:
		physx::PxController* m_pxController = nullptr;
	};
}

#pragma once

#include "engine/core/scene/node.h"
#include "physx_base.h"

namespace Echo
{
	class PhysxCharacterController : public Node
	{
		ECHO_CLASS(PhysxCharacterController, Node)

	public:
		PhysxCharacterController();
		virtual ~PhysxCharacterController();

		// type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// get physx body
		physx::PxRigidActor* getPxBody() { return m_pxBody; }

		// linear velocity
		void setLinearVelocity(const Vector3& velocity);
		Vector3 getLinearVelocity();

		// apply force
		void addForce(const Vector3& force);

	private:
		// update
		virtual void update_self() override;

	private:
		physx::PxRigidActor* m_pxBody = nullptr;
		StringOption		m_type;
	};
}

#pragma once

#include "engine/core/scene/node.h"
#include "physx_controller.h"

namespace Echo
{
	class PhysxControllerCapsule : public PhysxController
	{
		ECHO_CLASS(PhysxControllerCapsule, PhysxController)

	public:
		PhysxControllerCapsule();
		virtual ~PhysxControllerCapsule();

		// Radius
		float getRadius() const { return m_radius; }
		void setRadius(float radius);

		// Height
		float getHeight() const { return m_height; }
		void setHeight(float height);

	protected:
		// Create controller
		virtual physx::PxController* createController() override;

	private:
		float	m_radius = 1.f;
		float	m_height = 2.f;
	};
}

#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapeCapsule : public PhysxShape
	{
		ECHO_CLASS(PhysxShapeCapsule, PhysxShape)

	public:
		PhysxShapeCapsule();
		~PhysxShapeCapsule();

		// radius
		float getRadius() const { return m_radius; }
		void setRadius(float radius);

		// height
		float getHeight() const { return m_height; }
		void setHeight(float height);

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
		float	m_radius = 1.f;
		float	m_height = 1.f;
	};
}
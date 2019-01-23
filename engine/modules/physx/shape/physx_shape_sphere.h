#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapeSphere : public PhysxShape
	{
		ECHO_CLASS(PhysxShapeSphere, PhysxShape)

	public:
		PhysxShapeSphere();
		~PhysxShapeSphere();

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
		float	m_radius = 1.f;
	};
}
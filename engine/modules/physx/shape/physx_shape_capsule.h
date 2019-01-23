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

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
		float	m_radius = 1.f;
		float	m_halfHeight = 1.f;
	};
}
#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapeHeightfield : public PhysxShape
	{
		ECHO_CLASS(PhysxShapeHeightfield, PhysxShape)

	public:
		PhysxShapeHeightfield();
		virtual ~PhysxShapeHeightfield();

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
		physx::PxHeightField* m_pxHeightField = nullptr;
	};
}
#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapePlane : public PhysxShape
	{
		ECHO_CLASS(PhysxShapePlane, PhysxShape)

	public:
		PhysxShapePlane();
		~PhysxShapePlane();

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
	};
}
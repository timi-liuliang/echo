#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapeBox : public PhysxShape
	{
		ECHO_CLASS(PhysxShapeBox, PhysxShape)

	public:
		PhysxShapeBox();
		~PhysxShapeBox();

		// radius
		const Vector3& getHalfExtents() const { return m_halfExtents; }
		void setHalfExtents(const Vector3& halfExtents);

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
		Vector3	m_halfExtents = Vector3(0.5f, 0.5f, 0.5f);
	};
}
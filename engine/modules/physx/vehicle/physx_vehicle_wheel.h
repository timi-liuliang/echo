#pragma once

#include "engine/core/scene/node.h"
#include "../physx_base.h"

namespace Echo
{
	class PhysxVehicleWheel : public Node
	{
		ECHO_CLASS(PhysxVehicleWheel, Node)

	public:
		PhysxVehicleWheel();
		virtual ~PhysxVehicleWheel();

		// Mass
		float getMass() const { return m_mass; }

		// Width
		float getWidth() const { return m_width; }

		// Radius
		float getRadius() const { return m_width; }

		// MOI
		float getMOI() const { return 0.5f * m_mass * m_radius * m_radius; }

		// Sim filter data
		const physx::PxFilterData& getPxSimFilterData() const { return m_simFilterData; }

		// Physx material
		physx::PxMaterial* getPxMaterial();

		// Get wheel mesh
		physx::PxConvexMesh* getPxWheelMesh();

	public:
		// Create convex mesh
		static physx::PxConvexMesh* createConvexMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts);

	private:
		physx::PxF32			m_mass = 20.f;
		physx::PxF32			m_width = 0.4f;
		physx::PxF32			m_radius = 0.5f;
		physx::PxMaterial*		m_material = nullptr;
		physx::PxFilterData		m_simFilterData;
		physx::PxConvexMesh*	m_pxWheelMesh = nullptr;

		physx::PxU32			m_tireType;
	};
}

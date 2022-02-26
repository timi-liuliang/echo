#pragma once

#include "engine/core/scene/node.h"
#include "../physx_base.h"

namespace Echo
{
	class PhysxVehicleChassis : public Node
	{
		ECHO_CLASS(PhysxVehicleChassis, Node)

	public:
		PhysxVehicleChassis();
		virtual ~PhysxVehicleChassis();

		// Mass
		void setMass(double mass) { m_mass = mass;}
		double getMass() const { return m_mass; }

		// Dims
		Vector3 getDims() const { return Vector3(m_dims.x, m_dims.y, m_dims.z); }

		// MOI
		const physx::PxVec3& getMOI() const { return m_MOI; }

		// CMOffset
		const physx::PxVec3& getCMOffset() const { return m_CMOffset; }

		// Filter data
		const physx::PxFilterData& getPxFilterData() const { return m_filterData; }

		// Physx material
		physx::PxMaterial* getPxMaterial();

		// Get wheel mesh
		physx::PxConvexMesh* getPxMesh();

	private:
		double					m_mass = 1500.0;
		physx::PxVec3			m_dims;
		physx::PxVec3			m_MOI;
		physx::PxVec3			m_CMOffset;				// Center of mass offset
		physx::PxMaterial*		m_material = nullptr;
		physx::PxFilterData		m_filterData;
	};
}

#include "physx_vehicle_chassis.h"
#include "physx_vehicle_wheel.h"
#include "../physx_module.h"

namespace Echo
{
	PhysxVehicleChassis::PhysxVehicleChassis()
		: Node()
	{
		m_mass = 1500.0;
		m_dims = physx::PxVec3(9.f, 2.f, 3.5f);
		m_MOI = physx::PxVec3(
			(m_dims.y * m_dims.y + m_dims.z * m_dims.z) * m_mass / 12.0f,
			(m_dims.x * m_dims.x + m_dims.z * m_dims.z) * 0.8f * m_mass / 12.0f,
			(m_dims.x * m_dims.x + m_dims.y * m_dims.y) * m_mass / 12.0f);
		m_CMOffset = physx::PxVec3(0.0f, -m_dims.y * 0.5f + 0.65f, 0.f);
	}

	PhysxVehicleChassis::~PhysxVehicleChassis()
	{
	}

	void PhysxVehicleChassis::bindMethods()
	{
		CLASS_BIND_METHOD(PhysxVehicleChassis, getMass);
		CLASS_BIND_METHOD(PhysxVehicleChassis, setMass);

		CLASS_REGISTER_PROPERTY(PhysxVehicleChassis, "Mass", Variant::Type::Real, getMass, setMass);
	}

	physx::PxMaterial* PhysxVehicleChassis::getPxMaterial()
	{
		if (!m_material)
		{
			m_material = PhysxModule::instance()->getPxPhysics()->createMaterial(0.5f, 0.5f, 0.5f);
		}

		return m_material;
	}

	physx::PxConvexMesh* PhysxVehicleChassis::getPxMesh()
	{
		const physx::PxF32 x = m_dims.x * 0.5f;
		const physx::PxF32 y = m_dims.y * 0.5f;
		const physx::PxF32 z = m_dims.z * 0.5f;
		physx::PxVec3 verts[8] =
		{
			physx::PxVec3(x,y,-z),
			physx::PxVec3(x,y,z),
			physx::PxVec3(x,-y,z),
			physx::PxVec3(x,-y,-z),
			physx::PxVec3(-x,y,-z),
			physx::PxVec3(-x,y,z),
			physx::PxVec3(-x,-y,z),
			physx::PxVec3(-x,-y,-z)
		};

		return PhysxVehicleWheel::createConvexMesh(verts, 8);
	}
}

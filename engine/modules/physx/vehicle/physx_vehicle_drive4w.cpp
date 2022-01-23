#include "physx_vehicle_drive4w.h"
#include "../physx_module.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	PhysxVehicleDrive4W::PhysxVehicleDrive4W()
		: Node()
	{

	}

	PhysxVehicleDrive4W::~PhysxVehicleDrive4W()
	{
		reset();
	}

	void PhysxVehicleDrive4W::bindMethods()
	{
	}

	void PhysxVehicleDrive4W::settingUp()
	{
		reset();

		physx::PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (m_numWheels > 0 && physics)
		{
			// Wheels
			m_wheelsSimData = physx::PxVehicleWheelsSimData::allocate(m_numWheels);
			setupWheelsSimulationData(m_wheelsSimData);

			// Drive
			physx::PxVehicleDriveSimData4W driveSimData;
			setupDriveSimData(driveSimData);

			// Vehicle actor
			Vector3 startPosition = getWorldPosition() + PhysxModule::instance()->getShift();
			physx::PxTransform pxTransform((physx::PxVec3&)startPosition, (physx::PxQuat&)getWorldOrientation());
			m_vehicleActor = physics->createRigidDynamic(pxTransform);
			setupVehicleActor(m_vehicleActor);
			PhysxModule::instance()->getPxScene()->addActor(*m_vehicleActor);

			// Vehicle drive 4w
			m_vehicleDrive4W = physx::PxVehicleDrive4W::allocate(m_numWheels);
			m_vehicleDrive4W->setup(physics, m_vehicleActor, *m_wheelsSimData, driveSimData, m_numWheels - 4);
		}
	}

	void PhysxVehicleDrive4W::setupWheelsSimulationData(physx::PxVehicleWheelsSimData* wheelsSimData)
	{

	}

	void PhysxVehicleDrive4W::setupDriveSimData(physx::PxVehicleDriveSimData4W& driveSimData)
	{

	}

	void PhysxVehicleDrive4W::setupVehicleActor(physx::PxRigidDynamic* vehicleActor)
	{

	}

	void PhysxVehicleDrive4W::reset()
	{
		if (m_wheelsSimData)
		{
			m_wheelsSimData->free();
			m_wheelsSimData = nullptr;
		}
	}
}

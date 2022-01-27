#pragma once

#include "engine/core/scene/node.h"
#include "../physx_base.h"
#include "vehicle/PxVehicleDrive4W.h"

namespace Echo
{
	class PhysxVehicleDrive4W : public Node 
	{
		ECHO_CLASS(PhysxVehicleDrive4W, Node)

	public:
		// Surface Type
		enum SurfaceType
		{
			DrivableSurface = 0xffff0000,
			UndriveableSurface = 0x0000ffff
		};

	public:
		PhysxVehicleDrive4W();
		virtual ~PhysxVehicleDrive4W();

	protected:
		// Setting up the vehicle
		void settingUp();
		void setupWheelsSimulationData(physx::PxVehicleWheelsSimData* wheelsSimData);
		void setupDriveSimData(physx::PxVehicleDriveSimData4W& driveSimData);
		void setupVehicleActor(physx::PxRigidDynamic* vehicleActor);
		void setupNonDrivableSurface(physx::PxFilterData& filterData);

		// Reset
		void reset();

	private:
		i32								m_numWheels = 4;
		physx::PxVehicleWheelsSimData*	m_wheelsSimData = nullptr;
		physx::PxRigidDynamic*			m_vehicleActor = nullptr;
		physx::PxVehicleDrive4W*		m_vehicleDrive4W = nullptr;
	};
}

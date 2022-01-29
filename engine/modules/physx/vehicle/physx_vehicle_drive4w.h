#pragma once

#include "engine/core/scene/node.h"
#include "physx_vehicle_wheel.h"

namespace Echo
{
	// https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Vehicles.html
	class PhysxVehicleDrive4W : public Node 
	{
		ECHO_CLASS(PhysxVehicleDrive4W, Node)

	public:
		typedef vector<PhysxVehicleWheel*>::type WheelList;

		// Surface Type
		enum SurfaceType
		{
			DrivableSurface = 0xffff0000,
			UndriveableSurface = 0x0000ffff
		};

	public:
		PhysxVehicleDrive4W();
		virtual ~PhysxVehicleDrive4W();

		// Get wheel by index
		vector<PhysxVehicleWheel*>::type getAllWheels();

		// Reset
		void reset();

	protected:
		// update
		virtual void updateInternal(float elapsedTime) override;

		// Setting up the vehicle
		void settingUp();
		void setupWheelsSimulationData(physx::PxVehicleWheelsSimData* wheelsSimData);
		void setupDriveSimData(physx::PxVehicleDriveSimData4W& driveSimData);
		void setupVehicleActor();
		void setupNonDrivableSurface(physx::PxFilterData& filterData);

		// Chassis mesh
		physx::PxConvexMesh* createChassisMesh(const physx::PxVec3& dims);

	private:
		physx::PxF32					m_chassisMass;
		physx::PxVec3					m_chassisDims;
		physx::PxVec3					m_chassisMOI;
		physx::PxVec3					m_chassisCMOffset;				// Center of mass offset
		physx::PxMaterial*				m_chassisMaterial = nullptr;
		physx::PxFilterData				m_chassisFilterData;
		WheelList						m_wheels;
		physx::PxVehicleWheelsSimData*	m_wheelsSimData = nullptr;
		physx::PxRigidDynamic*			m_vehicleActor = nullptr;
		physx::PxVehicleDrive4W*		m_vehicleDrive4W = nullptr;
	};
}

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
		float chassisMass = 0.f;
		physx::PxVec3 chassisCMOffset(0.f, 0.f, 0.f);

		float wheelMass = 20.f;
		float wheelRadius = 0.5f;
		float wheelWidth = 0.4f;
		float wheelMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;

		// Set up the wheel data structures with mass, moi, radius, width
		physx::PxVehicleWheelData wheels[PX_MAX_NB_WHEELS];
		{
			for (ui32 i = 0; i < m_numWheels; i++)
			{
				wheels[i].mMass = wheelMass;
				wheels[i].mMOI = wheelMOI;
				wheels[i].mRadius = wheelRadius;
				wheels[i].mWidth = wheelWidth;
			}

			wheels[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.f;
			wheels[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.f;
			wheels[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = physx::PxPi * 0.3333f;
			wheels[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = physx::PxPi * 0.3333f;
		}

		// Set up the tires
		physx::PxVehicleTireData tires[PX_MAX_NB_WHEELS];
		{
			for (ui32 i = 0; i < m_numWheels; i++)
			{
				tires[i].mType = 0;// physx::TIRE_TYPE_NORMAL;
			}
		}

		// Set up the suspensions
		physx::PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
		physx::PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
		{
			physx::PxVec3 chassisCMOffset(0.f, 0.f, 0.f);
			physx::PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
			physx::PxVehicleComputeSprungMasses(m_numWheels, wheelCenterActorOffsets, chassisCMOffset, chassisMass, 1, suspSprungMasses);

			// Set the suspenson data
			for (ui32 i = 0; i < m_numWheels; i++)
			{
				suspensions[i].mMaxCompression = 0.3f;
				suspensions[i].mMaxDroop = 0.1f;
				suspensions[i].mSpringStrength = 35000.f;
				suspensions[i].mSpringDamperRate = 4500.f;
				suspensions[i].mSprungMass = suspSprungMasses[i];
			}

			// Set the camber angles
			const physx::PxF32 camberAngleAtRest = 0.0;
			const physx::PxF32 camberAngleAtMaxDroop = 0.01f;
			const physx::PxF32 camberAngleAtMaxCompression = -0.01f;
			for (physx::PxU32 i = 0; i < m_numWheels; i += 2)
			{
				suspensions[i + 0].mCamberAtRest = camberAngleAtRest;
				suspensions[i + 1].mCamberAtRest = -camberAngleAtRest;
				suspensions[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop;
				suspensions[i + 1].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
				suspensions[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression;
				suspensions[i + 1].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
			}
		}

		//Set up the wheel geometry.
		physx::PxVec3 suspTravelDirections[PX_MAX_NB_WHEELS];
		physx::PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
		physx::PxVec3 suspForceAppCMOffsets[PX_MAX_NB_WHEELS];
		physx::PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
		{
			//Set the geometry data.
			for (physx::PxU32 i = 0; i < m_numWheels; i++)
			{
				//Vertical suspension travel.
				suspTravelDirections[i] = physx::PxVec3(0, -1, 0);

				//Wheel center offset is offset from rigid body center of mass.
				wheelCentreCMOffsets[i] = wheelCenterActorOffsets[i] - chassisCMOffset;

				//Suspension force application point 0.3 metres below
				//rigid body center of mass.
				suspForceAppCMOffsets[i] = physx::PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);

				//Tire force application point 0.3 metres below
				//rigid body center of mass.
				tireForceAppCMOffsets[i] = physx::PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
			}
		}

		//Set up the filter data of the raycast that will be issued by each suspension.
		physx::PxFilterData qryFilterData;
		setupNonDrivableSurface(qryFilterData);

		//Set the wheel, tire and suspension data.
		//Set the geometry data.
		//Set the query filter data
		for (physx::PxU32 i = 0; i < m_numWheels; i++)
		{
			wheelsSimData->setWheelData(i, wheels[i]);
			wheelsSimData->setTireData(i, tires[i]);
			wheelsSimData->setSuspensionData(i, suspensions[i]);
			wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
			wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
			wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
			wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
			wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
			wheelsSimData->setWheelShapeMapping(i, i);
		}
	}

	void PhysxVehicleDrive4W::setupDriveSimData(physx::PxVehicleDriveSimData4W& driveSimData)
	{

	}

	void PhysxVehicleDrive4W::setupVehicleActor(physx::PxRigidDynamic* vehicleActor)
	{

	}

	void PhysxVehicleDrive4W::setupNonDrivableSurface(physx::PxFilterData& filterData)
	{
		filterData.word3 = UndriveableSurface;
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

#include "physx_vehicle_drive4w.h"
#include "../physx_module.h"
#include <engine/core/main/Engine.h>

namespace Echo
{
	PhysxVehicleDrive4W::PhysxVehicleDrive4W()
		: Node()
	{
		m_chassisMass = 1500.f;
		m_chassisDims = physx::PxVec3(3.5f, 2.f, 9.f);
		m_chassisMOI = physx::PxVec3(
			(m_chassisDims.y * m_chassisDims.y + m_chassisDims.z * m_chassisDims.z) * m_chassisMass / 12.0f,
			(m_chassisDims.x * m_chassisDims.x + m_chassisDims.z * m_chassisDims.z) * 0.8f * m_chassisMass / 12.0f,
			(m_chassisDims.x * m_chassisDims.x + m_chassisDims.y * m_chassisDims.y) * m_chassisMass / 12.0f);
		m_chassisCMOffset = physx::PxVec3(0.0f, -m_chassisDims.y * 0.5f + 0.65f, 0.25f);
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

		m_wheels = getAllWheels();

		physx::PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (m_wheels.size() > 0 && physics)
		{
			// Wheels
			m_wheelsSimData = physx::PxVehicleWheelsSimData::allocate(m_wheels.size());
			setupWheelsSimulationData(m_wheelsSimData);

			// Drive
			physx::PxVehicleDriveSimData4W driveSimData;
			setupDriveSimData(driveSimData);

			// Vehicle actor
			setupVehicleActor();
			PhysxModule::instance()->getPxScene()->addActor(*m_vehicleActor);

			// Vehicle drive 4w
			m_vehicleDrive4W = physx::PxVehicleDrive4W::allocate(m_wheels.size());
			m_vehicleDrive4W->setup(physics, m_vehicleActor, *m_wheelsSimData, driveSimData, m_wheels.size() - 4);

			PhysxModule::instance()->addVehicle(m_vehicleDrive4W);
		}
	}

	void PhysxVehicleDrive4W::setupWheelsSimulationData(physx::PxVehicleWheelsSimData* wheelsSimData)
	{
		// Set up the wheel data structures with mass, moi, radius, width
		physx::PxVehicleWheelData wheels[PX_MAX_NB_WHEELS];
		{
			for (ui32 i = 0; i < m_wheels.size(); i++)
			{
				wheels[i].mMass = m_wheels[i]->getMass();
				wheels[i].mMOI = m_wheels[i]->getMOI();
				wheels[i].mRadius = m_wheels[i]->getRadius();
				wheels[i].mWidth = m_wheels[i]->getWidth();
			}

			wheels[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.f;
			wheels[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.f;
			wheels[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = physx::PxPi * 0.3333f;
			wheels[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = physx::PxPi * 0.3333f;
		}

		physx::PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
		for (ui32 i = 0; i < m_wheels.size(); i++)
		{
			const Vector3& wheelLocalPos = m_wheels[i]->getLocalPosition();
			wheelCenterActorOffsets[i] = physx::PxVec3(wheelLocalPos.x, wheelLocalPos.y, wheelLocalPos.z);
		}

		// Set up the tires
		physx::PxVehicleTireData tires[PX_MAX_NB_WHEELS];
		{
			for (ui32 i = 0; i < m_wheels.size(); i++)
			{
				tires[i].mType = 0;// physx::TIRE_TYPE_NORMAL;
			}
		}

		// Set up the suspensions
		physx::PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
		{
			physx::PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
			physx::PxVehicleComputeSprungMasses(m_wheels.size(), wheelCenterActorOffsets, m_chassisCMOffset, m_chassisMass, 1, suspSprungMasses);

			// Set the suspenson data
			for (ui32 i = 0; i < m_wheels.size(); i++)
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
			for (physx::PxU32 i = 0; i < m_wheels.size(); i += 2)
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
			for (physx::PxU32 i = 0; i < m_wheels.size(); i++)
			{
				//Vertical suspension travel.
				suspTravelDirections[i] = physx::PxVec3(0, -1, 0);

				//Wheel center offset is offset from rigid body center of mass.
				wheelCentreCMOffsets[i] = wheelCenterActorOffsets[i] - m_chassisCMOffset;

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
		for (physx::PxU32 i = 0; i < m_wheels.size(); i++)
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
		// Diff
		physx::PxVehicleDifferential4WData diff;
		diff.mType = physx::PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
		driveSimData.setDiffData(diff);

		// Engine
		physx::PxVehicleEngineData engine;
		engine.mPeakTorque = 500.f;
		engine.mMaxOmega = 600.f;	// Approx 6000 RPM

		// Gears
		physx::PxVehicleGearsData gears;
		gears.mSwitchTime = 0.5f;
		driveSimData.setGearsData(gears);

		// Cluth
		physx::PxVehicleClutchData clutch;
		clutch.mStrength = 10.f;
		driveSimData.setClutchData(clutch);

		// Ackermann steer accuracy
		physx::PxVehicleAckermannGeometryData ackermann;
		ackermann.mAccuracy = 1.f;
		ackermann.mAxleSeparation = 
			m_wheelsSimData->getWheelCentreOffset(physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z -
			m_wheelsSimData->getWheelCentreOffset(physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT).z;
		ackermann.mFrontWidth =
			m_wheelsSimData->getWheelCentreOffset(physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x -
			m_wheelsSimData->getWheelCentreOffset(physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
		ackermann.mRearWidth =
			m_wheelsSimData->getWheelCentreOffset(physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x -
			m_wheelsSimData->getWheelCentreOffset(physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;
		driveSimData.setAckermannGeometryData(ackermann);
	}

	void PhysxVehicleDrive4W::setupVehicleActor()
	{
		physx::PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (physics)
		{
			m_chassisMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);

			Vector3 startPosition = getWorldPosition() + PhysxModule::instance()->getShift();
			physx::PxTransform pxTransform((physx::PxVec3&)startPosition, (physx::PxQuat&)getWorldOrientation());
			m_vehicleActor = physics->createRigidDynamic(pxTransform);

			physx::PxFilterData wheelQueryFilterData;
			setupNonDrivableSurface(wheelQueryFilterData);

			physx::PxFilterData chassisQueryFilterData;
			setupNonDrivableSurface(chassisQueryFilterData);

			// Add all wheel shapes to the actor
			for (physx::PxU32 i = 0; i < m_wheels.size(); i++)
			{
				physx::PxConvexMeshGeometry geom(m_wheels[i]->getPxWheelMesh());
				physx::PxShape* wheelShape = physx::PxRigidActorExt::createExclusiveShape(*m_vehicleActor, geom, *m_wheels[i]->getPxMaterial());
				wheelShape->setQueryFilterData(wheelQueryFilterData);
				wheelShape->setSimulationFilterData(m_wheels[i]->getPxSimFilterData());
				wheelShape->setLocalPose(physx::PxTransform(physx::PxIdentity));
			}

			// Add chassis shapes to the actor
			vector<physx::PxConvexMesh*>::type chassisConvexMeshes = { createChassisMesh(m_chassisDims) };
			for (physx::PxU32 i = 0; i < chassisConvexMeshes.size(); i++)
			{
				physx::PxShape* chassisShape = physx::PxRigidActorExt::createExclusiveShape(*m_vehicleActor, physx::PxConvexMeshGeometry(chassisConvexMeshes[i]), *m_chassisMaterial);
				chassisShape->setQueryFilterData(chassisQueryFilterData);
				chassisShape->setSimulationFilterData(m_chassisFilterData);
				chassisShape->setLocalPose(physx::PxTransform(physx::PxIdentity));
			}

			m_vehicleActor->setMass(m_chassisMass);
			m_vehicleActor->setMassSpaceInertiaTensor(m_chassisMOI);
			m_vehicleActor->setCMassLocalPose(physx::PxTransform(m_chassisCMOffset, physx::PxQuat(physx::PxIdentity)));
		}
	}

	vector<PhysxVehicleWheel*>::type PhysxVehicleDrive4W::getAllWheels()
	{
		vector<PhysxVehicleWheel*>::type wheels;
		for (Node* child : m_children)
		{
			PhysxVehicleWheel* wheel = dynamic_cast<PhysxVehicleWheel*>(child);
			if (wheel)
			{
				wheels.emplace_back(wheel);
			}
		}

		return wheels;
	}

	void PhysxVehicleDrive4W::setupNonDrivableSurface(physx::PxFilterData& filterData)
	{
		filterData.word3 = UndriveableSurface;
	}

	physx::PxConvexMesh* PhysxVehicleDrive4W::createChassisMesh(const physx::PxVec3& dims)
	{
		const physx::PxF32 x = dims.x * 0.5f;
		const physx::PxF32 y = dims.y * 0.5f;
		const physx::PxF32 z = dims.z * 0.5f;
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

	void PhysxVehicleDrive4W::reset()
	{
		if (m_wheelsSimData)
		{
			PhysxModule::instance()->removeVehicle(m_vehicleDrive4W);

			m_wheelsSimData->free();
			m_wheelsSimData = nullptr;
		}
	}

	void PhysxVehicleDrive4W::updateInternal(float elapsedTime)
	{
		if (m_isEnable && !m_vehicleDrive4W)
		{
			settingUp();
		}

		// test
		static float acc = 0;
		acc += elapsedTime;
		if (acc > 1.f)
			acc = 0.f;

		setUseAutoGears(true);
		//setAccel(0.1f);
		setSteer(-25.f * Math::DEG2RAD);
	}

	void PhysxVehicleDrive4W::setToRestState()
	{
		if (m_vehicleDrive4W)
			m_vehicleDrive4W->setToRestState();
	}

	void PhysxVehicleDrive4W::setAccel(float accel)
	{
		if (m_vehicleDrive4W)
			m_vehicleDrive4W->mDriveDynData.setAnalogInput(physx::PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL, accel);
	}

	void PhysxVehicleDrive4W::setSteer(float steer)
	{
		if (m_vehicleDrive4W)
		{
			if(steer>0.f)
				m_vehicleDrive4W->mDriveDynData.setAnalogInput(physx::PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT, steer);
			else
				m_vehicleDrive4W->mDriveDynData.setAnalogInput(physx::PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT, Math::Abs(steer));
		}
	}

	void PhysxVehicleDrive4W::setBrake(float brake)
	{
		if (m_vehicleDrive4W)
			m_vehicleDrive4W->mDriveDynData.setAnalogInput(physx::PxVehicleDrive4WControl::eANALOG_INPUT_BRAKE, brake);
	}

	void PhysxVehicleDrive4W::setHandBrake(float handBrake)
	{
		if (m_vehicleDrive4W)
			m_vehicleDrive4W->mDriveDynData.setAnalogInput(physx::PxVehicleDrive4WControl::eANALOG_INPUT_HANDBRAKE, handBrake);
	}

	void PhysxVehicleDrive4W::setUseAutoGears(bool useAutoGears)
	{
		if (m_vehicleDrive4W)
			m_vehicleDrive4W->mDriveDynData.setUseAutoGears(useAutoGears);
	}
}

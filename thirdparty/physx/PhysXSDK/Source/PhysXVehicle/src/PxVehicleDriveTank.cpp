/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PxVehicleDriveTank.h"
#include "PxVehicleWheels.h"
#include "PxVehicleSDK.h"
#include "PxVehicleSuspWheelTire4.h"
#include "PxVehicleSuspLimitConstraintShader.h"
#include "PxVehicleDefaults.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"
#include "PsFoundation.h"
#include "PsUtilities.h"
#include "CmPhysXCommon.h"
#include "PxScene.h"
#include "CmUtils.h"

namespace physx
{

bool PxVehicleDriveTank::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleDrive::isValid(), "invalid PxVehicleDrive", false);
	PX_CHECK_AND_RETURN_VAL(mDriveSimData.isValid(), "Invalid PxVehicleDriveTank.mCoreSimData", false);
	return true;
}

PxVehicleDriveTank* PxVehicleDriveTank::allocate(const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN_NULL(numWheels>0, "Cars with zero wheels are illegal");
	PX_CHECK_AND_RETURN_NULL(0 == (numWheels % 2), "PxVehicleDriveTank::allocate - needs to have even number of wheels");

	//Compute the bytes needed.
	const PxU32 numWheels4 = (((numWheels + 3) & ~3) >> 2);
	const PxU32 byteSize = sizeof(PxVehicleDriveTank) + + PxVehicleDrive::computeByteSize(numWheels4);

	//Allocate the memory.
	PxVehicleDriveTank* veh = (PxVehicleDriveTank*)PX_ALLOC(byteSize, PX_DEBUG_EXP("PxVehicleDriveTank"));
	Cm::markSerializedMem(veh, byteSize);
	new(veh) PxVehicleDriveTank();

	//Patch up the pointers.
	PxU8* ptr = (PxU8*)veh + sizeof(PxVehicleDriveTank);
	PxVehicleDrive::patchupPointers(veh,ptr,numWheels4,numWheels);

	//Set the vehicle type.
	veh->mType = PxVehicleTypes::eDRIVETANK;

	//Set the default drive model.
	veh->mDriveModel = PxVehicleDriveTankControlModel::eSTANDARD;

	return veh;
}

void PxVehicleDriveTank::free()
{
	PxVehicleDrive::free();
}

void PxVehicleDriveTank::setup
(PxPhysics* physics, PxRigidDynamic* vehActor, 
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData& driveData,
 const PxU32 numDrivenWheels)
{
	PX_CHECK_AND_RETURN(driveData.isValid(), "PxVehicleDriveTank::setup - illegal drive data");

	//Set up the wheels.
	PxVehicleDrive::setup(physics,vehActor,wheelsData,numDrivenWheels,0);

	//Start setting up the drive.
	PX_CHECK_MSG(driveData.isValid(), "PxVehicle4WDrive - invalid driveData");

	//Copy the simulation data.
	mDriveSimData = driveData;
}

PxVehicleDriveTank* PxVehicleDriveTank::create
(PxPhysics* physics, PxRigidDynamic* vehActor, 
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData& driveData,
 const PxU32 numDrivenWheels)
{
	PxVehicleDriveTank* tank=PxVehicleDriveTank::allocate(numDrivenWheels);
	tank->setup(physics,vehActor,wheelsData,driveData,numDrivenWheels);
	return tank;
}


void PxVehicleDriveTank::setToRestState()
{
	//Set core to rest state.
	PxVehicleDrive::setToRestState();
}
} //namespace physx


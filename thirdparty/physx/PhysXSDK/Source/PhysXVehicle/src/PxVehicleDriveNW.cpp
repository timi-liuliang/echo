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

#include "PxVehicleDriveNW.h"
#include "PxVehicleDrive.h"
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


void PxVehicleDriveSimDataNW::setDiffData(const PxVehicleDifferentialNWData& diff)
{
	PX_CHECK_AND_RETURN(diff.isValid(), "Invalid PxVehicleCoreSimulationData.mDiff");
	mDiff=diff;
}

bool PxVehicleDriveSimDataNW::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(mDiff.isValid(), "Invalid PxVehicleDifferentialNWData", false);
	PX_CHECK_AND_RETURN_VAL(PxVehicleDriveSimData::isValid(), "Invalid PxVehicleDriveSimDataNW", false);
	return true;
}

///////////////////////////////////

bool PxVehicleDriveNW::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleDrive::isValid(), "invalid PxVehicleDrive", false);
	PX_CHECK_AND_RETURN_VAL(mDriveSimData.isValid(), "Invalid PxVehicleNW.mCoreSimData", false);
	return true;
}

PxVehicleDriveNW* PxVehicleDriveNW::allocate(const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN_NULL(numWheels>0, "Cars with zero wheels are illegal");

	//Compute the bytes needed.
	const PxU32 numWheels4 = (((numWheels + 3) & ~3) >> 2);
	const PxU32 byteSize = sizeof(PxVehicleDriveNW) + PxVehicleDrive::computeByteSize(numWheels4);

	//Allocate the memory.
	PxVehicleDriveNW* veh = (PxVehicleDriveNW*)PX_ALLOC(byteSize, PX_DEBUG_EXP("PxVehicleDriveNW"));
	Cm::markSerializedMem(veh, byteSize);
	new(veh) PxVehicleDriveNW();

	//Patch up the pointers.
	PxU8* ptr = (PxU8*)veh + sizeof(PxVehicleDriveNW);
	ptr=PxVehicleDrive::patchupPointers(veh,ptr,numWheels4,numWheels);

	//Set the vehicle type.
	veh->mType = PxVehicleTypes::eDRIVENW;

	return veh;
}

void PxVehicleDriveNW::free()
{
	PxVehicleDrive::free();
}

void PxVehicleDriveNW::setup
(PxPhysics* physics, PxRigidDynamic* vehActor,
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimDataNW& driveData,
 const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN(driveData.isValid(), "PxVehicleDriveNW::setup - invalid driveData");

	//Set up the wheels.
	PxVehicleDrive::setup(physics,vehActor,wheelsData,numWheels,0);

	//Start setting up the drive.
	PX_CHECK_MSG(driveData.isValid(), "PxVehicleNWDrive - invalid driveData");

	//Copy the simulation data.
	mDriveSimData = driveData;
}

PxVehicleDriveNW* PxVehicleDriveNW::create
(PxPhysics* physics, PxRigidDynamic* vehActor,
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimDataNW& driveData,
 const PxU32 numWheels)
{
	PxVehicleDriveNW* vehNW=PxVehicleDriveNW::allocate(numWheels);
	vehNW->setup(physics,vehActor,wheelsData,driveData,numWheels);
	return vehNW;
}


void PxVehicleDriveNW::setToRestState()
{
	//Set core to rest state.
	PxVehicleDrive::setToRestState();
}












} //namespace physx


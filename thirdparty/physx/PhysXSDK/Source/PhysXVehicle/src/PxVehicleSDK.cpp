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

#include "PxVehicleSDK.h"
#include "PxPhysics.h"
#include "PxTolerancesScale.h"
#include "CmPhysXCommon.h"
#include "PsFoundation.h"
#include "PsUtilities.h"
#include "PxVehicleDrive4W.h"
#include "PxMetaDataObjects.h"
#include "PxVehicleMetaDataObjects.h"
#include "PxVehicleSerialization.h"
#include "SnRepXSerializerImpl.h"
#include "PxSerializer.h"
#include "PxVehicleDriveTank.h"
#include "PxSerialFramework.h"
#include "PxMetaData.h"
#include "PxVehicleNoDrive.h"
#include "PxVehicleDriveNW.h"

namespace physx
{

void setVehicleToleranceScale(const PxTolerancesScale& ts);
void resetVehicleToleranceScale();
void setSerializationRegistryPtr(const PxSerializationRegistry* sr);
const PxSerializationRegistry* resetSerializationRegistryPtr();
#ifdef PX_PROFILE
void setupVehicleProfileZones(PxFoundation& foundation, PxProfileZoneManager* ptr);
void releaseVehicleProfileZones();
#endif

bool PxInitVehicleSDK(PxPhysics& physics, PxSerializationRegistry* sr)
{
	PX_ASSERT(static_cast<Ps::Foundation*>(&physics.getFoundation()) == &Ps::Foundation::getInstance());
	Ps::Foundation::incRefCount();
	setVehicleToleranceScale(physics.getTolerancesScale());

	setSerializationRegistryPtr(sr);
	if(sr)
	{
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleDrive4W,		PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleDrive4W>));
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleDriveTank,	PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleDriveTank>));
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleDriveNW,		PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleDriveNW>));
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleNoDrive,		PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleNoDrive>));
		
		sr->registerSerializer(PxVehicleConcreteType::eVehicleDrive4W,   		PX_NEW_SERIALIZER_ADAPTER(PxVehicleDrive4W));
		sr->registerSerializer(PxVehicleConcreteType::eVehicleDriveTank, 		PX_NEW_SERIALIZER_ADAPTER(PxVehicleDriveTank));
		sr->registerSerializer(PxVehicleConcreteType::eVehicleNoDrive,   		PX_NEW_SERIALIZER_ADAPTER(PxVehicleNoDrive));
		sr->registerSerializer(PxVehicleConcreteType::eVehicleDriveNW,   		PX_NEW_SERIALIZER_ADAPTER(PxVehicleDriveNW));

		sr->registerBinaryMetaDataCallback(PxVehicleDrive4W::getBinaryMetaData);	
		sr->registerBinaryMetaDataCallback(PxVehicleDriveTank::getBinaryMetaData);	
		sr->registerBinaryMetaDataCallback(PxVehicleNoDrive::getBinaryMetaData);
		sr->registerBinaryMetaDataCallback(PxVehicleDriveNW::getBinaryMetaData);
	}

#ifdef PX_PROFILE
	setupVehicleProfileZones(physics.getFoundation(), physics.getProfileZoneManager());
#endif // PX_PROFILE

	return true;
}

void PxCloseVehicleSDK(PxSerializationRegistry* sr)
{
	Ps::Foundation::decRefCount();
	resetVehicleToleranceScale();

	if (sr != resetSerializationRegistryPtr())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxCloseVehicleSDK called with different PxSerializationRegistry instance than PxInitVehicleSDK.");
		return;
	}

	if(sr)
	{
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleDrive4W));
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleDriveTank));
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleNoDrive));
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleDriveNW));
		
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleDrive4W));
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleDriveTank));
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleNoDrive));
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleDriveNW));
	}

#ifdef PX_PROFILE
	releaseVehicleProfileZones();
#endif // PX_PROFILE
}
/////////////////////////




}//physx


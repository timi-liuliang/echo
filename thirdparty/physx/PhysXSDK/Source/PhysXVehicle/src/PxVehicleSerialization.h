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

#ifndef PX_VEHICLE_SERIALIZATION_H
#define PX_VEHICLE_SERIALIZATION_H

#include "vehicle/PxVehicleSDK.h"
#include "extensions/PxRepXSimpleType.h"

namespace physx
{
	class PxRepXSerializer;
	class PxSerializationRegistry;
	
	template<typename TLiveType>
	struct RepXSerializerImpl;
	
	class XmlReader;
	class XmlMemoryAllocator;
	class XmlWriter;
	class MemoryBuffer;


	PX_DEFINE_TYPEINFO(PxVehicleNoDrive,		PxVehicleConcreteType::eVehicleNoDrive)
	PX_DEFINE_TYPEINFO(PxVehicleDrive4W,		PxVehicleConcreteType::eVehicleDrive4W)
	PX_DEFINE_TYPEINFO(PxVehicleDriveNW,		PxVehicleConcreteType::eVehicleDriveNW)
	PX_DEFINE_TYPEINFO(PxVehicleDriveTank,		PxVehicleConcreteType::eVehicleDriveTank)
	
	template<typename TVehicleType>
	struct PxVehicleRepXSerializer : public RepXSerializerImpl<TVehicleType>
	{
		PxVehicleRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<TVehicleType>( inCallback ) {}
		virtual PxRepXObject fileToObject( XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection );
		virtual void objectToFileImpl( const TVehicleType* , PxCollection* , XmlWriter& , MemoryBuffer& , PxRepXInstantiationArgs& );
		virtual TVehicleType* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};

}


#endif//PX_VEHICLE_REPX_SERIALIZER_H

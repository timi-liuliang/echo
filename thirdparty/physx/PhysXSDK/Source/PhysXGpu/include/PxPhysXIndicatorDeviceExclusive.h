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


#ifndef PX_PHYSX_INDICATOR_DEVICE_EXCLUSIVE_H
#define PX_PHYSX_INDICATOR_DEVICE_EXCLUSIVE_H

#include "PxPhysXCommonConfig.h"

namespace physx
{

/**
\brief API for gpu specific PhysX Indicator functionality.
*/
class PxPhysXIndicatorDeviceExclusive
{
public:

	/**
	\brief Register external Gpu client of PhysX Indicator.
	
	By calling this method, the PhysX Indicator will increment the number of external Cpu clients by one.

	\param[in] physics PxPhysics to register the client in.

	@see PxPhysXIndicatorDeviceExclusive.unregisterPhysXIndicatorGpuClient
	*/
	PX_PHYSX_CORE_API static void registerPhysXIndicatorGpuClient(class PxPhysics& physics);

	/**
	\brief Unregister external Gpu client of PhysX Indicator.
	
	By calling this method, the PhysX Indicator will decrement the number of external Cpu clients by one.

	\param[in] physics PxPhysics to unregister the client in.

	@see PxPhysXIndicatorDeviceExclusive.registerPhysXIndicatorGpuClient
	*/
	PX_PHYSX_CORE_API static void unregisterPhysXIndicatorGpuClient(class PxPhysics& physics);
};

}

#endif // PX_PHYSX_INDICATOR_DEVICE_EXCLUSIVE_H

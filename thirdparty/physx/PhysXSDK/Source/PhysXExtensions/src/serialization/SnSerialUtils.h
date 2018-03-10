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


#ifndef PX_PHYSICS_SN_SERIAL_UTILS
#define PX_PHYSICS_SN_SERIAL_UTILS

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PxBase.h"

namespace physx
{

class PxBase;

namespace Sn
{
	PxU32 getBinaryPlatformTag();
	bool isBinaryPlatformTagValid(PxU32 platformTag);
	const char* getBinaryPlatformName(PxU32 platformTag);
	bool checkCompatibility(const PxU32 version, const PxU32 binaryVersion);
	void getCompatibilityVersionsStr(char* buffer, PxU32 lenght);
}

}

#endif

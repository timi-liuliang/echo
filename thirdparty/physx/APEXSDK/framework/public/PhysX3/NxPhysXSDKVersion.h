/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_PHYSX_SDK_VERSION_H
#define NX_PHYSX_SDK_VERSION_H

/*!
\file
\brief Version identifiers for PhysX major release 3

	This file is intended to be included by NxApexDefs.h
*/

/// helper  Version number, this number should not be trusted for differentiating between different PhysX 3 versions
#define NX_SDK_VERSION_NUMBER 300

#include "common/PxPhysXCommonConfig.h"

/// helper PhysX version
#define NX_PHYSICS_SDK_VERSION  PX_PHYSICS_VERSION

/// helper  PhysX version, major part
#define NX_SDK_VERSION_MAJOR	PX_PHYSICS_VERSION_MAJOR

#endif // NX_PHYSX_SDK_VERSION_H

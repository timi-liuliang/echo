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

#ifndef GU_INTERSECTION_RAY_CAPSULE_H
#define GU_INTERSECTION_RAY_CAPSULE_H

#include "CmPhysXCommon.h"

namespace physx
{
namespace Gu
{
	class Capsule;
	PxU32 intersectRayCapsule(const PxVec3& origin, const PxVec3& dir, const Gu::Capsule& capsule, PxReal s[2]);
}
}

#endif

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

#ifndef GU_SWEEP_CAPSULE_CAPSULE_H
#define GU_SWEEP_CAPSULE_CAPSULE_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"

namespace physx
{
namespace Gu
{
	class Capsule;

	bool sweepCapsuleCapsule(const Capsule& lss0, const Capsule& lss1, const PxVec3& dir, PxReal length, PxReal& min_dist, PxVec3& ip, PxVec3& normal, PxU32 inHintFlags, PxU16& outHintFlags);

} // namespace Gu

}

#endif

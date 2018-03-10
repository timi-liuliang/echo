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

#ifndef GU_SWEEP_BOX_BOX_H
#define GU_SWEEP_BOX_BOX_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"
#include "PxQueryReport.h"

namespace physx
{
namespace Gu
{
	class Box;

	bool sweepBoxBox(const Box& box0, const Box& box1, const PxVec3& dir, PxReal length, PxVec3& hit, PxVec3& normal, PxReal& t, PxHitFlags hintFlags);

} // namespace Gu

}

#endif

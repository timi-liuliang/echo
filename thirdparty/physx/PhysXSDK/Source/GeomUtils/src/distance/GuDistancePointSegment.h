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

#ifndef GU_DISTANCE_POINT_SEGMENT_H
#define GU_DISTANCE_POINT_SEGMENT_H

#include "PxPhysXCommonConfig.h"
#include "GuSegment.h"

namespace physx
{
namespace Gu
{

	PX_PHYSX_COMMON_API PxReal distancePointSegmentSquared(const PxVec3& p0, const PxVec3& p1, const PxVec3& point, PxReal* param=NULL);

	PX_INLINE PxReal distancePointSegmentSquared(const Gu::Segment& segment, const PxVec3& point, PxReal* param=NULL)
	{
		return  distancePointSegmentSquared(segment.p0, segment.p1, point, param);
	}

} // namespace Gu

}

#endif

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

#ifndef GU_DISTANCE_SEGMENT_TRIANGLE_H
#define GU_DISTANCE_SEGMENT_TRIANGLE_H

#include "PxPhysXCommonConfig.h"
#include "GuSegment.h"

namespace physx
{
namespace Gu
{

	PX_PHYSX_COMMON_API PxReal distanceSegmentTriangleSquared(
		const PxVec3& segmentOrigin, const PxVec3& segmentExtent,
		const PxVec3& triangleOrigin, const PxVec3& triangleEdge0, const PxVec3& triangleEdge1,
		PxReal* t=NULL, PxReal* u=NULL, PxReal* v=NULL);

	PX_INLINE PxReal distanceSegmentTriangleSquared(
		const Gu::Segment& segment, 
		const PxVec3& triangleOrigin, 
		const PxVec3& triangleEdge0, 
		const PxVec3& triangleEdge1,
		PxReal* t=NULL, 
		PxReal* u=NULL, 
		PxReal* v=NULL)
	{
		return distanceSegmentTriangleSquared(
			segment.p0, segment.computeDirection(), triangleOrigin, triangleEdge0, triangleEdge1, t, u, v);
	}

} // namespace Gu

}

#endif

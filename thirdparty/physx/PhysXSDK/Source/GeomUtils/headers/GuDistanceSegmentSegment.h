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

#ifndef GU_DISTANCE_SEGMENT_SEGMENT_H
#define GU_DISTANCE_SEGMENT_SEGMENT_H

#include "common/PxPhysXCommonConfig.h"
#include "GuSegment.h"

namespace physx
{
namespace Gu
{

	PX_PHYSX_COMMON_API PxReal distanceSegmentSegmentSquaredOLD(const PxVec3& seg0_origin, const PxVec3& seg0_extent,
																const PxVec3& seg1_origin, const PxVec3& seg1_extent,
																PxReal* s = NULL, PxReal* t = NULL);

	PX_INLINE PxReal distanceSegmentSegmentSquared(const Gu::Segment& seg0, const Gu::Segment& seg1, PxReal* s = NULL, PxReal* t = NULL)
	{
		return distanceSegmentSegmentSquaredOLD(seg0.p0, seg0.computeDirection(),
												seg1.p0, seg1.computeDirection(),
												s, t);
	}

	// This version fixes accuracy issues from the "OLD" one, but needs to do 2 square roots in order
	// to find the normalized direction and length of the segments, and then
	// a division in order to renormalize the output
	PX_PHYSX_COMMON_API PxReal distanceSegmentSegmentSquared2(	const Gu::Segment& segment0,
																const Gu::Segment& segment1,
																PxReal* param0=NULL, 
																PxReal* param1=NULL);

} // namespace Gu

}

#endif

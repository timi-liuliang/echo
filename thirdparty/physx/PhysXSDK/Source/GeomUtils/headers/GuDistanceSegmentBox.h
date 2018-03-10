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

#ifndef GU_DISTANCE_SEGMENT_BOX_H
#define GU_DISTANCE_SEGMENT_BOX_H

#include "PxMat33.h"
#include "GuSegment.h"
#include "GuBox.h"

namespace physx
{
namespace Gu
{

	//! Compute the smallest distance from the (finite) line segment to the box.
	PX_PHYSX_COMMON_API PxReal distanceSegmentBoxSquared(	const PxVec3& segmentPoint0, const PxVec3& segmentPoint1,
										const PxVec3& boxOrigin, const PxVec3& boxExtent, const PxMat33& boxBase,
										PxReal* segmentParam = NULL,
										PxVec3* boxParam = NULL);

	PX_FORCE_INLINE PxReal distanceSegmentBoxSquared(const Gu::Segment& segment, const Gu::Box& box, PxReal* t = NULL, PxVec3* p = NULL)
	{
		return distanceSegmentBoxSquared(segment.p0, segment.p1, box.center, box.extents, box.rot, t, p);
	}

} // namespace Gu

}

#endif

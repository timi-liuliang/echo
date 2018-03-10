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

#ifndef GU_INTERSECTION_RAY_PLANE_H
#define GU_INTERSECTION_RAY_PLANE_H

#include "PxPlane.h"

namespace physx
{
namespace Gu
{
	// Returns true if line and plane are not parallel
	PX_INLINE bool intersectRayPlane(const PxVec3& orig, const PxVec3& dir, const PxPlane& plane, float& distanceAlongLine, PxVec3* pointOnPlane = NULL)
	{
		const float dn = dir.dot(plane.n);
		if(-1E-7 < dn && dn < 1E-7)
			return false; // parallel

		distanceAlongLine = -plane.distance(orig)/dn;

		if(pointOnPlane)
			*pointOnPlane = orig + distanceAlongLine * dir;

		return true;
	}

} // namespace Gu

}

#endif

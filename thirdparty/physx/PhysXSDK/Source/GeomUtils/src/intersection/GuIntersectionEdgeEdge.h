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

#ifndef GU_INTERSECTION_EDGE_EDGE_H
#define GU_INTERSECTION_EDGE_EDGE_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Gu
{

	// collide edge (p1,p2) moving in direction (dir) colliding
	// width edge (p3,p4). Return true on a collision with
	// collision distance (dist) and intersection point (ip)
	// note: dist and ip are invalid if function returns false.
	// note: ip is on (p1,p2), not (p1+dist*dir,p2+dist*dir)
	PX_PHYSX_COMMON_API bool intersectEdgeEdge(const PxVec3& p1, const PxVec3& p2, const PxVec3& dir, const PxVec3& p3, const PxVec3& p4, PxReal& dist, PxVec3& ip);

} // namespace Gu

}

#endif

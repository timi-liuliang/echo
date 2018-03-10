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

#ifndef GU_DISTANCE_POINT_TRIANGLE_H
#define GU_DISTANCE_POINT_TRIANGLE_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "PxVec3.h"

namespace physx
{
namespace Gu
{

	PX_PHYSX_COMMON_API PxVec3	closestPtPointTriangle(const PxVec3& p, const PxVec3& a, const PxVec3& b, const PxVec3& c, float& s, float& t);

	PX_FORCE_INLINE PxReal distancePointTriangleSquared(const PxVec3& point, 
														const PxVec3& triangleOrigin, 
														const PxVec3& triangleEdge0, 
														const PxVec3& triangleEdge1,
														PxReal* param0=NULL, 
														PxReal* param1=NULL)
	{
		const PxVec3 pt0 = triangleEdge0 + triangleOrigin;
		const PxVec3 pt1 = triangleEdge1 + triangleOrigin;
		float s,t;
		const PxVec3 cp = closestPtPointTriangle(point, triangleOrigin, pt0, pt1, s, t);
		if(param0)
			*param0 = s;
		if(param1)
			*param1 = t;
		return (cp - point).magnitudeSquared();
	}

} // namespace Gu

}

#endif

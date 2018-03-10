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

#include "GuDistancePointBox.h"

using namespace physx;

PxReal Gu::distancePointBoxSquared(	const PxVec3& point, 
									const PxVec3& boxOrigin, const PxVec3& boxExtent, const PxMat33& boxBase, 
									PxVec3* boxParam)
{
	// Compute coordinates of point in box coordinate system
	const PxVec3 diff = point - boxOrigin;

	PxVec3 closest(	boxBase.column0.dot(diff),
					boxBase.column1.dot(diff),
					boxBase.column2.dot(diff));
	
	// Project test point onto box
	PxReal sqrDistance = 0.0f;
	for(PxU32 ax=0; ax<3; ax++) 
	{
		if(closest[ax] < -boxExtent[ax])
		{
			const PxReal delta = closest[ax] + boxExtent[ax];
			sqrDistance += delta*delta;
			closest[ax] = -boxExtent[ax];
		}
		else if(closest[ax] > boxExtent[ax])
		{
			const PxReal delta = closest[ax] - boxExtent[ax];
			sqrDistance += delta*delta;
			closest[ax] = boxExtent[ax];
		}
	}
	
	if(boxParam) *boxParam = closest;
	
	return sqrDistance;
}

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

#include "GuDistancePointSegment.h"

using namespace physx;

/**
A segment is defined by S(t) = mP0 * (1 - t) + mP1 * t, with 0 <= t <= 1
Alternatively, a segment is S(t) = Origin + t * Direction for 0 <= t <= 1.
Direction is not necessarily unit length. The end points are Origin = mP0 and Origin + Direction = mP1.
*/
PxReal Gu::distancePointSegmentSquared(const PxVec3& p0, const PxVec3& p1, const PxVec3& point, PxReal* param)
{
	PxVec3 Diff = point - p0;
	const PxVec3 Dir = p1 - p0;
	PxReal fT = Diff.dot(Dir);

	if(fT<=0.0f)
	{
		fT = 0.0f;
	}
	else
	{
		const PxReal SqrLen = Dir.magnitudeSquared();
		if(fT>=SqrLen)
		{
			fT = 1.0f;
			Diff -= Dir;
		}
		else
		{
			fT /= SqrLen;
			Diff -= fT*Dir;
		}
	}

	if(param)	*param = fT;

	return Diff.magnitudeSquared();
}

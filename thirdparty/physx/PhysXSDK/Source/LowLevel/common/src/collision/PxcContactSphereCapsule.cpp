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
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"
#include "GuContactBuffer.h"
#include "GuGeomUtilsInternal.h"

using namespace physx;
using namespace Gu;

namespace physx
{
bool PxcContactSphereCapsule(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxCapsuleGeometry& shapeCapsule = shape1.get<const PxCapsuleGeometry>();

	//Sphere in world space
	const PxVec3& sphere = transform0.p;
	
	//Capsule in world space
	Gu::Segment segment;
	getCapsuleSegment(transform1, shapeCapsule, segment);
	
	const PxReal radiusSum = shapeSphere.radius + shapeCapsule.radius;
	const PxReal inflatedSum = radiusSum + contactDistance;

	// Collision detection
	PxReal u;
	const PxReal squareDist = Gu::distancePointSegmentSquared(segment, sphere, &u);

	if(squareDist < inflatedSum*inflatedSum)
	{
		PxVec3 normal = sphere - segment.getPointAt(u);
		
		//We do a *manual* normalization to check for singularity condition
		const PxReal lenSq = normal.magnitudeSquared();
		if(lenSq==0.0f) 
		{
			// PT: zero normal => pick up random one
			normal = PxVec3(1.0f, 0.0f, 0.0f);
		}
		else
		{
			normal *= PxRecipSqrt(lenSq);
		}
		const PxVec3 point = sphere - normal * shapeSphere.radius;

		contactBuffer.contact(point, normal, PxSqrt(squareDist) - radiusSum);
		return true;
	}
	return false;
}
}

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

#include "GuContactBuffer.h"
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"

using namespace physx;

namespace physx
{
bool PxcContactSphereSphere(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere0 = shape0.get<const PxSphereGeometry>();
	const PxSphereGeometry& shapeSphere1 = shape1.get<const PxSphereGeometry>();

	// Separation
	PxVec3 delta = transform0.p - transform1.p;

	const PxReal distanceSq = delta.magnitudeSquared();
	const PxReal radiusSum = shapeSphere0.radius + shapeSphere1.radius;
	const PxReal inflatedSum = radiusSum + contactDistance;

	if(distanceSq < inflatedSum*inflatedSum)
	{
		// Have some kind of contact
		const PxReal magn = PxSqrt(distanceSq);
		if(magn<=0.00001f)
		{
			// PT: spheres are exactly overlapping => can't create normal => pick up random one
			delta = PxVec3(1.0f, 0.0f, 0.0f);
		}
		else
		{
			// Normalize delta
			delta *= 1.0f/magn;
		}

		// PT: why is this formula different from the original code?
		const PxVec3 contact = delta * ((shapeSphere0.radius + magn - shapeSphere1.radius)*-0.5f) + transform0.p;
		
		contactBuffer.contact(contact, delta, magn - radiusSum);
		return true;
	}
	return false;
}
}

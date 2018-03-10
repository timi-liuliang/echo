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

#include "GuSweepSphereCapsule.h"
#include "GuSphere.h"
#include "GuCapsule.h"
#include "GuDistancePointSegment.h"
#include "GuSweepSphereSphere.h"
#include "GuIntersectionRayCapsule.h"

using namespace physx;
using namespace Gu;

bool Gu::sweepSphereCapsule(const Sphere& sphere, const Capsule& lss, const PxVec3& dir, PxReal length, PxReal& d, PxVec3& ip, PxVec3& nrm, PxHitFlags hintFlags)
{
	const PxReal radiusSum = lss.radius + sphere.radius;

	if(!(hintFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
	{
		// PT: test if shapes initially overlap
		if(distancePointSegmentSquared(lss.p0, lss.p1, sphere.center)<radiusSum*radiusSum)
		{
			d	= 0.0f;
			nrm	= -dir;
			return true;
		}
	}

	if(lss.p0 == lss.p1)
	{
		// Sphere vs. sphere
		if(sweepSphereSphere(sphere.center, sphere.radius, lss.p0, lss.radius, -dir*length, d, nrm))
		{
			d*=length;
//				if(hintFlags & PxHitFlag::ePOSITION)	// PT: TODO
				ip = sphere.center + nrm * sphere.radius;
			return true;
		}
		return false;
	}

	// Create inflated capsule
	Capsule Inflated(lss.p0, lss.p1, radiusSum);

	// Raycast against it
	PxReal s[2];
	PxU32 n = intersectRayCapsule(sphere.center, dir, Inflated, s);
	if(n)
	{
		PxReal t;
		if (n == 1)
			t = s[0];
		else
			t = (s[0] < s[1]) ? s[0]:s[1];

		if(t>=0.0f && t<=length)
		{
			d = t;

// PT: TODO:
//			const Ps::IntBool needsImpactPoint = hintFlags & PxHitFlag::ePOSITION;
//			if(needsImpactPoint || hintFlags & PxHitFlag::eNORMAL)
			{
				// Move capsule against sphere
				const PxVec3 tdir = t*dir;
				Inflated.p0 -= tdir;
				Inflated.p1 -= tdir;

				// Compute closest point between moved capsule & sphere
				distancePointSegmentSquared(Inflated, sphere.center, &t);
				Inflated.computePoint(ip, t);

				// Normal
				nrm = (ip - sphere.center);
				nrm.normalize();

//					if(needsImpactPoint)	// PT: TODO
					ip -= nrm * lss.radius;
			}
			return true;
		}
	}
	return false;
}

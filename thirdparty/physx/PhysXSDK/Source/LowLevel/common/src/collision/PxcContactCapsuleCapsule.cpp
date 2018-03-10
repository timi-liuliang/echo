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
#include "GuDistanceSegmentSegment.h"
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"

using namespace physx;

// See TTP 4617
#define USE_NEW_VERSION

namespace physx
{
bool PxcContactCapsuleCapsule(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	// Get actual shape data
	const PxCapsuleGeometry& shapeCapsule0 = shape0.get<const PxCapsuleGeometry>();
	const PxCapsuleGeometry& shapeCapsule1 = shape1.get<const PxCapsuleGeometry>();

	// Capsule-capsule contact generation
	Gu::Segment segment[2];	

	segment[0].p0 = transform0.q.getBasisVector0() * shapeCapsule0.halfHeight;
	segment[0].p1 = -segment[0].p0;

	segment[1].p0 = transform1.q.getBasisVector0() * shapeCapsule1.halfHeight;
	segment[1].p1 = -segment[1].p0;

	PxVec3 delta = transform1.p - transform0.p;
	segment[1].p1 += delta;
	segment[1].p0 += delta;

	// Collision detection
	PxReal s,t;
#ifdef USE_NEW_VERSION
	PxReal squareDist = Gu::distanceSegmentSegmentSquared2(segment[0], segment[1], &s, &t);
#else
	PxReal squareDist = PxcDistanceSegmentSegmentSquaredOLD(segment[0].point0, segment[0].direction(), segment[1].point0, segment[1].direction(), &s, &t);
#endif

	const PxReal radiusSum = shapeCapsule0.radius + shapeCapsule1.radius;
	const PxReal inflatedSum = radiusSum + contactDistance;
	const PxReal inflatedSumSquared = inflatedSum*inflatedSum;

	if(squareDist < inflatedSumSquared)
	{
		PxVec3 dir[2];
		dir[0] = segment[0].computeDirection();
		dir[1] = segment[1].computeDirection();

		PxReal segLen[2];
		segLen[0] = dir[0].magnitude();
		segLen[1] = dir[1].magnitude();

		if (segLen[0]) dir[0] *= 1.0f / segLen[0];
		if (segLen[1]) dir[1] *= 1.0f / segLen[1];

		if (PxAbs(dir[0].dot(dir[1])) > 0.9998f)	//almost parallel, ca. 1 degree difference --> generate two contact points at ends
		{
			PxU32 numCons = 0;

			PxReal segLenEps[2];
			segLenEps[0] = segLen[0] * 0.001f;//0.1% error is ok.
			segLenEps[1] = segLen[1] * 0.001f;
			
			//project the two end points of each onto the axis of the other and take those 4 points.
			//we could also generate a single normal at the single closest point, but this would be 'unstable'.

			for (PxU32 destShapeIndex = 0; destShapeIndex < 2; destShapeIndex ++)
			{
				for (PxU32 startEnd = 0; startEnd < 2; startEnd ++)
				{
					const PxU32 srcShapeIndex = 1-destShapeIndex;
					//project start/end of srcShapeIndex onto destShapeIndex.
					PxVec3 pos[2];
					pos[destShapeIndex] = startEnd ? segment[srcShapeIndex].p1 : segment[srcShapeIndex].p0;
					const PxReal p = dir[destShapeIndex].dot(pos[destShapeIndex] - segment[destShapeIndex].p0);
					if (p >= -segLenEps[destShapeIndex] && p <= (segLen[destShapeIndex] + segLenEps[destShapeIndex]))
					{
						pos[srcShapeIndex] = p * dir[destShapeIndex] + segment[destShapeIndex].p0;

						PxVec3 normal = pos[1] - pos[0];
						
						const PxReal normalLenSq = normal.magnitudeSquared();
						if (normalLenSq > 1e-6 && normalLenSq < inflatedSumSquared)
						{
							const PxReal distance = PxSqrt(normalLenSq);
							normal *= 1.0f/distance;
							PxVec3 point = pos[1] - normal * (srcShapeIndex ? shapeCapsule1 : shapeCapsule0).radius;
							point += transform0.p;
							contactBuffer.contact(point, normal, distance - radiusSum);
							numCons++;
						}					
					}
				}
			}

			if (numCons)	//if we did not have contacts, then we may have the case where they are parallel, but are stacked end to end, in which case the old code will generate good contacts.
				return true;
		}

		// Collision response
		PxVec3 pos1 = segment[0].getPointAt(s);
		PxVec3 pos2 = segment[1].getPointAt(t);

		PxVec3 normal = pos1 - pos2;

		const PxReal normalLenSq = normal.magnitudeSquared();
		if (normalLenSq < 1e-6)
		{
			// Zero normal -> pick the direction of segment 0.
			// Not always accurate but consistent with FW.
			if (segLen[0] > 1e-6)
				normal = dir[0];
			else 
				normal = PxVec3(1.0f, 0.0f, 0.0f);
		}
		else
		{
			normal *= PxRecipSqrt(normalLenSq);
		}
	
		pos1 += transform0.p;
		contactBuffer.contact(pos1 - normal * shapeCapsule0.radius, normal, PxSqrt(squareDist) - radiusSum);
		return true;
	}
	return false;
}
}



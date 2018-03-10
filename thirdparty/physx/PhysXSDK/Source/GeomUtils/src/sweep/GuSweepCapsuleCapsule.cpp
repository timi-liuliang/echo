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

#include "GuSweepCapsuleCapsule.h"
#include "GuCapsule.h"
#include "GuDistancePointSegment.h"
#include "GuDistanceSegmentSegment.h"
#include "GuIntersectionRayCapsule.h"
#include "PxQueryReport.h"
#include "PxTriangle.h"

using namespace physx;
using namespace Gu;

#define LOCAL_EPSILON 0.00001f	// PT: this value makes the 'basicAngleTest' pass. Fails because of a ray almost parallel to a triangle

static void edgeEdgeDist(PxVec3& x, PxVec3& y,				// closest points
						 const PxVec3& p, const PxVec3& a,	// seg 1 origin, vector
						 const PxVec3& q, const PxVec3& b)	// seg 2 origin, vector
{
	const PxVec3 T = q - p;
	const PxReal ADotA = a.dot(a);
	const PxReal BDotB = b.dot(b);
	const PxReal ADotB = a.dot(b);
	const PxReal ADotT = a.dot(T);
	const PxReal BDotT = b.dot(T);

	// t parameterizes ray (p, a)
	// u parameterizes ray (q, b)

	// Compute t for the closest point on ray (p, a) to ray (q, b)
	const PxReal Denom = ADotA*BDotB - ADotB*ADotB;

	PxReal t;
	if(Denom!=0.0f)	
	{
		t = (ADotT*BDotB - BDotT*ADotB) / Denom;

		// Clamp result so t is on the segment (p, a)
				if(t<0.0f)	t = 0.0f;
		else	if(t>1.0f)	t = 1.0f;
	}
	else
	{
		t = 0.0f;
	}

	// find u for point on ray (q, b) closest to point at t
	PxReal u;
	if(BDotB!=0.0f)
	{
		u = (t*ADotB - BDotT) / BDotB;

		// if u is on segment (q, b), t and u correspond to closest points, otherwise, clamp u, recompute and clamp t
		if(u<0.0f)
		{
			u = 0.0f;
			if(ADotA!=0.0f)
			{
				t = ADotT / ADotA;

						if(t<0.0f)	t = 0.0f;
				else	if(t>1.0f)	t = 1.0f;
			}
			else
			{
				t = 0.0f;
			}
		}
		else if(u > 1.0f)
		{
			u = 1.0f;
			if(ADotA!=0.0f)
			{
				t = (ADotB + ADotT) / ADotA;

						if(t<0.0f)	t = 0.0f;
				else	if(t>1.0f)	t = 1.0f;
			}
			else
			{
				t = 0.0f;
			}
		}
	}
	else
	{
		u = 0.0f;

		if(ADotA!=0.0f)
		{
			t = ADotT / ADotA;

					if(t<0.0f)	t = 0.0f;
			else	if(t>1.0f)	t = 1.0f;
		}
		else
		{
			t = 0.0f;
		}
	}

	x = p + a * t;
	y = q + b * u;
}

static bool rayQuad(const PxVec3& orig, const PxVec3& dir, const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2, PxReal& t, PxReal& u, PxReal& v, bool cull)
{
	// Find vectors for two edges sharing vert0
	const PxVec3 edge1 = vert1 - vert0;
	const PxVec3 edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	const PxVec3 pvec = dir.cross(edge2);

	// If determinant is near zero, ray lies in plane of triangle
	const PxReal det = edge1.dot(pvec);

	if(cull)
	{
		if(det<LOCAL_EPSILON)
			return false;

		// Calculate distance from vert0 to ray origin
		const PxVec3 tvec = orig - vert0;

		// Calculate U parameter and test bounds
		u = tvec.dot(pvec);
		if(u<0.0f || u>det)
			return false;

		// Prepare to test V parameter
		const PxVec3 qvec = tvec.cross(edge1);

		// Calculate V parameter and test bounds
		v = dir.dot(qvec);
		if(v<0.0f || v>det)
			return false;

		// Calculate t, scale parameters, ray intersects triangle
		t = edge2.dot(qvec);
		const PxReal oneOverDet = 1.0f / det;
		t *= oneOverDet;
		u *= oneOverDet;
		v *= oneOverDet;
	}
	else
	{
		// the non-culling branch
		if(det>-LOCAL_EPSILON && det<LOCAL_EPSILON)
			return false;
		const PxReal oneOverDet = 1.0f / det;

		// Calculate distance from vert0 to ray origin
		const PxVec3 tvec = orig - vert0;

		// Calculate U parameter and test bounds
		u = (tvec.dot(pvec)) * oneOverDet;
		if(u<0.0f || u>1.0f)
			return false;

		// prepare to test V parameter
		const PxVec3 qvec = tvec.cross(edge1);

		// Calculate V parameter and test bounds
		v = (dir.dot(qvec)) * oneOverDet;
		if(v<0.0f || v>1.0f)
			return false;

		// Calculate t, ray intersects triangle
		t = (edge2.dot(qvec)) * oneOverDet;
	}
	return true;
}

bool Gu::sweepCapsuleCapsule(const Capsule& lss0, const Capsule& lss1, const PxVec3& dir, PxReal length, PxReal& min_dist, PxVec3& ip, PxVec3& normal, PxU32 inHintFlags, PxU16& outHintFlags)
{
	const PxReal radiusSum = lss0.radius + lss1.radius;
	const PxVec3 center = lss1.computeCenter();

	if(!(inHintFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
	{
		// PT: test if shapes initially overlap

		// PT: It would be better not to use the same code path for spheres and capsules. The segment-segment distance
		// function doesn't work for degenerate capsules so we need to test all combinations here anyway.
		bool initialOverlapStatus;
		if(lss0.p0==lss0.p1)
			initialOverlapStatus = distancePointSegmentSquared(lss1, lss0.p0)<radiusSum*radiusSum;
		else if(lss1.p0==lss1.p1)
			initialOverlapStatus = distancePointSegmentSquared(lss0, lss1.p0)<radiusSum*radiusSum;
		else
			initialOverlapStatus = distanceSegmentSegmentSquared2(lss0, lss1)<radiusSum*radiusSum;
			
		if(initialOverlapStatus)
		{
			min_dist	= 0.0f;
			normal		= -dir;
			outHintFlags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
			return true;
		}
	}

	// 1. Extrude lss0 by lss1's length
	// 2. Inflate extruded shape by lss1's radius
	// 3. Raycast against resulting shape

	// Extrusion dir = capsule segment
	const PxVec3 D = (lss1.p1 - lss1.p0)*0.5f;

	const PxVec3 p0 = lss0.p0 - D;
	const PxVec3 p1 = lss0.p1 - D;
	const PxVec3 p0b = lss0.p0 + D;
	const PxVec3 p1b = lss0.p1 + D;

	PxTriangle T(p0b, p1b, p1);
	PxVec3 Normal;
	T.normal(Normal);

	PxReal MinDist = length;
	bool Status = false;

	PxVec3 pa,pb,pc;
	if((Normal.dot(dir)) >= 0)  // Same direction
	{
		Normal *= radiusSum;
		pc = p0 - Normal;
		pa = p1 - Normal;
		pb = p1b - Normal;
	}
	else
	{
		Normal *= radiusSum;
		pb = p0 + Normal;
		pa = p1 + Normal;
		pc = p1b + Normal;
	}
	PxReal t, u, v;
	if(rayQuad(center, dir, pa, pb, pc, t, u, v, true) && t>=0.0f && t<MinDist)
	{
		MinDist = t;
		Status = true;
	}

	// PT: optimization: if we hit one of the quad we can't possibly get a better hit, so let's skip all
	// the remaining tests!
	if(!Status)
	{
		Capsule Caps[4];
		Caps[0] = Capsule(p0, p1, radiusSum);
		Caps[1] = Capsule(p1, p1b, radiusSum);
		Caps[2] = Capsule(p1b, p0b, radiusSum);
		Caps[3] = Capsule(p0, p0b, radiusSum);

		// ### a lot of ray-sphere tests could be factored out of the ray-capsule tests...
		for(PxU32 i=0;i<4;i++)
		{
			PxReal s[2];
			PxU32 n = intersectRayCapsule(center, dir, Caps[i], s);
			if(n)
			{
				PxReal w;
				if (n == 1)
					w = s[0];
				else
					w = (s[0] < s[1]) ? s[0]:s[1];

				if(w>=0.0f && w<= MinDist)
				{
					MinDist = w;
					Status = true;
				}
			}
		}
	}

	if(Status)
	{
		outHintFlags = PxHitFlag::eDISTANCE;
		if(inHintFlags & (PxU32)(PxHitFlag::ePOSITION|PxHitFlag::eNORMAL))
		{
			const PxVec3 p00 = lss0.p0 - MinDist * dir;
			const PxVec3 p01 = lss0.p1 - MinDist * dir;
//			const PxVec3 p10 = lss1.p0;// - MinDist * dir;
//			const PxVec3 p11 = lss1.p1;// - MinDist * dir;

			const PxVec3 edge0 = p01 - p00;
			const PxVec3 edge1 = lss1.p1 - lss1.p0;

			PxVec3 x, y;
			edgeEdgeDist(x, y, p00, edge0, lss1.p0, edge1);

			if(inHintFlags & PxHitFlag::eNORMAL)
			{
				normal = (x - y);
				const float epsilon = 0.001f;
				if(normal.normalize()<epsilon)
				{
					// PT: happens when radiuses are zero
					normal = edge1.cross(edge0);
					if(normal.normalize()<epsilon)
					{
						// PT: happens when edges are parallel
						edgeEdgeDist(x, y, lss0.p0, lss0.p1 - lss0.p0, lss1.p0, edge1);
						normal = (x - y);
						normal.normalize();
					}
				}

				outHintFlags |= PxHitFlag::eNORMAL;
			}

			if(inHintFlags & PxHitFlag::ePOSITION)
			{
				ip = (lss1.radius*x + lss0.radius*y)/(lss0.radius+lss1.radius);
				outHintFlags |= PxHitFlag::ePOSITION;
			}
		}
		min_dist = MinDist;
	}
	return Status;
}

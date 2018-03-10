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

#include "GuSweepTriangleUtils.h"
#include "GuDistancePointTriangle.h"
#include "GuVecTriangle.h"
#include "GuVecBox.h"
#include "GuGJKWrapper.h"
#include "GuSweepBoxTriangle_FeatureBased.h"

using namespace physx;
using namespace Gu;
using namespace physx::shdfnd::aos;

#define GU_SAFE_DISTANCE_FOR_NORMAL_COMPUTATION 0.1f

void Gu::computeSphereTriImpactData(PxVec3& hit, PxVec3& normal, const PxVec3& center, const PxVec3& dir, float t, const PxTriangle& tri)
{
	const PxVec3 newSphereCenter = center + dir*t;

	// We need the impact point, not computed by the new code
	PxReal u, v;
	hit = closestPtPointTriangle(newSphereCenter, tri.verts[0], tri.verts[1], tri.verts[2], u, v);
	PX_UNUSED(u);
	PX_UNUSED(v);

	// This is responsible for the cap-vs-box stuck while jumping. However it's needed to slide on box corners!
	// PT: this one is also dubious since the sphere/capsule center can be far away from the hit point when the radius is big!
	normal = newSphereCenter - hit;
	const PxReal m = normal.normalize();
	if(m<1e-3f)
		tri.normal(normal);
}

// PT: not inlining this rarely-run function makes the benchmark ~500.000 cycles faster...
// PT: using this version all the time makes the benchmark ~300.000 cycles slower. So we just use it as a backup.
static bool runBackupProcedure(PxVec3& hit, PxVec3& normal, const PxVec3& localMotion, const PxVec3& boxExtents, const PxTriangle& triInBoxSpace)
{
	const Vec3V v0 = V3LoadU(triInBoxSpace.verts[0]);
	const Vec3V v1 = V3LoadU(triInBoxSpace.verts[1]);
	const Vec3V v2 = V3LoadU(triInBoxSpace.verts[2]);

	const TriangleV triangleV(v0, v1, v2);

	// PT: the box is in the triangle's space already
	const BoxV boxV(V3LoadU(PxVec3(0.0f)), V3LoadU(boxExtents), 
					V3LoadU(PxVec3(1.0f, 0.0f, 0.0f)), V3LoadU(PxVec3(0.0f, 1.0f, 0.0f)), V3LoadU(PxVec3(0.0f, 0.0f, 1.0f)));

	Ps::aos::Vec3V closestA;
	Ps::aos::Vec3V closestB;
	Ps::aos::Vec3V normalV;
	Ps::aos::FloatV sqDistV;
	//PxGJKStatus status_ = GJK(triangleV, boxV, closestA, closestB, normalV, sqDistV);
	PxGJKStatus status_ = GJKLocal(triangleV, boxV, closestA, closestB, normalV, sqDistV);

	if(status_!=GJK_NON_INTERSECT)
		return false;

	//PxVec3 ml_closestA;
	PxVec3 ml_closestB;
	PxVec3 ml_normal;
	//float ml_sqDist;
	//PxVec3_From_Vec3V(closestA, ml_closestA);
	V3StoreU(closestB, ml_closestB);
	V3StoreU(normalV, ml_normal);
	//PxF32_From_FloatV(sqDistV, &ml_sqDist);

	hit = ml_closestB + localMotion;
	normal = -ml_normal;
	return true;
}

void Gu::computeBoxTriImpactData(	PxVec3& hit, PxVec3& normal, const PxVec3& boxExtents, const PxVec3& localDir, const PxVec3& localMotion,
									const PxVec3& oneOverMotion, const PxTriangle& triInBoxSpace)
{
	// PT: the triangle is in "box space", i.e. the box can be seen as an AABB centered around the origin.

	// PT: compute impact point/normal in a second pass. Here we simply re-sweep the box against the best triangle,
	// using the feature-based code (which computes impact point and normal). This is not great because:
	// - we know there's an impact so why do all tests again?
	// - the SAT test & the feature-based tests could return different results because of FPU accuracy.
	// The backup procedure makes sure we compute a proper answer even when the SAT and feature-based versions differ.
	const PxBounds3 aabb(-boxExtents, boxExtents);

	float t = PX_MAX_F32;
	if(!sweepBoxTriangle(triInBoxSpace, aabb, localMotion, oneOverMotion, hit, normal, t))
	{
		// PT: move triangle close to box
		const PxVec3 delta = localMotion - localDir*GU_SAFE_DISTANCE_FOR_NORMAL_COMPUTATION;
		const PxTriangle movedTriangle(
			triInBoxSpace.verts[0] - delta,
			triInBoxSpace.verts[1] - delta,
			triInBoxSpace.verts[2] - delta);

		if(!runBackupProcedure(hit, normal, localMotion, boxExtents, movedTriangle))
		{
			// PT: if the backup procedure fails, we give up
			hit = PxVec3(0.0f);
			normal = -localDir;
		}
	}
}

// PT: copy where we know that input vectors are not zero
static PX_FORCE_INLINE void edgeEdgeDistNoZeroVector(	PxVec3& x, PxVec3& y,				// closest points
														const PxVec3& p, const PxVec3& a,	// seg 1 origin, vector
														const PxVec3& q, const PxVec3& b)	// seg 2 origin, vector
{
	const PxVec3 T = q - p;
	const PxReal ADotA = a.dot(a);
	const PxReal BDotB = b.dot(b);
	PX_ASSERT(ADotA!=0.0f);
	PX_ASSERT(BDotB!=0.0f);
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
	{
		u = (t*ADotB - BDotT) / BDotB;

		// if u is on segment (q, b), t and u correspond to closest points, otherwise, clamp u, recompute and clamp t
		if(u<0.0f)
		{
			u = 0.0f;
			t = ADotT / ADotA;

					if(t<0.0f)	t = 0.0f;
			else	if(t>1.0f)	t = 1.0f;
		}
		else if(u > 1.0f)
		{
			u = 1.0f;
			t = (ADotB + ADotT) / ADotA;

					if(t<0.0f)	t = 0.0f;
			else	if(t>1.0f)	t = 1.0f;
		}
	}

	x = p + a * t;
	y = q + b * u;
}

void Gu::computeEdgeEdgeNormal(PxVec3& normal, const PxVec3& p1, const PxVec3& p2_p1, const PxVec3& p3, const PxVec3& p4_p3, const PxVec3& dir, float d)
{
	// PT: cross-product doesn't produce nice normals so we use an edge-edge distance function itself

	// PT: move the edges "0.1" units from each other before the computation. If the edges are too far
	// away, computed normal tend to align itself with the swept direction. If the edges are too close,
	// closest points x and y become identical and we can't compute a proper normal.
	const PxVec3 p1s = p1 + dir*(d-GU_SAFE_DISTANCE_FOR_NORMAL_COMPUTATION);

	PxVec3 x, y;
	edgeEdgeDistNoZeroVector(x, y, p1s, p2_p1, p3, p4_p3);
	normal = x - y;
}

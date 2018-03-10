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

#include "GuSweepSphereTriangle.h"
#include "GuIntersectionRaySphere.h"
#include "GuIntersectionRayCapsule.h"
#include "GuSweepTriangleUtils.h"

using namespace physx;
using namespace Gu;

#define LOCAL_EPSILON 0.00001f	// PT: this value makes the 'basicAngleTest' pass. Fails because of a ray almost parallel to a triangle

// PT: special version computing (u,v) even when the ray misses the tri
static PX_FORCE_INLINE PxU32 rayTriSpecial(const PxVec3& orig, const PxVec3& dir, const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2, PxReal& t, PxReal& u, PxReal& v)
{
	// Find vectors for two edges sharing vert0
	const PxVec3 edge1 = vert1 - vert0;
	const PxVec3 edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	const PxVec3 pvec = dir.cross(edge2);

	// If determinant is near zero, ray lies in plane of triangle
	const PxReal det = edge1.dot(pvec);

	// the non-culling branch
	if(det>-LOCAL_EPSILON && det<LOCAL_EPSILON)
		return 0;
	const PxReal OneOverDet = 1.0f / det;

	// Calculate distance from vert0 to ray origin
	const PxVec3 tvec = orig - vert0;

	// Calculate U parameter
	u = (tvec.dot(pvec)) * OneOverDet;

	// prepare to test V parameter
	const PxVec3 qvec = tvec.cross(edge1);

	// Calculate V parameter
	v = (dir.dot(qvec)) * OneOverDet;

	if(u<0.0f || u>1.0f)
		return 1;
	if(v<0.0f || u+v>1.0f)
		return 1;

	// Calculate t, ray intersects triangle
	t = (edge2.dot(qvec)) * OneOverDet;

	return 2;
}

// Returns true if sphere can be tested against triangle vertex, false if edge test should be performed
//
// Uses a conservative approach to work for "sliver triangles" (long & thin) as well.
static SPU_INLINE bool edgeOrVertexTest(const PxVec3& planeIntersectPoint, const PxTriangle& tri, PxU32 vertIntersectCandidate, PxU32 vert0, PxU32 vert1, PxU32& secondEdgeVert)
{
	const PxVec3 edge0 = tri.verts[vertIntersectCandidate] - tri.verts[vert0];
	const PxReal edge0LengthSqr = edge0.dot(edge0);

	PxVec3 diff = planeIntersectPoint - tri.verts[vert0];

	if (edge0.dot(diff) < edge0LengthSqr)  // If the squared edge length is used for comparison, the edge vector does not need to be normalized
	{
		secondEdgeVert = vert0;
		return false;
	}

	const PxVec3 edge1 = tri.verts[vertIntersectCandidate] - tri.verts[vert1];
	const PxReal edge1LengthSqr = edge1.dot(edge1);

	diff = planeIntersectPoint - tri.verts[vert1];

	if (edge1.dot(diff) < edge1LengthSqr)
	{
		secondEdgeVert = vert1;
		return false;
	}
	return true;
}

static PX_FORCE_INLINE float squareDistance(const Segment& segment, const PxVec3& point)
{
	PxVec3 diff = point - segment.p0;
	const PxVec3& dir = segment.p1 - segment.p0;
	float fT = diff.dot(dir);

	if(fT<=0.0f)
	{
		fT = 0.0f;
	}
	else
	{
		const float sqrLen = dir.magnitudeSquared();
		if(fT>=sqrLen)
		{
			fT = 1.0f;
			diff -= dir;
		}
		else
		{
			fT /= sqrLen;
			diff -= fT*dir;
		}
	}
	return diff.magnitudeSquared();
}

bool Gu::sweepTriSphere(const PxTriangle& tri, const PxVec3& normal, const PxVec3& center, PxReal radius, const PxVec3& dir, PxReal& min_dist)
{
	// Ok, this new version is now faster than the original code. Needs more testing though.

	#define INTERSECT_POINT (tri.verts[1]*u) + (tri.verts[2]*v) + (tri.verts[0] * (1.0f-u-v))

	PxReal u,v;
	{
		PxVec3 R = normal * radius;
		if(dir.dot(R) >= 0.0f)
			R = -R;

		// The first point of the sphere to hit the triangle plane is the point of the sphere nearest to
		// the triangle plane. Hence, we use center - (normal*radius) below.

		// PT: casting against the extruded triangle in direction R is the same as casting from a ray moved by -R
		PxReal t;
//		int r = rayTriSpecial(center, dir, tri.mVerts[0]+R, tri.mVerts[1]+R, tri.mVerts[2]+R, t, u, v);
		int r = (int)rayTriSpecial(center-R, dir, tri.verts[0], tri.verts[1], tri.verts[2], t, u, v);
		if(!r)	return false;
		if(r==2)
		{
			if(t<0.0f)	return false;
			min_dist = t;
			return true;
		}
	}

	//
	// Let's do some art!
	//
	// The triangle gets divided into the following areas (based on the barycentric coordinates (u,v)):
	//
	//               \   A0    /
	//                 \      /
	//                   \   /
	//                     \/ 0
	//            A02      *      A01
	//   u /              /   \          \ v
	//    *              /      \         *
	//                  /         \						.
	//               2 /            \ 1
	//          ------*--------------*-------
	//               /                 \				.
	//        A2    /        A12         \   A1
	//
	//
	// Based on the area where the computed triangle plane intersection point lies in, a different sweep test will be applied.
	//
	// A) A01, A02, A12  : Test sphere against the corresponding edge
	// B) A0, A1, A2     : Test sphere against the corresponding vertex
	//
	// Unfortunately, B) does not work for long, thin triangles. Hence there is some extra code which does a conservative check and
	// switches to edge tests if necessary.
	//

	bool TestSphere;
	PxU32 e0,e1;
	if(u<0.0f)
	{
		if(v<0.0f)
		{
			// 0 or 0-1 or 0-2
			e0 = 0;
			PxVec3 intersectPoint = INTERSECT_POINT;
			TestSphere = edgeOrVertexTest(intersectPoint, tri, 0, 1, 2, e1);
		}
		else if(u+v>1.0f)
		{
			// 2 or 2-0 or 2-1
			e0 = 2;
			PxVec3 intersectPoint = INTERSECT_POINT;
			TestSphere = edgeOrVertexTest(intersectPoint, tri, 2, 0, 1, e1);
		}
		else
		{
			// 0-2
			TestSphere = false;
			e0 = 0;
			e1 = 2;
		}
	}
	else
	{
		if(v<0.0f)
		{
			if(u+v>1.0f)
			{
				// 1 or 1-0 or 1-2
				e0 = 1;
				PxVec3 intersectPoint = INTERSECT_POINT;
				TestSphere = edgeOrVertexTest(intersectPoint, tri, 1, 0, 2, e1);
			}
			else
			{
				// 0-1
				TestSphere = false;
				e0 = 0;
				e1 = 1;
			}
		}
		else
		{
			PX_ASSERT(u+v>=1.0f);	// Else hit triangle
			// 1-2
			TestSphere = false;
			e0 = 1;
			e1 = 2;
		}
	}

	if(TestSphere)
	{
		PxReal t;
//		if(intersectRaySphere(center, dir, min_dist*2.0f, tri.verts[e0], radius, t))
		if(intersectRaySphere(center, dir, PX_MAX_F32, tri.verts[e0], radius, t))
		{
			min_dist = t;
			return true;
		}
	}
	else
	{
		// PT: TODO: make this part of regular intersectRayCapsule function
		const Capsule capsule(tri.verts[e0], tri.verts[e1], radius);

		float l = ::squareDistance(capsule, center);
		l = PxSqrt(l) - radius - 10.0f;
		l = PxMax(l, 0.0f);

		PxReal s[2];
		PxU32 n = intersectRayCapsule(center + l*dir, dir, capsule, s);
		if(n)
		{
			PxReal t;
			if (n == 1)	t = s[0];
			else t = (s[0] < s[1]) ? s[0]:s[1];

			t+=l;

			if(t>=0.0f/* && t<MinDist*/)
			{
				min_dist = t;
				return true;
			}
		}
	}
	return false;
}

bool Gu::sweepSphereTriangles(	PxU32 nbTris, const PxTriangle* PX_RESTRICT triangles,								// Triangle data
								const PxVec3& center, const PxReal radius,											// Sphere data
								const PxVec3& unitDir, PxReal distance,												// Ray data
								const PxU32* PX_RESTRICT cachedIndex,												// Cache data
								PxVec3& _hit, PxVec3& _normal, PxReal& _t, PxU32& _index, PxHitFlags& _outFlags,	// Results
								PxVec3& _triNormal,
								bool isDoubleSided, bool meshBothSides)												// Query modifiers
{
	if(!nbTris)
		return false;

	const bool doBackfaceCulling = !isDoubleSided && !meshBothSides;

	PxU32 index = PX_INVALID_U32;
	const PxU32 initIndex = getInitIndex(cachedIndex, nbTris);

	PxReal curT = distance;
	const PxReal dpc0 = center.dot(unitDir);

	PxReal mostOpposingHitDot = 2.0f;

	PxVec3 bestTriNormal(0.0f);

	for(PxU32 ii=0; ii<nbTris; ii++)	// We need i for returned triangle index
	{
		const PxU32 i = getTriangleIndex(ii, initIndex);

		const PxTriangle& currentTri = triangles[i];

		if(rejectTriangle(center, unitDir, curT, radius, currentTri, dpc0))
			continue;

		PxVec3 triNormal;
		currentTri.denormalizedNormal(triNormal);

		// Backface culling
		if(doBackfaceCulling && (triNormal.dot(unitDir) > 0.0f))
			continue;

		const PxReal magnitude = triNormal.magnitude();
		if(magnitude==0.0f)
			continue;

		triNormal /= magnitude;

		PxReal currentDistance;
		if(!sweepTriSphere(currentTri, triNormal, center, radius, unitDir, currentDistance))
			continue;

		const PxReal distEpsilon = GU_EPSILON_SAME_DISTANCE; // pick a farther hit within distEpsilon that is more opposing than the previous closest hit
		const PxReal hitDot1 = computeAlignmentValue(triNormal, unitDir);
		if(!keepTriangle(currentDistance, hitDot1, curT, mostOpposingHitDot, distEpsilon))
			continue;

		curT = currentDistance;
		index = i;
		mostOpposingHitDot = hitDot1; // arbitrary bias. works for hitDot1=-1, prevHitDot=0
		bestTriNormal = triNormal;
	}
	if(index==PX_INVALID_U32)
		return false;	// We didn't touch any triangle

	// Compute impact data only once, using best triangle
	PxVec3 hit, normal;
	computeSphereTriImpactData(hit, normal, center, unitDir, curT, triangles[index]);

	// PT: by design, returned normal is opposed to the sweep direction.
	if(shouldFlipNormal(normal, meshBothSides, isDoubleSided, bestTriNormal, unitDir))
		normal = -normal;

	_hit		= hit;
	_normal		= normal;
	_t			= curT;
	_index		= index;
	_outFlags	= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL|PxHitFlag::ePOSITION;
	_triNormal	= bestTriNormal;
	return true;
}

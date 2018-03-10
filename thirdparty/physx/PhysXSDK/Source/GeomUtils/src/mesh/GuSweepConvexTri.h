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

#ifndef GU_SWEEP_CONVEX_TRI
#define GU_SWEEP_CONVEX_TRI

#include "GuVecTriangle.h"
#include "GuVecConvexHull.h"
#include "GuGJKWrapper.h"
#include "GuConvexMesh.h"
#include "PxConvexMeshGeometry.h"


// return true if hit, false if no hit
static PX_FORCE_INLINE bool sweepConvexVsTriangle(
	const PxVec3& v0, const PxVec3& v1, const PxVec3& v2,
	ConvexHullV& convexHull, const Ps::aos::PsMatTransformV& meshToConvex, const Ps::aos::PsTransformV& convexTransfV,
	const Ps::aos::Vec3VArg convexSpaceDir, const PxVec3& unitDir, const PxVec3& meshSpaceUnitDir,
	const Ps::aos::FloatVArg fullDistance, PxU32/* hintFlags*/, PxReal shrunkDistance,
	PxSweepHit& hit, bool isDoubleSided, const PxReal inflation, bool& initialOverlap)
{
	using namespace Ps::aos;
	// Create triangle normal
	const PxVec3 denormalizedNormal = (v1 - v0).cross(v2 - v1);

	// Backface culling
	// PT: WARNING, the test is reversed compared to usual because we pass -unitDir to this function
	const bool culled = !isDoubleSided && (denormalizedNormal.dot(meshSpaceUnitDir) <= 0.0f);
	if(culled)
		return false;

	const Vec3V zeroV = V3Zero();
	const FloatV zero = FZero();

	const Vec3V p0 = V3LoadU(v0); // in mesh local space
	const Vec3V	p1 = V3LoadU(v1);
	const Vec3V p2 = V3LoadU(v2);

	// transform triangle verts from mesh local to convex local space
	TriangleV triangleV(meshToConvex.transform(p0), meshToConvex.transform(p1), meshToConvex.transform(p2));

	FloatV toi;
	Vec3V closestA,normal;

	// run GJK raycast
	// sweep triangle in convex local space vs convex, closestA will be the impact point in convex local space
	bool gjkHit = GJKLocalRayCast(
		triangleV, convexHull, zero, zeroV, convexSpaceDir, toi, normal, closestA, inflation, false);
	if (gjkHit)
	{
		const FloatV minDist = FLoad(shrunkDistance);
		const Vec3V destWorldPointA = convexTransfV.transform(closestA);
		const Vec3V destNormal = V3Normalize(convexTransfV.rotate(normal));

		if(FAllGrtrOrEq(zero, toi))
		{
			hit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
			hit.distance	= 0.0f;
			hit.normal		= -unitDir;
			initialOverlap	= true;
			return true;

		}
		else
		{
			const FloatV dist = FMul(toi, fullDistance); // scale the toi to original full sweep distance
			if(FAllGrtr(minDist, dist)) // is current dist < minDist?
			{
				hit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
				V3StoreU(destWorldPointA, hit.position);
				V3StoreU(destNormal, hit.normal);
				FStore(dist, &hit.distance);
				return true; // report a hit
			}

		}
	}

	return false; // report no hit
}

#endif

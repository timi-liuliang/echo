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

#include "GuSweepTests.h"
#include "GuHeightFieldUtil.h"
#include "CmScaling.h"
#include "GuConvexMesh.h"
#include "GuIntersectionRayPlane.h"
#include "GuVecBox.h"
#include "GuVecCapsule.h"
#include "GuVecConvexHull.h"
#include "GuGJKWrapper.h"
#include "GuSweepMTD.h"
#include "PxConvexMeshGeometry.h"
#include "PxSphereGeometry.h"
#include "GuSweepSphereSphere.h"
#include "GuSweepSphereCapsule.h"
#include "GuSweepCapsuleCapsule.h"
#include "GuSweepSphereTriangle.h"
#include "GuSweepTriangleUtils.h"
#include "GuSweepCapsuleTriangle.h"

using namespace physx;
using namespace Gu;
using namespace Cm;
using namespace physx::shdfnd::aos;

static const PxReal gEpsilon = .01f;

static PxU32 computeSweepConvexPlane(
	const PxConvexMeshGeometry& convexGeom, ConvexHullData* hullData, const PxU32& nbPolys, const PxTransform& pose,
	const PxVec3& impact_, const PxVec3& unitDir)
{
	PX_ASSERT(nbPolys);

	const PxVec3 impact = impact_ - unitDir * gEpsilon;

	const PxVec3 localPoint = pose.transformInv(impact);
	const PxVec3 localDir = pose.rotateInv(unitDir);

	const FastVertex2ShapeScaling scaling(convexGeom.scale);

	PxU32 minIndex = 0;
	PxReal minD = PX_MAX_REAL;
	for(PxU32 j=0; j<nbPolys; j++)
	{
		const PxPlane& pl = hullData->mPolygons[j].mPlane;

		PxPlane plane;
		scaling.transformPlaneToShapeSpace(pl.n, pl.d, plane.n, plane.d);

		PxReal d = plane.distance(localPoint);
		if(d<0.0f)
			continue;

		const PxReal tweak = plane.n.dot(localDir) * gEpsilon;
		d += tweak;

		if(d<minD)
		{
			minIndex = j;
			minD = d;
		}
	}
	return minIndex;
}

/////////////////////////////////////////////////  sweepCapsule/Sphere  //////////////////////////////////////////////////////
bool sweepCapsule_SphereGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	const Sphere sphere(pose.p, sphereGeom.radius+inflation);

	if(!sweepSphereCapsule(sphere, lss, -unitDir, distance, sweepHit.distance, sweepHit.position, sweepHit.normal, hintFlags))
		return false;

	const bool isMtd = hintFlags & PxHitFlag::eMTD;

	if((!PX_IS_SPU) && isMtd)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

		if(sweepHit.distance == 0.f)
		{
			//intialOverlap
			if(lss.p0 == lss.p1)
			{
				//sphere
				return computeSphere_SphereMTD(sphere, Sphere(lss.p0, lss.radius), sweepHit);
			}
			else
			{
				//capsule
				return computeSphere_CapsuleMTD(sphere, lss, sweepHit);
			}
		}
	}
	else
	{
		if(sweepHit.distance!=0.0f)
			sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
		else
			sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
	}
	return true;
}

bool sweepCapsule_PlaneGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(geom);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom);

	const PxPlane& worldPlane = getPlane(pose);

	const PxF32 capsuleRadius = lss.radius + inflation;

	PxU32 index = 0;
	PxVec3 pts[2];

	PxReal minDp = PX_MAX_REAL;

	sweepHit.faceIndex	= 0xFFFFffff; // spec says face index is undefined for planes

	// Find extreme point on the capsule
	// AP: removed if (lss.p0 == lss.p1 clause because it wasn't properly computing minDp)
	pts[0] = lss.p0;
	pts[1] = lss.p1;
	for(PxU32 i=0; i<2; i++)
	{
		const PxReal dp = pts[i].dot(worldPlane.n);
		if(dp<minDp)
		{
			minDp = dp;
			index = i;
		}
	}

	const bool isMtd = hintFlags & PxHitFlag::eMTD;

	if(!(PX_IS_SPU) && isMtd)
	{
		//initial overlap with the plane
		if(minDp <= capsuleRadius - worldPlane.d)
		{
			sweepHit.flags			= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL| PxHitFlag::ePOSITION;
			return computePlane_CapsuleMTD(worldPlane, lss, sweepHit);
		}
	}
	else
	{
		if(!(hintFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
		{
			// test if the capsule initially overlaps with plane
			if(minDp <= capsuleRadius - worldPlane.d)
			{
				sweepHit.flags		= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL;
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
				return true;
			}
		}
	}

	const PxVec3 ptOnCapsule = pts[index] - worldPlane.n*capsuleRadius;

	// Raycast extreme vertex against plane
	bool hitPlane = intersectRayPlane(ptOnCapsule, unitDir, worldPlane, sweepHit.distance, &sweepHit.position);
	if(hitPlane && sweepHit.distance > 0 && sweepHit.distance <= distance)
	{
		sweepHit.normal = worldPlane.n;
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
		return true;
	}
	return false;
}

bool sweepCapsule_CapsuleGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	Capsule staticCapsule;
	getCapsule(staticCapsule, capsuleGeom, pose);
	staticCapsule.radius +=inflation;

	const bool isMtd = hintFlags & PxHitFlag::eMTD;

	PxU16 outFlags;
	if(sweepCapsuleCapsule(lss, staticCapsule, -unitDir, distance, sweepHit.distance, sweepHit.position, sweepHit.normal, hintFlags, outFlags))
	{
		sweepHit.flags = PxHitFlags(outFlags);
		if(sweepHit.distance == 0)
		{
			//initial overlap
			if(!(PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				return computeCapsule_CapsuleMTD(lss, staticCapsule, sweepHit);
			}
		}
		return true;
	}
	return false;
}

bool sweepCapsule_ConvexGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);

	using namespace Ps::aos;
	
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

	FETCH_CONVEX_HULL_DATA(convexGeom)

	PxReal _capsuleHalfHeight = 0.0f;
	const PxTransform capTransform = getCapsuleTransform(lss, _capsuleHalfHeight);

	const Vec3V zeroV = V3Zero();
	const FloatV zero = FZero();
	const FloatV dist = FLoad(distance);
	const Vec3V worldDir = V3LoadU(unitDir);

	const PsTransformV capPose = loadTransformU(capTransform);
	const PsTransformV convexPose = loadTransformU(pose);

	const PsMatTransformV aToB(convexPose.transformInv(capPose));

	//const PsMatTransformV aToB(pose.transformInv(capsuleTransform));

	const FloatV capsuleHalfHeight = FLoad(_capsuleHalfHeight);
	const FloatV capsuleRadius = FLoad(lss.radius);

	const Vec3V vScale = Vec3V_From_Vec4V(V4LoadU(&convexGeom.scale.scale.x));
	const QuatV vQuat = QuatVLoadU(&convexGeom.scale.rotation.x);

	
	CapsuleV capsule(aToB.p, aToB.rotate( V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);
	//CapsuleV capsule(zeroV, V3Scale(V3UnitX(), capsuleHalfHeight), capsuleRadius);
	ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);

	const Vec3V dir = convexPose.rotateInv(V3Neg(V3Scale(worldDir, dist)));

	bool isMtd = hintFlags & PxHitFlag::eMTD;

	FloatV toi;
	Vec3V closestA, normal;//closestA and normal is in the local space of convex hull
	bool hit  = GJKLocalRayCast(capsule, convexHull, zero, zeroV, dir, toi, normal, closestA,
		lss.radius + inflation, isMtd);

	if(hit)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

		if(FAllGrtrOrEq(zero, toi))
		{
			if(!(PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const FloatV length = toi;
				const Vec3V destNormal = V3Normalize(convexPose.rotate(normal));
				const Vec3V worldPointA = convexPose.transform(closestA);
				const Vec3V destWorldPointA = V3NegScaleSub(destNormal, length, worldPointA);
				V3StoreU(destNormal, sweepHit.normal);
				V3StoreU(destWorldPointA, sweepHit.position);
				FStore(length, &sweepHit.distance);
			}
			else
			{
				sweepHit.distance = 0.f;
				sweepHit.normal = -unitDir;
			}
		}
		else
		{
			sweepHit.flags |= PxHitFlag::ePOSITION;
			const Vec3V worldPointA = convexPose.transform(closestA);
			const FloatV length = FMul(dist, toi);
			const Vec3V destNormal = V3Normalize(convexPose.rotate(normal));
			const Vec3V destWorldPointA = V3ScaleAdd(worldDir, length, worldPointA);
			V3StoreU(destNormal, sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}
	
		// PT: compute closest polygon using the same tweak as in swept-capsule-vs-mesh
		sweepHit.faceIndex = computeSweepConvexPlane(convexGeom,hullData,nbPolys,pose,sweepHit.position,unitDir);
		//pxPrintf("fi = %d, pos=%.7f %.7f %.7f\n",
		//	sweepHit.faceIndex, sweepHit.position.x, sweepHit.position.y, sweepHit.position.z);
		return true;
	}
	return false;
}


/////////////////////////////////////////////////  sweepBox  //////////////////////////////////////////////////////

bool sweepBox_PlaneGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(geom);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom);

	sweepHit.faceIndex	= 0xFFFFffff; // spec says face index is undefined for planes

	PxPlane worldPlane = getPlane(pose);
	worldPlane.d -=inflation;

	// Find extreme point on the box
	PxVec3 boxPts[8];
	box.computeBoxPoints(boxPts);
	PxU32 index = 0;
	PxReal minDp = PX_MAX_REAL;
	for(PxU32 i=0;i<8;i++)
	{
		const PxReal dp = boxPts[i].dot(worldPlane.n);
	
		if(dp<minDp)
		{
			minDp = dp;
			index = i;
		}
	}

	bool isMtd = hintFlags & PxHitFlag::eMTD;

	if(!(PX_IS_SPU) && isMtd)
	{
		// test if box initially overlap with plane
		if(minDp <= -worldPlane.d)
		{
			sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
			//compute Mtd;
			return computePlane_BoxMTD(worldPlane, box, sweepHit);
		}
	}
	else
	{
		if(!(hintFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
		{
			// test if box initially overlap with plane
			if(minDp <= -worldPlane.d)
			{
				sweepHit.flags			= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL;
				sweepHit.distance		= 0.0f;
				sweepHit.normal			= -unitDir;
				return true;
			}
		}
	}

	// Raycast extreme vertex against plane
	bool hitPlane = intersectRayPlane(boxPts[index], unitDir, worldPlane, sweepHit.distance, &sweepHit.position);
	if(hitPlane && sweepHit.distance > 0 && sweepHit.distance <= distance)
	{
		sweepHit.normal = worldPlane.n;
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
		return true;
	}
	return false;
}


bool sweepBox_ConvexGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	using namespace Ps::aos;
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

	PX_ALIGN_PREFIX(16) PxTransform boxTransform PX_ALIGN_SUFFIX(16); boxTransform = box.getTransform();

	FETCH_CONVEX_HULL_DATA(convexGeom)

	const Vec3V zeroV = V3Zero();
	const FloatV zero = FZero();

	const PsTransformV boxPose = loadTransformA(boxTransform);
	const PsTransformV convexPose = loadTransformU(pose);

	const PsMatTransformV aToB(convexPose.transformInv(boxPose));

	const Vec3V boxExtents = V3LoadU(box.extents);

	const Vec3V vScale = V3LoadU(convexGeom.scale.scale);
	const QuatV vQuat = QuatVLoadU(&convexGeom.scale.rotation.x);
	
	BoxV boxV(zeroV, boxExtents);
	ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);

	const Vec3V worldDir = V3LoadU(unitDir);
	const FloatV dist = FLoad(distance);
	const Vec3V dir =convexPose.rotateInv(V3Neg(V3Scale(worldDir, dist)));

	bool isMtd = hintFlags & PxHitFlag::eMTD;

	FloatV toi;
	Vec3V closestA, normal;

	bool hit = GJKRelativeRayCast(boxV, convexHull, aToB, zero, zeroV, dir, toi, normal, closestA,
		inflation, isMtd);

	if(hit)
	{
		
		const Vec3V worldPointA = convexPose.transform(closestA);
		
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

		if(FAllGrtrOrEq(zero, toi))
		{
			//ML: initial overlap
			if(!(PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const Vec3V destNormal = V3Normalize(convexPose.rotate(normal));
				const FloatV length = toi;
				const Vec3V destWorldPointA =V3NegScaleSub(destNormal, length, worldPointA);
				V3StoreU(destNormal, sweepHit.normal);
				V3StoreU(destWorldPointA, sweepHit.position);
				FStore(length, &sweepHit.distance);
			}
			else
			{
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
			}
		}
		else
		{
			sweepHit.flags |= PxHitFlag::ePOSITION;
			const Vec3V destNormal = V3Normalize(convexPose.rotate(normal));
			const FloatV length = FMul(dist, toi);
			const Vec3V destWorldPointA = V3ScaleAdd(worldDir, length, worldPointA);
			V3StoreU(destNormal, sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}

		// PT: compute closest polygon using the same tweak as in swept-capsule-vs-mesh
		sweepHit.faceIndex = computeSweepConvexPlane(convexGeom,hullData,nbPolys,pose,sweepHit.position,unitDir);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Gu::SweepCapsuleTriangles(	PxU32 nbTris, const PxTriangle* triangles, bool doubleSided,
								const PxCapsuleGeometry& capsuleGeom, const PxTransform& capsulePose,
								const PxVec3& dir, const PxReal length, const PxU32* cachedIndex,
								PxVec3& hit, PxVec3& normal, PxReal& d, PxU32& index, const PxReal inflation, PxHitFlags hintFlags)
{
	// PT: trying to reuse already existing function.
	Capsule capsule;
	getCapsule(capsule, capsuleGeom, capsulePose);
	capsule.radius +=inflation;

	// Compute swept box
	Box capsuleBox;
	computeBoxAroundCapsule(capsule, capsuleBox);

	Box sweptBounds;
	computeSweptBox(sweptBounds, capsuleBox.extents, capsuleBox.center, capsuleBox.rot, dir, length);

	PxHitFlags unused;
	PxVec3 triNormal;
	return sweepCapsuleTriangles(nbTris, triangles, capsule, dir, length, cachedIndex, unused, d, normal, hit, index,
								triNormal, hintFlags, doubleSided, &sweptBounds);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool sweepConvex_SphereGeom(const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
							const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	FETCH_CONVEX_HULL_DATA(convexGeom)

	const Vec3V zeroV = V3Zero();
	const FloatV zero= FZero();

	const Vec3V vScale =  Vec3V_From_Vec4V(V4LoadU(&convexGeom.scale.scale.x));
	const QuatV vQuat = QuatVLoadU(&convexGeom.scale.rotation.x);

	const FloatV sphereRadius = FLoad(sphereGeom.radius);

	const PsTransformV sphereTransf = loadTransformU(pose);
	const PsTransformV convexTransf = loadTransformU(convexPose);

	const PsMatTransformV aToB(convexTransf.transformInv(sphereTransf));

	const Vec3V worldDir = V3LoadU(unitDir);
	const FloatV dist = FLoad(distance);
	const Vec3V dir = convexTransf.rotateInv(V3Scale(worldDir, dist));

	ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);
	//CapsuleV capsule(zeroV, sphereRadius);
	CapsuleV capsule(aToB.p, sphereRadius);

	const bool isMtd = hintFlags & PxHitFlag::eMTD;

	
	FloatV toi;
	Vec3V closestA, normal;
	//bool hit = GJKRelativeRayCast(capsule, convexHull, aToB, zero, zeroV, dir, toi, normal, closestA, inflation);
	bool hit = GJKLocalRayCast(capsule, convexHull, zero, zeroV, dir, toi, normal, closestA,
		sphereGeom.radius+inflation, isMtd);


	if(hit)
	{
		sweepHit.faceIndex = 0xffffffff;

		//closestA = V3NegScaleSub(normal, sphereRadius, closestA);
		const Vec3V destWorldPointA = convexTransf.transform(closestA);
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

		if(FAllGrtrOrEq(zero, toi))
		{
			//ML: initial overlap
			if(!(PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const Vec3V destNormal = V3Neg(V3Normalize(convexTransf.rotate(normal)));
				const FloatV length = toi;
				V3StoreU(destNormal, sweepHit.normal);
				V3StoreU(destWorldPointA, sweepHit.position);
				FStore(length, &sweepHit.distance);
			}
			else
			{
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
			}
		}
		else
		{
			sweepHit.flags |= PxHitFlag::ePOSITION;
			const Vec3V destNormal = V3Neg(V3Normalize(convexTransf.rotate(normal)));
			const FloatV length = FMul(dist, toi);
			V3StoreU(destNormal, sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}
		return true;
	}
	return false;
}

bool sweepConvex_PlaneGeom(	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
							const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(hintFlags);
	PX_UNUSED(geom);

	FETCH_CONVEX_HULL_DATA(convexGeom)

	sweepHit.faceIndex	= 0xFFFFffff; // spec says face index is undefined for planes

	const PxVec3* PX_RESTRICT hullVertices = hullData->getHullVertices();
	PxU32 numHullVertices = hullData->mNbHullVertices;

	const bool isMtd = hintFlags & PxHitFlag::eMTD;

	const FastVertex2ShapeScaling convexScaling(convexGeom.scale);

	PxPlane plane = getPlane(pose);
	plane.d -=inflation;

	sweepHit.distance	= distance;
	bool status = false;
	bool initialOverlap = false;
	while(numHullVertices--)
	{
		const PxVec3& vertex = *hullVertices++;
		const PxVec3 worldPt = convexPose.transform(convexScaling * vertex);
		float t;
		PxVec3 pointOnPlane;
		if(intersectRayPlane(worldPt, unitDir, plane, t, &pointOnPlane))
		{	
			
			if(plane.distance(worldPt) <= 0.0f)
			{
				initialOverlap = true;
				break;
				//// Convex touches plane
				//sweepHit.distance		= 0.0f;
				//sweepHit.flags			= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
				//sweepHit.normal			= -unitDir;
				//return true;
			}
			if(t > 0.0f && t <= sweepHit.distance)
			{
				sweepHit.distance	= t;
				sweepHit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
				sweepHit.position	= pointOnPlane;
				sweepHit.normal		= plane.n;
				status				= true;
			}
		}
	}

	if(initialOverlap)
	{
		if(!(PX_IS_SPU) && isMtd)
		{
			sweepHit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
			return computePlane_ConvexMTD(plane, convexGeom, convexPose, sweepHit);
		}
		else
		{
			sweepHit.distance		= 0.0f;
			sweepHit.flags			= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
			sweepHit.normal			= -unitDir;
			return true;
		}
	}
	return status;
}

bool sweepConvex_CapsuleGeom(	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
								const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	Capsule capsule;
	getCapsule(capsule, capsuleGeom, pose);

	if(sweepCapsule_ConvexGeom(convexGeom, convexPose, capsule, -unitDir, distance, sweepHit, hintFlags, inflation))
	{
		// do not write to position if it has not been set (initialOverlap)
		if (sweepHit.flags & PxHitFlag::ePOSITION)
		{
			sweepHit.position += unitDir * sweepHit.distance;
		}
		sweepHit.normal = -sweepHit.normal;
		sweepHit.faceIndex = 0xffffffff;
		return true;
	}
	return false;
}

bool sweepConvex_BoxGeom(	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
							const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	Box box;
	buildFrom1(box, pose.p, boxGeom.halfExtents, pose.q);

	//pxPrintf("sweepConvex begin\n");
	if(sweepBox_ConvexGeom(convexGeom, convexPose, box, -unitDir, distance, sweepHit, hintFlags, inflation))
	{
		// do not write to position if it has not been set (initialOverlap)
		if (sweepHit.flags & PxHitFlag::ePOSITION)
		{
			sweepHit.position += unitDir * sweepHit.distance;
		}
		sweepHit.normal = -sweepHit.normal;
		sweepHit.faceIndex = 0xffffffff;
		return true;
	}
	return false;
}

bool sweepConvex_ConvexGeom(const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
							const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	using namespace Ps::aos;
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	const PxConvexMeshGeometry& otherConvexGeom = static_cast<const PxConvexMeshGeometry&>(geom);
	ConvexMesh& otherConvexMesh = *static_cast<ConvexMesh*>(otherConvexGeom.convexMesh);

	FETCH_CONVEX_HULL_DATA(convexGeom)

// PT: TODO: find a way to use the FETCH_CONVEX_HULL_DATA macro for the second hull as well
#ifdef __SPU__
	PX_COMPILE_TIME_ASSERT(&((ConvexMesh*)NULL)->getHull()==NULL);
	
	PX_ALIGN_PREFIX(16)  PxU8 otherconvexMeshBuffer[sizeof(ConvexMesh)+32] PX_ALIGN_SUFFIX(16);
	ConvexMesh* otherMesh = memFetchAsync<ConvexMesh>(otherconvexMeshBuffer, (uintptr_t)(&otherConvexMesh), sizeof(ConvexMesh),1);
	memFetchWait(1); // convexMesh	

	PxU32 otherNPolys = otherMesh->getNbPolygonsFast();
	const HullPolygonData* PX_RESTRICT otherPolysEA = otherMesh->getPolygons();
	const PxU32 otherPolysSize = sizeof(HullPolygonData)*otherNPolys + sizeof(PxVec3)*otherMesh->getNbVerts();
	
 	//TODO: Need optimization with dma cache --jiayang
	void* otherHullBuffer = PxAlloca(CELL_ALIGN_SIZE_16(otherPolysSize+32));
	HullPolygonData* otherPolys = memFetchAsync<HullPolygonData>(otherHullBuffer, (uintptr_t)(otherPolysEA), otherPolysSize, 1);

	ConvexHullData* otherHullData = &otherMesh->getHull();
	otherHullData->mPolygons = otherPolys;

	memFetchWait(1); // convexMesh
#else
	ConvexHullData* otherHullData = &otherConvexMesh.getHull();	
#endif
	
	const Vec3V zeroV = V3Zero();
	const FloatV zero = FZero();

	const Vec3V otherVScale = V3LoadU(otherConvexGeom.scale.scale);
	const QuatV otherVQuat = QuatVLoadU(&otherConvexGeom.scale.rotation.x);

	const Vec3V vScale = Vec3V_From_Vec4V(V4LoadU(&convexGeom.scale.scale.x));
	const QuatV vQuat = QuatVLoadU(&convexGeom.scale.rotation.x);

	const PsTransformV otherTransf = loadTransformU(pose);
	const PsTransformV convexTransf = loadTransformU(convexPose);

	const Vec3V worldDir = V3LoadU(unitDir);
	const FloatV dist = FLoad(distance);
	const Vec3V dir = convexTransf.rotateInv(V3Scale(worldDir, dist));

	const PsMatTransformV aToB(convexTransf.transformInv(otherTransf));
	
	ConvexHullV otherConvexHull(otherHullData, zeroV, otherVScale, otherVQuat);
	ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);

	bool isMtd = hintFlags & PxHitFlag::eMTD;
	
	FloatV toi;
	Vec3V closestA, normal;
	bool hit = GJKRelativeRayCast(otherConvexHull, convexHull, aToB, zero, zeroV, dir, toi, normal, closestA,
		inflation, isMtd);

	if(hit)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

		if(FAllGrtrOrEq(zero, toi))
		{
			//initial overlap
			if(!(PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const Vec3V worldPointA = convexTransf.transform(closestA);
				const Vec3V destNormal = V3Neg(V3Normalize(convexTransf.rotate(normal)));
				const FloatV length = toi;
				V3StoreU(destNormal, sweepHit.normal);
				V3StoreU(worldPointA, sweepHit.position);
				FStore(length, &sweepHit.distance);
			}
			else
			{
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
			}
		}
		else
		{
			sweepHit.flags |= PxHitFlag::ePOSITION;
			const Vec3V worldPointA = convexTransf.transform(closestA);
			const Vec3V destNormal = V3Neg(V3Normalize(convexTransf.rotate(normal)));
			const FloatV length = FMul(dist, toi);
			V3StoreU(destNormal, sweepHit.normal);
			V3StoreU(worldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}

		// PT: compute closest polygon using the same tweak as in swept-capsule-vs-mesh
		sweepHit.faceIndex = computeSweepConvexPlane(convexGeom,hullData,nbPolys,pose,sweepHit.position,unitDir);
		return true;
	}
	return false;
}


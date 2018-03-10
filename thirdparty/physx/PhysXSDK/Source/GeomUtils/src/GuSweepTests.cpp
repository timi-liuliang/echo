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
#include "PxSphereGeometry.h"
#include "GuSweepSharedTests.h"  
#include "GuVecCapsule.h"
#include "GuVecBox.h"
#include "GuVecTriangle.h"
#include "GuGJKWrapper.h"
#include "GuSweepMTD.h"
#include "GuSweepTriangleUtils.h"

using namespace physx;
using namespace Gu;
using namespace Cm;
using namespace physx::shdfnd::aos;  



bool sweepCapsule_BoxGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_UNUSED(hintFlags);

	using namespace Ps::aos;
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	const FloatV zero = FZero();
	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents0 = V3LoadU(boxGeom.halfExtents);
	const FloatV dist = FLoad(distance);
	const Vec3V worldDir = V3LoadU(unitDir);

	PxReal _capsuleHalfHeight = 0.0f;
	const PxTransform capTransform = getCapsuleTransform(lss, _capsuleHalfHeight);

	const PsTransformV capPos = loadTransformU(capTransform);
	const PsTransformV boxPos = loadTransformU(pose);

	const PsMatTransformV aToB(boxPos.transformInv(capPos));

	const FloatV capsuleHalfHeight = FLoad(_capsuleHalfHeight);
	const FloatV capsuleRadius = FLoad(lss.radius);

	BoxV box(zeroV, boxExtents0);
	CapsuleV capsule(aToB.p, aToB.rotate(V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);

	const Vec3V dir = boxPos.rotateInv(V3Neg(V3Scale(worldDir, dist)));

	const bool isMtd = hintFlags & PxHitFlag::eMTD;
	FloatV toi = FMax();
	Vec3V closestA, normal;//closestA and normal is in the local space of box
	bool hit  = GJKLocalRayCast(capsule, box, zero, zeroV, dir, toi, normal, closestA, lss.radius + inflation, isMtd);

	if(hit)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
		if(FAllGrtrOrEq(zero, toi))
		{
			//initial overlap
			if((!PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const Vec3V worldPointA = boxPos.transform(closestA);
				const Vec3V destNormal = boxPos.rotate(normal);
				const FloatV length = toi;
				const Vec3V destWorldPointA = V3NegScaleSub(destNormal, length, worldPointA);
				V3StoreU(destWorldPointA, sweepHit.position);
				V3StoreU(destNormal, sweepHit.normal);
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
			const Vec3V worldPointA = boxPos.transform(closestA);
			const Vec3V destNormal = boxPos.rotate(normal);
			const FloatV length = FMul(dist, toi);
			const Vec3V destWorldPointA = V3ScaleAdd(worldDir, length, worldPointA);
			V3StoreU(destNormal, sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);

		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool sweepBox_SphereGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	PX_UNUSED(hintFlags);
	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	const FloatV zero = FZero();
	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents = V3LoadU(box.extents);
	const FloatV worldDist = FLoad(distance);
	const Vec3V  unitDirV = V3LoadU(unitDir);

	const FloatV sphereRadius = FLoad(sphereGeom.radius);

	/* 
		DE10168
		original code:
		const PxTransform boxWorldPose = box.getTransform(); 
		starting with SDK 1.700 it breaks various PS4 non-debug tests:
		SqTestSweep and RecoveryModuleTest

		for some unknown reason, the vector elements end up in the wrong location
		(in y where it should be in x it seems)

		while we investigate the real cause, use the equivalent code directly
	*/
	const PxTransform boxWorldPose = PxTransform(box.center, PxQuat(box.rot));
	
	const PsTransformV spherePos = loadTransformU(pose);
	const PsTransformV boxPos = loadTransformU(boxWorldPose);

	const PsMatTransformV aToB(boxPos.transformInv(spherePos));

	BoxV boxV(zeroV, boxExtents);
	CapsuleV capsuleV(aToB.p, sphereRadius);

	//transform into b space
	const Vec3V dir = boxPos.rotateInv(V3Scale(unitDirV, worldDist));

	bool isMtd = hintFlags & PxHitFlag::eMTD;
	FloatV toi;
	Vec3V closestA, normal;//closestA and normal is in the local space of box
	bool hit  = GJKLocalRayCast(capsuleV, boxV, zero, zeroV, dir, toi, normal, closestA, sphereGeom.radius+inflation, isMtd);

	if(hit)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

		//initial overlap
		if(FAllGrtrOrEq(zero, toi))
		{
			if((!PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const Vec3V destWorldPointA = boxPos.transform(closestA);
				const Vec3V destNormal = V3Neg(boxPos.rotate(normal));
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
			const Vec3V destWorldPointA = boxPos.transform(closestA);
			const Vec3V destNormal = V3Neg(boxPos.rotate(normal));
			const FloatV length = FMul(worldDist, toi);
			V3StoreU(destNormal, sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}
		return true;
	}
	return false;
}

bool sweepBox_CapsuleGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	using namespace Ps::aos;
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	PX_UNUSED(hintFlags);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	const FloatV capsuleHalfHeight = FLoad(capsuleGeom.halfHeight);
	const FloatV capsuleRadius = FLoad(capsuleGeom.radius);

	const FloatV zero = FZero();
	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents = V3LoadU(box.extents);
	const FloatV worldDist = FLoad(distance);
	const Vec3V  unitDirV = V3LoadU(unitDir);

	const PxTransform boxWorldPose = box.getTransform();

	const PsTransformV capPos = loadTransformU(pose);
	const PsTransformV boxPos = loadTransformU(boxWorldPose);

	const PsMatTransformV aToB(boxPos.transformInv(capPos));

	BoxV boxV(zeroV, boxExtents);
	CapsuleV capsuleV(aToB.p, aToB.rotate(V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);

	//transform into b space
	const Vec3V dir = boxPos.rotateInv(V3Scale(unitDirV, worldDist));

	const bool isMtd = hintFlags & PxHitFlag::eMTD;
	FloatV toi;
	Vec3V closestA, normal;//closestA and normal is in the local space of box
	bool hit  = GJKLocalRayCast(capsuleV, boxV, zero, zeroV, dir, toi, normal, closestA, capsuleGeom.radius+inflation, isMtd);

	if(hit)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

		//initial overlap
		if(FAllGrtrOrEq(zero, toi))
		{
			if((!PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				//initial overlap is toi < 0 
				const FloatV length = toi;
				const Vec3V destWorldPointA = boxPos.transform(closestA);
				const Vec3V destNormal = boxPos.rotate(normal);
				V3StoreU(V3Neg(destNormal), sweepHit.normal);
				V3StoreU(destWorldPointA, sweepHit.position);
				FStore(length, &sweepHit.distance);
			}
			else
			{
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
			}
			return true;
		}
		else
		{
			sweepHit.flags |= PxHitFlag::ePOSITION;
			const Vec3V destWorldPointA = boxPos.transform(closestA);
			const Vec3V destNormal = boxPos.rotate(normal);
			const FloatV length = FMul(worldDist, toi);
			V3StoreU(V3Neg(destNormal), sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}
		
		return true;	
	}
	return false;
}

bool sweepBox_BoxGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	const FloatV zero = FZero();
	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents0 = V3LoadU(boxGeom.halfExtents);
	const Vec3V boxExtents1 = V3LoadU(box.extents);
	const FloatV worldDist = FLoad(distance);
	const Vec3V  unitDirV = V3LoadU(unitDir);

	const PxTransform boxWorldPose = box.getTransform();

	const PsTransformV boxTrans0 = loadTransformU(pose);
	const PsTransformV boxTrans1 = loadTransformU(boxWorldPose);

	const PsMatTransformV aToB(boxTrans1.transformInv(boxTrans0));

	BoxV box0(zeroV, boxExtents0);
	BoxV box1(zeroV, boxExtents1);

	//transform into b space
	const Vec3V dir = boxTrans1.rotateInv(V3Scale(unitDirV, worldDist));
	const bool isMtd = hintFlags & PxHitFlag::eMTD;
	FloatV toi;
	Vec3V closestA, normal;//closestA and normal is in the local space of box
	bool hit  = GJKRelativeRayCast(box0, box1, aToB, zero, zeroV, dir, toi, normal, closestA, inflation, isMtd);
	
	if(hit)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
		if(FAllGrtrOrEq(zero, toi))
		{
			if((!PX_IS_SPU) && isMtd)
			{
				sweepHit.flags |= PxHitFlag::ePOSITION;
				const FloatV length = toi;
				const Vec3V destWorldPointA = boxTrans1.transform(closestA);
				const Vec3V destNormal = V3Normalize(boxTrans1.rotate(normal));
				V3StoreU(V3Neg(destNormal), sweepHit.normal);
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
			const Vec3V destWorldPointA = boxTrans1.transform(closestA);
			const Vec3V destNormal = V3Normalize(boxTrans1.rotate(normal));
			const FloatV length = FMul(worldDist, toi);
			V3StoreU(V3Neg(destNormal), sweepHit.normal);
			V3StoreU(destWorldPointA, sweepHit.position);
			FStore(length, &sweepHit.distance);
		}
		return true;
	}
	return false;
}

bool Gu::SweepBoxTriangles(	PxU32 nbTris, const PxTriangle* triangles, bool isDoubleSided,
							const PxBoxGeometry& boxGeom, const PxTransform& boxPose, const PxVec3& dir, const PxReal length, PxVec3& _hit,
							PxVec3& _normal, float& _d, PxU32& _index, const PxU32* cachedIndex, const PxReal inflation, PxHitFlags hintFlags)
{
	PX_UNUSED(hintFlags);

	if(!nbTris)
		return false;

	const bool meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	const bool doBackfaceCulling = !isDoubleSided && !meshBothSides;

	Box box;
	buildFrom1(box, boxPose.p, boxGeom.halfExtents, boxPose.q);

	PxSweepHit sweepHit;
	// Move to AABB space
	Matrix34 worldToBox;
	computeWorldToBoxMatrix(worldToBox, box);

	const PxVec3 localDir = worldToBox.rotate(dir);
	const PxVec3 localMotion = localDir * length;

	const Vec3V base0 = V3LoadU(worldToBox.base0);
	const Vec3V base1 = V3LoadU(worldToBox.base1);
	const Vec3V base2 = V3LoadU(worldToBox.base2);
	const Mat33V matV(base0, base1, base2);
	const Vec3V p	  = V3LoadU(worldToBox.base3);
	const PsMatTransformV worldToBoxV(p, matV);

	const FloatV zero = FZero();
	const Vec3V zeroV = V3Zero();
	const BoolV bTrue = BTTTT();
	const Vec3V boxExtents = V3LoadU(box.extents);
	const Vec3V boxDir = V3LoadU(localDir);
	const FloatV inflationV = FLoad(inflation);
	const Vec3V absBoxDir = V3Abs(boxDir);
	const FloatV boxRadiusV = FAdd(V3Dot(absBoxDir, boxExtents), inflationV);
	BoxV boxV(zeroV, boxExtents);

#ifdef PX_DEBUG
	PxU32 totalTestsExpected = nbTris;
	PxU32 totalTestsReal = 0;
	PX_UNUSED(totalTestsExpected);
	PX_UNUSED(totalTestsReal);
#endif

	Vec3V boxLocalMotion = V3LoadU(localMotion);
	Vec3V minClosestA = zeroV, minNormal = zeroV;
	PxU32 minTriangleIndex = 0;
	PxVec3 bestTriNormal(0.0f);
	FloatV dist = FLoad(length);

	const PsTransformV boxPos = loadTransformU(boxPose);

	bool status = false;

	const PxU32 idx = cachedIndex ? *cachedIndex : 0;

	for(PxU32 ii=0;ii<nbTris;ii++)
	{
		const PxU32 triangleIndex = getTriangleIndex(ii, idx);

		const Vec3V localV0 =  V3LoadU(triangles[triangleIndex].verts[0]);
		const Vec3V localV1 =  V3LoadU(triangles[triangleIndex].verts[1]);
		const Vec3V localV2 =  V3LoadU(triangles[triangleIndex].verts[2]);

		const Vec3V triV0 = worldToBoxV.transform(localV0);
		const Vec3V triV1 = worldToBoxV.transform(localV1);
		const Vec3V triV2 = worldToBoxV.transform(localV2);

		const Vec3V triNormal = V3Cross(V3Sub(triV2, triV1),V3Sub(triV0, triV1)); 

		if(doBackfaceCulling && FAllGrtrOrEq(V3Dot(triNormal, boxLocalMotion), zero)) // backface culling
			continue;

		const FloatV dp0 = V3Dot(triV0, boxDir);
		const FloatV dp1 = V3Dot(triV1, boxDir);
		const FloatV dp2 = V3Dot(triV2, boxDir);
		
		const FloatV dp = FMin(dp0, FMin(dp1, dp2));

		const Vec3V dpV = V3Merge(dp0, dp1, dp2);

		const FloatV temp1 = FAdd(boxRadiusV, dist);
		const BoolV con0 = FIsGrtr(dp, temp1);
		const BoolV con1 = V3IsGrtr(zeroV, dpV);

		if(BAllEq(BOr(con0, con1), bTrue))
			continue;

#ifdef PX_DEBUG
		totalTestsReal++;
#endif

		TriangleV triangleV(triV0, triV1, triV2);
		
		FloatV lambda;   
		Vec3V closestA, normal;//closestA and normal is in the local space of convex hull
		bool hit  = GJKLocalRayCast(triangleV, boxV, zero, zeroV, boxLocalMotion, lambda, normal, closestA, inflation, false); 
		
		if(hit)
		{
			//hitCount++;
		
			if(FAllGrtrOrEq(zero, lambda))
			{
				_d		= 0.0f;
				_index	= triangleIndex;
				_normal	= -dir;
				return true;
			}

			dist = FMul(dist,lambda);
			boxLocalMotion = V3Scale(boxDir, dist);  
			minClosestA = closestA;
			minNormal = normal;
			minTriangleIndex = triangleIndex;
			V3StoreU(triNormal, bestTriNormal);
			status = true;
		}
	}

	if(status)
	{
		_index	= minTriangleIndex;
		const Vec3V destNormal = V3Neg(V3Normalize(boxPos.rotate(minNormal)));
		const Vec3V destWorldPointA = boxPos.transform(minClosestA);
		V3StoreU(destNormal, _normal);
		V3StoreU(destWorldPointA, _hit);
		FStore(dist, &_d);

		// PT: by design, returned normal is opposed to the sweep direction.
		if(shouldFlipNormal(_normal, meshBothSides, isDoubleSided, bestTriNormal, dir))
			_normal = -_normal;

		return true;
	}
	return false;
}

extern Gu::SweepConvexFunc gSweepConvexMap[7];

extern bool sweepCapsule_SphereGeom			(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCapsule_PlaneGeom			(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCapsule_CapsuleGeom		(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCapsule_BoxGeom			(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCCTCapsule_BoxGeom			(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCapsule_ConvexGeom			(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCapsule_MeshGeom			(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCCTCapsule_MeshGeom		(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCapsule_HeightFieldGeom	(GU_CAPSULE_SWEEP_FUNC_PARAMS);
extern bool sweepCCTCapsule_HeightFieldGeom	(GU_CAPSULE_SWEEP_FUNC_PARAMS);

extern bool sweepBox_SphereGeom			(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepCCTBox_SphereGeom		(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepBox_PlaneGeom			(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepBox_CapsuleGeom		(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepCCTBox_CapsuleGeom		(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepBox_BoxGeom			(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepCCTBox_BoxGeom			(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepBox_ConvexGeom			(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepBox_MeshGeom			(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepBox_HeightFieldGeom	(GU_BOX_SWEEP_FUNC_PARAMS);
extern bool sweepCCTBox_HeightFieldGeom	(GU_BOX_SWEEP_FUNC_PARAMS);

bool sweepConvex_SphereGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);
bool sweepConvex_CapsuleGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);
bool sweepConvex_PlaneGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);
bool sweepConvex_BoxGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);
bool sweepConvex_ConvexGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);
bool sweepConvex_MeshGeom(
	const PxGeometry& aMeshGeom, const PxTransform& meshPose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);
bool sweepConvex_HeightFieldGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation);

Gu::GeomSweepFuncs gGeomSweepFuncs =
{
	{
		sweepCapsule_SphereGeom,
		sweepCapsule_PlaneGeom,
		sweepCapsule_CapsuleGeom,
		sweepCapsule_BoxGeom,
		sweepCapsule_ConvexGeom,
		sweepCapsule_MeshGeom,
		sweepCapsule_HeightFieldGeom
	},
	{
		sweepCapsule_SphereGeom,
		sweepCapsule_PlaneGeom,
		sweepCapsule_CapsuleGeom,
		PX_IS_SPU ? sweepCapsule_BoxGeom : sweepCCTCapsule_BoxGeom,
		sweepCapsule_ConvexGeom,
		sweepCapsule_MeshGeom ,
		sweepCapsule_HeightFieldGeom
	},
	{
		sweepBox_SphereGeom,
		sweepBox_PlaneGeom,
		sweepBox_CapsuleGeom,
		sweepBox_BoxGeom,
		sweepBox_ConvexGeom,
		sweepBox_MeshGeom,
		sweepBox_HeightFieldGeom
	},
	{
		PX_IS_SPU ? sweepBox_SphereGeom : sweepCCTBox_SphereGeom,
		sweepBox_PlaneGeom,
		PX_IS_SPU ? sweepBox_CapsuleGeom : sweepCCTBox_CapsuleGeom,
		PX_IS_SPU ? sweepBox_BoxGeom : sweepCCTBox_BoxGeom,
		sweepBox_ConvexGeom,
		sweepBox_MeshGeom,
		PX_IS_SPU ? sweepBox_HeightFieldGeom : sweepCCTBox_HeightFieldGeom
	},
	{
		sweepConvex_SphereGeom,		// 0
		sweepConvex_PlaneGeom,		// 1
		sweepConvex_CapsuleGeom,	// 2
		sweepConvex_BoxGeom,		// 3
		sweepConvex_ConvexGeom,		// 4
		sweepConvex_MeshGeom,		// 5
		sweepConvex_HeightFieldGeom	// 6
	}
};

PX_PHYSX_COMMON_API const GeomSweepFuncs& Gu::GetGeomSweepFuncs()
{
	return gGeomSweepFuncs;
}

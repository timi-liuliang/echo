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

#include "PsIntrinsics.h"
#include "GuOverlapTests.h"

#include "CmScaling.h"
#include "PsUtilities.h"
#include "CmMidphaseTrace.h"

#include "PxSphereGeometry.h"
#include "PxBoxGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxPlaneGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxTriangleMeshGeometry.h"

#include "GuSphere.h"
#include "GuCapsule.h"
#include "GuBoxConversion.h"

#include "GuTriangleMesh.h"
#include "GuGeomUtilsInternal.h"
#include "GuConvexUtilsInternal.h"
#include "GuSPUHelpers.h"

#include "GuGJKWrapper.h"
#include "GuVecTriangle.h"
#include "GuVecSphere.h"
#include "GuVecCapsule.h"
#include "GuVecConvexHull.h"
#include "GuConvexMesh.h"

#include "GuMidphase.h"
#include "GuDistanceSegmentTriangle.h"
#include "GuDistanceSegmentTriangleSIMD.h"
#include "GuIntersectionTriangleBoxSIMD.h"
#include "GuSweepSharedTests.h"

#include "PsVecMath.h"

using namespace physx;
using namespace Cm;
using namespace Gu;
using namespace physx::shdfnd::aos;

enum { eSPHERE, eCAPSULE, eBOX }; // values for tSCB

struct LimitedResults
{
	PxU32*	mResults;
	PxU32	mNbResults;
	PxU32	mMaxResults;
	PxU32	mStartIndex;
	PxU32	mNbSkipped;
	bool	mOverflow;

	PX_FORCE_INLINE LimitedResults(PxU32* results, PxU32 maxResults, PxU32 startIndex)
		: mResults(results), mMaxResults(maxResults), mStartIndex(startIndex)
	{
		reset();
	}

	PX_FORCE_INLINE	void reset()
	{
		mNbResults	= 0;
		mNbSkipped	= 0;
		mOverflow	= false;
	}

	PX_FORCE_INLINE	bool add(PxU32 index)
	{
		if (mNbResults>=mMaxResults)
		{
			mOverflow = true;
			return false;
		}

		if (mNbSkipped>=mStartIndex)
			mResults[mNbResults++] = index;
		else
			mNbSkipped++;

		return true;
	}
};

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

// mCapsule, mCenter are in shape space
template<int tSCB, bool tScaleIsIdentity>
struct IntersectAnyVsMeshCallback : MeshHitCallback<PxRaycastHit>
{
	IntersectAnyVsMeshCallback(
		const Gu::RTreeMidphase& meshModel, const PxMat33& vertexToShapeSkew, LimitedResults* results)
		:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
			mMeshModel			(meshModel),
			mVertexToShapeSkew	(vertexToShapeSkew),
			mAnyHits			(false),
			mResults			(results)
	{
	}
	virtual	~IntersectAnyVsMeshCallback(){}

	const Gu::RTreeMidphase&	mMeshModel;
	bool						mScaleIsIdentity;
	const PxMat33&				mVertexToShapeSkew; // vertex to box without translation for boxes
	Cm::Matrix34				mVertexToBox;

	bool						mAnyHits;
	PxF32						mMinDist2;
	PxVec3						mCenter;
	Gu::Capsule					mCapsule;
	LimitedResults*				mResults;
	Vec3V						mBoxExtents, mBoxCenter;

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const PxRaycastHit& aHit, const PxVec3& av0, const PxVec3& av1, const PxVec3& av2, PxReal&, const PxU32*)
	{
		Vec3V v0, v1, v2;
		if (tSCB==eBOX)
		{
			if (tScaleIsIdentity)
			{
				v0 = V3LoadU(mVertexToShapeSkew * av0); // transform from skewed mesh vertex to box space,
				v1 = V3LoadU(mVertexToShapeSkew * av1); // this includes inverse skew, inverse mesh shape transform and inverse box basis
				v2 = V3LoadU(mVertexToShapeSkew * av2);
			} else
			{
				v0 = V3LoadU(mVertexToBox.transform(av0));
				v1 = V3LoadU(mVertexToBox.transform(av1));
				v2 = V3LoadU(mVertexToBox.transform(av2));
			}
		}
		else
		{
			v0 = V3LoadU(tScaleIsIdentity ? av0 : mVertexToShapeSkew * av0);
			v1 = V3LoadU(tScaleIsIdentity ? av1 : mVertexToShapeSkew * av1);
			v2 = V3LoadU(tScaleIsIdentity ? av2 : mVertexToShapeSkew * av2);
		}

		PxU32 hit = 0;
		if (tSCB==eCAPSULE)
		{
			// AP: verified the SIMD version is faster in benchmark OverlapMultipleCapMesh
			Vec3V dummy1, dummy2;
			Vec3V cap0 = V3LoadU(mCapsule.p0), cap1 = V3LoadU(mCapsule.p1);
			FloatV result = Gu::distanceSegmentTriangleSquared(cap0, cap1, v0, v1, v2, dummy1, dummy2);
			PxReal dist2 = FStore(result);
			hit = PxU32(dist2 <= mMinDist2);
		}
		else if (tSCB == eSPHERE)
		{
			FloatV dummy1, dummy2;
			Vec3V closestP;
			PxReal dist2 = FStore(Gu::distancePointTriangleSquared(V3LoadU(mCenter), v0, v1, v2, dummy1, dummy2, closestP));
			hit = PxU32(dist2 <= mMinDist2);
		} else
		{
			PX_ASSERT(tSCB == eBOX);
			hit = (PxU32)Gu::intersectTriangleBox(mBoxCenter, mBoxExtents, v0, v1, v2);
		}
		if (hit)
		{
			mAnyHits = true;
			if (mResults)
				mResults->add(aHit.faceIndex);
			else
				return false; // abort traversal if we are only interested in firstContact (mResults is NULL)
		}

		return true; // if we are here, either no triangles were hit or multiple results are expected => continue traversal
	}

	void operator=(const IntersectAnyVsMeshCallback<tSCB, tScaleIsIdentity>&) {}
};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif


template<int tSCB, bool idtMeshScale>
static bool intersectAnyVsMeshT(
	const Gu::Sphere* worldSphere, const Gu::Capsule* worldCapsule, const Gu::Box* worldOBB,
	const RTreeMidphase& hmd, const PxTransform& meshTransform, const PxMeshScale& meshScale,
	LimitedResults* results)
{
	PxMat33 shapeToVertexSkew, vertexToShapeSkew;
	if (!idtMeshScale && tSCB != eBOX)
	{
		vertexToShapeSkew = meshScale.toMat33();
		shapeToVertexSkew = vertexToShapeSkew.getInverse();
	}

	RTreeMidphaseData rtreeData;
	hmd.getRTreeMidphaseData(rtreeData);
	if (tSCB == eSPHERE)
	{
		IntersectAnyVsMeshCallback<tSCB, idtMeshScale> callback(hmd, vertexToShapeSkew, results);
		// transform sphere center from world to mesh shape space 
		PxVec3 center = meshTransform.transformInv(worldSphere->center);

		// callback will transform verts
		callback.mCenter = center;
		callback.mMinDist2 = worldSphere->radius*worldSphere->radius;

		PxVec3 sweepOrigin, sweepDir, sweepExtents;
		PxReal sweepLen;
		if (!idtMeshScale)
		{
			// AP: compute a swept AABB around an OBB around a skewed sphere
			// TODO: we could do better than an AABB around OBB actually because we can slice off the corners..
			const Gu::Box worldOBB_(worldSphere->center, PxVec3(worldSphere->radius), PxMat33(PxIdentity));
			Gu::Box vertexOBB;
			computeVertexSpaceOBB(vertexOBB, worldOBB_, meshTransform, meshScale);
			computeSweptAABBAroundOBB(vertexOBB, sweepOrigin, sweepExtents, sweepDir, sweepLen);
		} else
		{
			sweepOrigin = center;
			sweepDir = PxVec3(1.0f,0,0);
			sweepLen = 0.0f;
			sweepExtents = PxVec3(PxMax(worldSphere->radius, GU_MIN_AABB_EXTENT));
		}

		MPT_SET_CONTEXT("ovsm", meshTransform, meshScale);
		MeshRayCollider::collide<1,1>(sweepOrigin, sweepDir, sweepLen, true, rtreeData, callback, &sweepExtents);
		return callback.mAnyHits;
	}
	else if (tSCB == eCAPSULE)
	{
		IntersectAnyVsMeshCallback<tSCB, idtMeshScale> callback(hmd, vertexToShapeSkew, results);
		const PxF32 radius = worldCapsule->radius;
		callback.mMinDist2 = radius * radius;

		// transform world capsule to mesh shape space
		callback.mCapsule.p0		= meshTransform.transformInv(worldCapsule->p0);
		callback.mCapsule.p1		= meshTransform.transformInv(worldCapsule->p1);
		callback.mCapsule.radius	= radius;

		if (idtMeshScale)
		{
			// traverse a sweptAABB around the capsule
			PxVec3 radius3(radius);
			MPT_SET_CONTEXT("ovcm", meshTransform, meshScale);
			MeshRayCollider::collide<1,0>( // collider operates in mesh space (unless mesh transform is specified as last arg)
				callback.mCapsule.p0, callback.mCapsule.p1-callback.mCapsule.p0, 1.0f, true, rtreeData, callback, &radius3);
		}
		else
		{
			// make vertex space OBB
			Gu::Box vertexOBB;
			Gu::Box worldOBB_;
			worldOBB_.create(*worldCapsule); // AP: potential optimization (meshTransform.inverse is already in callback.mCapsule)
			computeVertexSpaceOBB(vertexOBB, worldOBB_, meshTransform, meshScale);

			MPT_SET_CONTEXT("ovcs", meshTransform, meshScale);
			MeshRayCollider::collideOBB(vertexOBB, true, rtreeData, callback);
		}
		return callback.mAnyHits;
	}
	else if (tSCB == eBOX)
	{
		Gu::Box vertexOBB; // query box in vertex space
		if (idtMeshScale)
		{
			// mesh scale is identity - just inverse transform the box without optimization
			vertexOBB = transformBoxOrthonormal(*worldOBB, meshTransform.getInverse());
			// mesh vertices will be transformed from skewed vertex space directly to box AABB space
			// box inverse rotation is baked into the vertexToShapeSkew transform
			// if meshScale is not identity, vertexOBB already effectively includes meshScale transform
			PxVec3 boxCenter;
			getInverse(vertexToShapeSkew, boxCenter, vertexOBB.rot, vertexOBB.center);
			IntersectAnyVsMeshCallback<tSCB, idtMeshScale> callback(hmd, vertexToShapeSkew, results);

			callback.mBoxCenter = V3Neg(V3LoadU(boxCenter));
			callback.mBoxExtents = V3LoadU(worldOBB->extents); // extents do not change

			MPT_SET_CONTEXT("ovbm", meshTransform, meshScale);
			MeshRayCollider::collideOBB(vertexOBB, true, rtreeData, callback);
			return callback.mAnyHits;
		} else
		{
			computeVertexSpaceOBB(vertexOBB, *worldOBB, meshTransform, meshScale);

			// mesh scale needs to be included - inverse transform and optimize the box
			const PxMat33 vertexToWorldSkew_Rot = PxMat33(meshTransform.q) * meshScale.toMat33();
			const PxVec3& vertexToWorldSkew_Trans = meshTransform.p;

			Cm::Matrix34 tmp;
			buildMatrixFromBox(tmp, *worldOBB);
			const Cm::Matrix34 inv = tmp.getInverseRT();
			const Cm::Matrix34 _vertexToWorldSkew(vertexToWorldSkew_Rot, vertexToWorldSkew_Trans);

			IntersectAnyVsMeshCallback<tSCB, idtMeshScale> callback(hmd, vertexToShapeSkew, results);
			callback.mVertexToBox = inv * _vertexToWorldSkew;
			callback.mBoxCenter = V3Zero();
			callback.mBoxExtents = V3LoadU(worldOBB->extents); // extents do not change

			MPT_SET_CONTEXT("ovbs", meshTransform, meshScale);
			MeshRayCollider::collideOBB(vertexOBB, true, rtreeData, callback);
			return callback.mAnyHits;
		}
	} else { PX_ASSERT(0); return false; }
}

template<int tSCB>
static bool intersectAnyVsMesh(
	const Gu::Sphere* worldSphere, const Gu::Capsule* worldCapsule, const Gu::Box* worldOBB,
	const RTreeMidphase& hmd, const PxTransform& meshTransform, const PxMeshScale& meshScale,
	LimitedResults* results)
{
	if (meshScale.isIdentity())
		return intersectAnyVsMeshT<tSCB, true>(worldSphere, worldCapsule, worldOBB, hmd, meshTransform, meshScale, results);
	else
		return intersectAnyVsMeshT<tSCB, false>(worldSphere, worldCapsule, worldOBB, hmd, meshTransform, meshScale, results);
}


PxU32 Gu::findOverlapSphereMesh(const Gu::Sphere& worldSphere, const Gu::RTreeMidphase& meshModel,
								const PxTransform& meshTransform, const PxMeshScale& scaling,
								PxU32* PX_RESTRICT results, PxU32 maxResults, PxU32 startIndex, bool& overflow)
{
	LimitedResults limitedResults(results, maxResults, startIndex);
	intersectAnyVsMesh<eSPHERE>(&worldSphere, NULL, NULL, meshModel, meshTransform, scaling, &limitedResults);
	overflow = limitedResults.mOverflow;
	return limitedResults.mNbResults;
}

PxU32 Gu::findOverlapCapsuleMesh(const Gu::Capsule& worldCapsule, const Gu::RTreeMidphase& meshModel,
								const PxTransform& meshTransform, const PxMeshScale& scaling,
								PxU32* PX_RESTRICT results, PxU32 maxResults, PxU32 startIndex, bool& overflow)
{
	LimitedResults limitedResults(results, maxResults, startIndex);
	intersectAnyVsMesh<eCAPSULE>(NULL, &worldCapsule, NULL, meshModel, meshTransform, scaling, &limitedResults);
	overflow = limitedResults.mOverflow;
	return limitedResults.mNbResults;
}

PxU32 Gu::findOverlapOBBMesh(const Gu::Box& worldOBB, const Gu::RTreeMidphase& meshModel,
							const PxTransform& meshTransform, const PxMeshScale& scaling,
							PxU32* PX_RESTRICT results, PxU32 maxResults, PxU32 startIndex, bool& overflow)
{
	LimitedResults limitedResults(results, maxResults, startIndex);
	RTreeMidphaseData rtreeData;
	meshModel.getRTreeMidphaseData(rtreeData);
	intersectAnyVsMesh<eBOX>(NULL, NULL, &worldOBB, meshModel, meshTransform, scaling, &limitedResults);
	overflow = limitedResults.mOverflow;
	return limitedResults.mNbResults;
}

bool checkOverlapSphere_triangleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	GU_FETCH_MESH_DATA(triGeom); // defines and initializes meshData in macro
	return intersectAnyVsMesh<eSPHERE>(&sphere, NULL, NULL, meshData->mCollisionModel, pose, triGeom.scale, NULL);
}

bool Gu::checkOverlapOBB_triangleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Box& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	GU_FETCH_MESH_DATA(triGeom); // defines and initializes meshData in macro
	return intersectAnyVsMesh<eBOX>(NULL, NULL, &box, meshData->mCollisionModel, pose, triGeom.scale, NULL);
}

bool checkOverlapCapsule_triangleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	GU_FETCH_MESH_DATA(triGeom); // defines and initializes meshData in macro
	return intersectAnyVsMesh<eCAPSULE>(NULL, &worldCapsule, NULL, meshData->mCollisionModel, pose, triGeom.scale, NULL);
}

bool GeomOverlapCallback_SphereMesh(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);	

	const Gu::Sphere worldSphere(transform0.p, sphereGeom.radius);

	GU_FETCH_MESH_DATA(meshGeom);
	return intersectAnyVsMesh<eSPHERE>(&worldSphere, NULL, NULL, meshData->mCollisionModel, transform1, meshGeom.scale, NULL);
}

bool GeomOverlapCallback_CapsuleMesh(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eCAPSULE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);

	GU_FETCH_MESH_DATA(meshGeom);

	Gu::Capsule capsule;
	Gu::getCapsule(capsule, capsuleGeom, transform0);

	return intersectAnyVsMesh<eCAPSULE>(NULL, &capsule, NULL, meshData->mCollisionModel, transform1, meshGeom.scale, NULL);
}

bool GeomOverlapCallback_BoxMesh(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eBOX);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);

	GU_FETCH_MESH_DATA(meshGeom);

	Gu::Box box;
	buildFrom(box, transform0.p, boxGeom.halfExtents, transform0.q);

	return intersectAnyVsMesh<eBOX>(NULL, NULL, &box, meshData->mCollisionModel, transform1, meshGeom.scale, NULL);
}

///////////////////////////////////////////////////////////////////////////////
struct ConvexVsMeshOverlapCallback : MeshHitCallback<PxRaycastHit>
{
	PsMatTransformV MeshToBoxV;
	Vec3V boxExtents;

	ConvexVsMeshOverlapCallback(
		const Gu::ConvexMesh& cm, const PxMeshScale& convexScale, const Cm::FastVertex2ShapeScaling& meshScale,
		const PxTransform& tr0, const PxTransform& tr1, bool identityScale, const Gu::Box& meshSpaceOBB)
		:
			MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
			mAnyHit			(false),
			mIdentityScale	(identityScale)
	{
		if (!mIdentityScale) // not done in initializer list for performance
			mMeshScale = Ps::aos::Mat33V(
				V3LoadU(meshScale.getVertex2ShapeSkew().column0),
				V3LoadU(meshScale.getVertex2ShapeSkew().column1),
				V3LoadU(meshScale.getVertex2ShapeSkew().column2) );
		using namespace Ps::aos;

		const ConvexHullData* hullData = &cm.getHull();

		const Vec3V vScale0 = V3LoadU(convexScale.scale);
		const QuatV vQuat0 = QuatVLoadU(&convexScale.rotation.x);

		mConvex =  Gu::ConvexHullV(hullData, V3Zero(), vScale0, vQuat0);
		aToB = PsMatTransformV(tr0.transformInv(tr1));
		const FloatV convexTolerance = CalculateConvexTolerance(hullData, vScale0);
		mSqTolerance = FMul(convexTolerance, convexTolerance);
		mIdentityScale = identityScale;

		{
			// Move to AABB space
			Cm::Matrix34 MeshToBox;
			computeWorldToBoxMatrix(MeshToBox, meshSpaceOBB);

			const Vec3V base0 = V3LoadU(MeshToBox.base0);
			const Vec3V base1 = V3LoadU(MeshToBox.base1);
			const Vec3V base2 = V3LoadU(MeshToBox.base2);
			const Mat33V matV(base0, base1, base2);
			const Vec3V p  = V3LoadU(MeshToBox.base3);
			MeshToBoxV = PsMatTransformV(p, matV);
			boxExtents = V3LoadU(meshSpaceOBB.extents+PxVec3(0.001f));
		}
	}
	virtual ~ConvexVsMeshOverlapCallback()	{}

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const PxRaycastHit&, const PxVec3& v0a, const PxVec3& v1a, const PxVec3& v2a, PxReal&, const PxU32*)
	{
		using namespace Ps::aos;
		Vec3V v0 = V3LoadU(v0a), v1 = V3LoadU(v1a), v2 = V3LoadU(v2a);

		// test triangle AABB in box space vs box AABB in box local space
		const Vec3V triV0 = MeshToBoxV.transform(v0); // AP: MeshToBoxV already includes mesh scale so we have to use unscaled verts here
		const Vec3V triV1 = MeshToBoxV.transform(v1);
		const Vec3V triV2 = MeshToBoxV.transform(v2);
		Vec3V triMn = V3Min(V3Min(triV0, triV1), triV2);
		Vec3V triMx = V3Max(V3Max(triV0, triV1), triV2);
		Vec3V negExtents = V3Neg(boxExtents);
		BoolV minSeparated = V3IsGrtr(triMn, boxExtents), maxSeparated = V3IsGrtr(negExtents, triMx);
		BoolV bSeparated = BAnyTrue3(BOr(minSeparated, maxSeparated));
		if (BAllEq(bSeparated, BTTTT()))
			return true; // continue traversal

		if (!mIdentityScale)
		{
			v0 = M33MulV3(mMeshScale, v0);
			v1 = M33MulV3(mMeshScale, v1);
			v2 = M33MulV3(mMeshScale, v2);
		}

		Gu::TriangleV triangle(v0, v1, v2);
		Vec3V contactA, contactB, normal;
		FloatV sqDist;
		PxGJKStatus status;
		status = Gu::GJKRelative(triangle, mConvex, aToB, contactA, contactB, normal, sqDist);
		if (status == GJK_CONTACT || FAllGrtrOrEq(mSqTolerance, sqDist))
		{
			mAnyHit = true;
			return false; // abort traversal
		}
		return true; // continue traversal
	}
	
	Gu::ConvexHullV						mConvex;
	PsMatTransformV						aToB;
	Ps::aos::FloatV						mSqTolerance;//for gjk
	Ps::aos::Mat33V						mMeshScale;
	bool								mAnyHit;
	bool								mIdentityScale;

private:
	ConvexVsMeshOverlapCallback& operator=(const ConvexVsMeshOverlapCallback&);
};

// PT: TODO: refactor bits of this with convex-vs-mesh code
bool GeomOverlapCallback_ConvexMesh(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eCONVEXMESH);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);

	GU_FETCH_CONVEX_DATA(convexGeom);
	GU_FETCH_MESH_DATA(meshGeom);

	const bool idtScaleConvex = convexGeom.scale.isIdentity();
	const bool idtScaleMesh = meshGeom.scale.isIdentity();

	Cm::FastVertex2ShapeScaling convexScaling;
	if (!idtScaleConvex)
		convexScaling.init(convexGeom.scale);

	Cm::FastVertex2ShapeScaling meshScaling;
	if (!idtScaleMesh)
		meshScaling.init(meshGeom.scale);

	const Cm::Matrix34 world0(transform0);
	const Cm::Matrix34 world1(transform1);

	PX_ASSERT(!cm->getLocalBoundsFast().isEmpty());
	PxBounds3 hullAABB = PxBounds3::transformFast(convexScaling.getVertex2ShapeSkew(), cm->getLocalBoundsFast());

	Gu::Box hullOBB;
	computeHullOBB(hullOBB, hullAABB, 0.0f, transform0, world0, world1, meshScaling, idtScaleMesh);

	Gu::RTreeMidphaseData hmd;
	meshData->mCollisionModel.getRTreeMidphaseData(hmd);

	ConvexVsMeshOverlapCallback cb(*cm, convexGeom.scale, meshScaling, transform0, transform1, idtScaleMesh, hullOBB);
	MPT_SET_CONTEXT("ovxm", transform1, meshGeom.scale);
	MeshRayCollider::collideOBB(hullOBB, true, hmd, cb, false);

	return cb.mAnyHit;
}

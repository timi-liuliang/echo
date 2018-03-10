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
#include "CmMemFetch.h"
#include "PsUtilities.h"

#include "GuIntersectionBoxBox.h"
#include "GuIntersectionTriangleBox.h"
#include "GuDistancePointSegment.h"
#include "GuDistanceSegmentBox.h"
#include "GuDistanceSegmentSegment.h"

#include "PxSphereGeometry.h"
#include "PxBoxGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxPlaneGeometry.h"
#include "PxConvexMeshGeometry.h"

#include "GuSphere.h"
#include "GuCapsule.h"
#include "GuBoxConversion.h"

#include "GuGeomUtilsInternal.h"
#include "GuConvexUtilsInternal.h"
#include "GuSPUHelpers.h"

#include "GuGJKFallBack.h"
#include "GuGJKWrapper.h"
#include "GuVecTriangle.h"
#include "GuVecSphere.h"
#include "GuVecCapsule.h"
#include "GuVecConvexHull.h"
#include "GuVecBox.h"
#include "GuConvexMesh.h"
#include "GuHillClimbing.h"

using namespace physx;
using namespace Cm;
using namespace Gu;

/**
 *	Checks a point is inside the hull.
 *	\param		p	[in] point in local space
 *	\return		true if the hull contains the point
 */
static bool convexHullContains(const ConvexHullData& data, const PxVec3& p)
{
	PxU32 Nb = data.mNbPolygons;
	const Gu::HullPolygonData* polygons = data.mPolygons;
	while(Nb--)
	{
		const PxPlane& pl = polygons->mPlane;
		if(pl.distance(p) > 0.0f)
			return false;
		polygons++;
	}
	return true;
}

//returns the maximal vertex in shape space
// PT: this function should be removed. We already have 2 different project hull functions in PxcShapeConvex & GuGJKObjectSupport, this one looks like a weird mix of both!
static PxVec3 projectHull_(	const ConvexHullData& hull,
							float& minimum, 
							float& maximum, 
							const PxVec3& localDir, // expected to be normalized
							const PxMat33& vert2ShapeSkew)
{
	PX_ASSERT(localDir.isNormalized());

	//use property that x|My == Mx|y for symmetric M to avoid having to transform vertices.
	const PxVec3 vertexSpaceDir = vert2ShapeSkew * localDir;

	const PxVec3* Verts = hull.getHullVertices();
	const PxVec3* bestVert = NULL;

	if(!hull.mBigConvexRawData)	// Brute-force, local space. Experiments show break-even point is around 32 verts.
	{
		PxU32 NbVerts = hull.mNbHullVertices;
		float min_ = PX_MAX_F32;
		float max_ = -PX_MAX_F32;
		while(NbVerts--)
		{
			const float dp = (*Verts).dot(vertexSpaceDir);
			min_ = physx::intrinsics::selectMin(min_, dp);
			if(dp > max_)	{ max_ = dp; bestVert = Verts; }

			Verts++;
		}
		minimum = min_;
		maximum = max_;

		PX_ASSERT(bestVert != NULL);

		return vert2ShapeSkew * *bestVert;
	}
	else //*/if(1)	// This version is better for objects with a lot of vertices
	{
		const PxU32 Offset = ComputeCubemapNearestOffset(vertexSpaceDir, hull.mBigConvexRawData->mSubdiv);
		PxU32 MinID = hull.mBigConvexRawData->mSamples[Offset];
		PxU32 MaxID = hull.mBigConvexRawData->getSamples2()[Offset];

		localSearch(MinID, -vertexSpaceDir, Verts, hull.mBigConvexRawData);
		localSearch(MaxID, vertexSpaceDir, Verts, hull.mBigConvexRawData);

		minimum = (Verts[MinID].dot(vertexSpaceDir));
		maximum = (Verts[MaxID].dot(vertexSpaceDir));

		PX_ASSERT(maximum >= minimum);

		return vert2ShapeSkew * Verts[MaxID];
	}
}


bool Gu::intersectPlaneBox(const PxPlane& plane, const Gu::Box& box)
{
	PxVec3 pts[8];
	box.computeBoxPoints(pts);

	for(PxU32 i=0;i<8;i++)
	{
		if(plane.distance(pts[i]) <= 0.0f)	// PT: objects are defined as closed, so we return 'true' in case of equality
			return true;
	}
	return false;
}

bool Gu::intersectPlaneCapsule(const Gu::Capsule& capsule, const PxPlane& plane)
{
	// We handle the capsule-plane collision with 2 sphere-plane collisions.
	// Seems ok so far, since plane is infinite.

	if(plane.distance(capsule.p0) <= capsule.radius)	// PT: objects are defined as closed, so we return 'true' in case of equality
		return true;

	if(plane.distance(capsule.p1) <= capsule.radius)	// PT: objects are defined as closed, so we return 'true' in case of equality
		return true;

	return false;
}

bool Gu::intersectSphereSphere(const Gu::Sphere& sphere0, const Gu::Sphere& sphere1)
{
	const PxVec3 delta = sphere1.center - sphere0.center;

	const PxReal distanceSq = delta.magnitudeSquared();

	const PxReal radSum = sphere0.radius + sphere1.radius;

	return distanceSq <= radSum * radSum;	// PT: objects are defined as closed, so we return 'true' in case of equality
}

bool Gu::intersectSphereCapsule(const Gu::Sphere& sphere, const Gu::Capsule& capsule)
{
	const PxReal r = sphere.radius + capsule.radius;
	return Gu::distancePointSegmentSquared(capsule, sphere.center, NULL) <= r*r;	// PT: objects are defined as closed, so we return 'true' in case of equality
}

bool Gu::intersectSphereBox(const Gu::Sphere& sphere, const Gu::Box& box)
{
	const PxVec3 delta = sphere.center - box.center;
	PxVec3 dRot = box.rot.transformTranspose(delta);	//transform delta into OBB body coords. (use method call!)

	//check if delta is outside ABB - and clip the vector to the ABB.
	bool outside = false;

	if(dRot.x < -box.extents.x)
	{ 
		outside = true; 
		dRot.x = -box.extents.x;
	}
	else if(dRot.x >  box.extents.x)
	{ 
		outside = true; 
		dRot.x = box.extents.x;
	}

	if(dRot.y < -box.extents.y)
	{ 
		outside = true; 
		dRot.y = -box.extents.y;
	}
	else if(dRot.y >  box.extents.y)
	{ 
		outside = true; 
		dRot.y = box.extents.y;
	}

	if(dRot.z < -box.extents.z)
	{ 
		outside = true; 
		dRot.z = -box.extents.z;
	}
	else if(dRot.z >  box.extents.z)
	{ 
		outside = true; 
		dRot.z = box.extents.z;
	}

	if(outside)	//if clipping was done, sphere center is outside of box.
	{
		const PxVec3 clippedDelta = box.rot.transform(dRot);	//get clipped delta back in world coords.

		const PxVec3 clippedVec = delta - clippedDelta;			  //what we clipped away.	
		const PxReal lenSquared = clippedVec.magnitudeSquared();
		const PxReal radius = sphere.radius;
		if(lenSquared > radius * radius)	// PT: objects are defined as closed, so we return 'true' in case of equality
			return false;	//disjoint
	}
	return true;
}

bool Gu::intersectBoxCapsule(const Gu::Box& box, const Gu::Capsule& capsule)
{
	// PT: objects are defined as closed, so we return 'true' in case of equality
	return Gu::distanceSegmentBoxSquared(capsule.p0, capsule.p1, box.center, box.extents, box.rot) <= capsule.radius*capsule.radius;
}

bool Gu::intersectSphereConvex(const Gu::Sphere& sphere, const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose,
						   PxVec3*)
{
	using namespace Ps::aos;
	const Vec3V zeroV = V3Zero();
	const Gu::ConvexHullData* hullData = &mesh.getHull();
	const FloatV sphereRadius = FLoad(sphere.radius);
	const Vec3V vScale = V3LoadU(meshScale.scale);
	const QuatV vQuat = QuatVLoadU(&meshScale.rotation.x);

	const PxTransform sphereTrans(sphere.center, PxQuat(PxIdentity));
	const PsMatTransformV aToB(convexGlobalPose.transformInv(sphereTrans)); 

	const Gu::ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);
	const Gu::CapsuleV capsule(aToB.p, sphereRadius);
	//const Gu::CapsuleV capsule(zeroV, sphereRadius);

	Vec3V contactA, contactB, normal;
	FloatV sqDist;
	PxGJKStatus status = Gu::GJKLocal(capsule, convexHull, contactA, contactB, normal, sqDist);

	const FloatV sqRadius = FMul(sphereRadius, sphereRadius);

	return status == GJK_CONTACT || FAllGrtrOrEq(sqRadius, sqDist) ;
}

bool Gu::intersectCapsuleConvex(const PxCapsuleGeometry& capsGeom, const PxTransform& capsGlobalPose,
							const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose,
							PxVec3*)
{
	using namespace Ps::aos;

	const Vec3V zeroV = V3Zero();
	const Gu::ConvexHullData* hull = &mesh.getHull();

	const FloatV capsuleHalfHeight = FLoad(capsGeom.halfHeight);
	const FloatV capsuleRadius = FLoad(capsGeom.radius);

	const Vec3V vScale = V3LoadU(meshScale.scale);
	const QuatV vQuat = QuatVLoadU(&meshScale.rotation.x);

	const PsMatTransformV aToB(convexGlobalPose.transformInv(capsGlobalPose));

	const Gu::ConvexHullV convexHull(hull, zeroV, vScale, vQuat);
	const Gu::CapsuleV capsule(aToB.p, aToB.rotate(V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);

	//const Gu::CapsuleV capsule(zeroV, V3Scale(V3UnitX(), capsuleHalfHeight), capsuleRadius);

	Vec3V contactA, contactB, normal;
	FloatV sqDist;
	PxGJKStatus  status = Gu::GJKLocal(capsule, convexHull, contactA, contactB, normal, sqDist);
	const FloatV sqRadius = FMul(capsuleRadius, capsuleRadius);
	//
	//const bool overlap = status == GJK_CONTACT || FAllGrtrOrEq(sqRadius, sqDist);

	//PX_PRINTF("CAPSULE status = %i, overlap = %i, PxVec3(%f, %f, %f)\n", status, overlap, capsGlobalPose.p.x, capsGlobalPose.p.y, capsGlobalPose.p.z);
	return status == GJK_CONTACT || FAllGrtrOrEq(sqRadius, sqDist);
}

bool Gu::intersectBoxConvex(const PxBoxGeometry& boxGeom, const PxTransform& boxGlobalPose,
						const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose,
						PxVec3*)
{
	// AP: see archived non-GJK version in //sw/physx/dev/pterdiman/graveyard/contactConvexBox.cpp
	using namespace Ps::aos;
	const Vec3V zeroV = V3Zero();
	const Gu::ConvexHullData* hull = &mesh.getHull();

	const Vec3V vScale = V3LoadU(meshScale.scale);
	const QuatV vQuat = QuatVLoadU(&meshScale.rotation.x);
	const Vec3V boxExtents = V3LoadU(boxGeom.halfExtents);
	const PsMatTransformV aToB(convexGlobalPose.transformInv(boxGlobalPose));

	const Gu::ConvexHullV convexHull(hull, zeroV, vScale, vQuat);
	const Gu::BoxV box(zeroV, boxExtents);

	/*const FloatV tolerance = FAdd(box.getMargin(), convexHull.getMargin());
	const FloatV sqTolerance = FMul(tolerance, tolerance);*/
	const FloatV convexTolerance = CalculateConvexTolerance(hull, vScale);
	const FloatV boxTolerance = CalculateBoxTolerance(boxExtents);
	const FloatV tolerance = FAdd(convexTolerance, boxTolerance);
	const FloatV sqTolerance = FMul(tolerance, tolerance);

	Vec3V contactA, contactB, normal;
	FloatV sqDist;
	PxGJKStatus status = Gu::GJKRelativeTesselation(box, convexHull, aToB, sqTolerance, contactA, contactB, normal, sqDist);

	//const bool overlap = status == GJK_CONTACT;

	//PX_PRINTF("BOX status = %i, overlap = %i, PxVec3(%f, %f, %f)\n", status, overlap, boxGlobalPose.p.x, boxGlobalPose.p.y, boxGlobalPose.p.z);

	return status == GJK_CONTACT;
}



/////////////////////////////////////////////////  checkOverlapSphere  ///////////////////////////////////////////////////////

bool Gu::checkOverlapSphere_boxGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	Gu::Box obb;
	buildFrom(obb, pose.p, boxGeom.halfExtents, pose.q);
	return intersectSphereBox(sphere, obb);
}

bool Gu::checkOverlapSphere_sphereGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	return intersectSphereSphere(sphere, Gu::Sphere(pose.p, sphereGeom.radius));
}

bool Gu::checkOverlapSphere_capsuleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	Gu::Capsule capsule;
	getCapsule(capsule, capsuleGeom, pose);
	return intersectSphereCapsule(sphere, capsule);
}

bool Gu::checkOverlapSphere_planeGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere)
{
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(geom);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom);

	const PxPlane plane = getPlane(pose);
	return plane.distance(sphere.center) - sphere.radius <= 0.0f;
}

bool Gu::checkOverlapSphere_convexGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	const PxConvexMeshGeometry& cvGeom = static_cast<const PxConvexMeshGeometry&>(geom);

	GU_FETCH_CONVEX_DATA(cvGeom);

	// PT: TODO: why do we bother doing this first test?
	//TODO: Support scaling
	if(cvGeom.scale.isIdentity())
	{
		// Test if sphere center is inside convex
		PxVec3 sphereCenter = pose.transformInv(sphere.center);
		if(convexHullContains(cm->getHull(), sphereCenter))
			return true;
	}

	return intersectSphereConvex(sphere, *cm, cvGeom.scale, pose, NULL);
}

/////////////////////////////////////////////////  checkOverlapOBB  //////////////////////////////////////////////////////////

bool Gu::checkOverlapOBB_boxGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Box& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	return Gu::intersectOBBOBB(	boxGeom.halfExtents, pose.p, PxMat33(pose.q),
								box.extents, box.center, box.rot,
								true);
}

bool Gu::checkOverlapOBB_sphereGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Box& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	return intersectSphereBox(Gu::Sphere(pose.p, sphereGeom.radius), box);
}

bool Gu::checkOverlapOBB_capsuleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Box& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	Gu::Capsule capsule;
	getCapsule(capsule, capsuleGeom, pose);

	return intersectBoxCapsule(box, capsule);
}

bool Gu::checkOverlapOBB_planeGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Box& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(geom);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom);

	const PxPlane plane = getPlane(pose);
	return intersectPlaneBox(plane, box);
}

bool Gu::checkOverlapOBB_convexGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Box& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	const PxConvexMeshGeometry& cvGeom = static_cast<const PxConvexMeshGeometry&>(geom);

	GU_FETCH_CONVEX_DATA(cvGeom);	

	// PT: TODO: why do we bother doing this first test?
	//TODO: Support scaling
	if(cvGeom.scale.isIdentity())
	{
		const PxVec3 boxCenter = pose.transformInv(box.center);
		if(convexHullContains(cm->getHull(), boxCenter))
			return true;
	}

	// PT: ### USELESS CONVERSION - PxBoxGeometry & PxTransform are not necessary here
	return intersectBoxConvex(PxBoxGeometry(box.extents), PxTransform(box.center, PxQuat(box.rot)), *cm, cvGeom.scale, pose, NULL);
}

/////////////////////////////////////////////////  checkOverlapCapsule  //////////////////////////////////////////////////////

bool Gu::checkOverlapCapsule_boxGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	Gu::Box box;
	buildFrom(box, pose.p, boxGeom.halfExtents, pose.q);

	return intersectBoxCapsule(box, worldCapsule);
}

bool Gu::checkOverlapCapsule_sphereGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	return intersectSphereCapsule(Gu::Sphere(pose.p, sphereGeom.radius), worldCapsule);
}

bool Gu::checkOverlapCapsule_capsuleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	Gu::Capsule thisWorldCapsule;
	getCapsule(thisWorldCapsule, capsuleGeom, pose);

	PxReal s,t;
	PxReal squareDist = Gu::distanceSegmentSegmentSquared(thisWorldCapsule, worldCapsule, &s, &t);
	PxReal totRad = thisWorldCapsule.radius + worldCapsule.radius;
	return squareDist <= totRad*totRad;	// PT: objects are defined as closed, so we return 'true' in case of equality
}

bool Gu::checkOverlapCapsule_planeGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule)
{
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(geom);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom);

	const PxPlane plane = getPlane(pose);
	return intersectPlaneCapsule(worldCapsule, plane);
}

bool Gu::checkOverlapCapsule_convexGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	const PxConvexMeshGeometry& cvGeom = static_cast<const PxConvexMeshGeometry&>(geom);


	GU_FETCH_CONVEX_DATA(cvGeom);

	// PT: TODO: why do we bother doing this first test?
	//TODO: Support scaling
	if(cvGeom.scale.isIdentity())
	{
		// Test if capsule center is inside convex
		PxVec3 capsuleCenter = (worldCapsule.p0 + worldCapsule.p1) * 0.5f;
		capsuleCenter = pose.transformInv(capsuleCenter);
		if(convexHullContains(cm->getHull(),capsuleCenter))
			return true;
	}
	PxCapsuleGeometry cg;
	cg.radius = worldCapsule.radius;
	PxTransform capsuleTransform = getCapsuleTransform(worldCapsule, cg.halfHeight);

	return intersectCapsuleConvex(cg, capsuleTransform, *cm, cvGeom.scale, pose, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PX_FORCE_INLINE PxVec3* getCachedAxis(Gu::TriggerCache* cache)
{
	if(cache && cache->state==Gu::TRIGGER_OVERLAP)
		return &cache->dir;
	else
		return NULL;
}

static PX_FORCE_INLINE bool updateTriggerCache(bool overlap, Gu::TriggerCache* cache)
{
	if(cache)
	{
		if(overlap)
			cache->state = Gu::TRIGGER_OVERLAP;
		else
			cache->state = Gu::TRIGGER_DISJOINT;
	}
	return overlap;
}

// Sphere-vs-shape

static bool GeomOverlapCallback_SphereSphere(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eSPHERE);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom0 = static_cast<const PxSphereGeometry&>(geom0);
	const PxSphereGeometry& sphereGeom1 = static_cast<const PxSphereGeometry&>(geom1);

	const PxVec3 delta = transform1.p - transform0.p;
	return delta.magnitudeSquared() <= Ps::sqr(sphereGeom0.radius + sphereGeom1.radius);	// PT: objects are defined as closed, so we return 'true' in case of equality
}

static bool GeomOverlapCallback_SpherePlane(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::ePLANE);
	PX_UNUSED(cache);
	PX_UNUSED(geom1);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom1);

	return Gu::getPlane(transform1).distance(transform0.p) <= sphereGeom.radius;	// PT: objects are defined as closed, so we return 'true' in case of equality
}

static bool GeomOverlapCallback_SphereCapsule(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCAPSULE);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom1);

	Gu::Segment segment;
	Gu::getCapsuleSegment(transform1, capsuleGeom, segment);

	const PxReal totRad = sphereGeom.radius + capsuleGeom.radius;

	return Gu::distancePointSegmentSquared(segment, transform0.p, NULL) <= totRad*totRad;	// PT: objects are defined as closed, so we return 'true' in case of equality
}

static bool GeomOverlapCallback_SphereBox(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eBOX);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom1);

	Gu::Box obb;
	buildFrom(obb, transform1.p, boxGeom.halfExtents, transform1.q);

	return Gu::intersectSphereBox(
		Gu::Sphere(transform0.p, sphereGeom.radius),
		obb);
}

static bool GeomOverlapCallback_SphereConvex(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	//pxPrintf("in sphereConvex\n");
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCONVEXMESH);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom1);

	GU_FETCH_CONVEX_DATA(convexGeom);

	PxVec3 cachedSepAxis;
	PxVec3* tmp = getCachedAxis(cache);
	if(tmp)
		cachedSepAxis = *tmp;
	else
		cachedSepAxis = PxVec3(0,0,1.f);

	const bool overlap = Gu::intersectSphereConvex(Gu::Sphere(transform0.p, sphereGeom.radius), 
		*cm,
		convexGeom.scale, transform1,
		&cachedSepAxis);

	if(cache && overlap)
		cache->dir = cachedSepAxis;

	return updateTriggerCache(overlap, cache);
}

// Plane-vs-shape

static bool GeomOverlapCallback_PlanePlane(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::ePLANE);
	PX_ASSERT(geom1.getType()==PxGeometryType::ePLANE);
	PX_ALWAYS_ASSERT_MESSAGE("NOT SUPPORTED");
	PX_UNUSED(cache);
	PX_UNUSED(transform0);
	PX_UNUSED(transform1);	
	PX_UNUSED(geom0);
	PX_UNUSED(geom1);
	return false;
}

static bool GeomOverlapCallback_PlaneCapsule(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::ePLANE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCAPSULE);
	PX_UNUSED(cache);
	PX_UNUSED(geom0);

//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom0);
	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom1);

	Gu::Capsule capsule;
	Gu::getCapsule(capsule, capsuleGeom, transform1);

	return intersectPlaneCapsule(capsule, Gu::getPlane(transform0));
}

static bool GeomOverlapCallback_PlaneBox(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::ePLANE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eBOX);
	PX_UNUSED(cache);
	PX_UNUSED(geom0);

//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom0);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom1);

	// I currently use the same code as for contact generation but maybe we could do something faster (in theory testing
	// only 2 pts is enough).

	const Cm::Matrix34 absPose(transform1);
	const PxPlane worldPlane = Gu::getPlane(transform0);

	for(int vx=-1; vx<=1; vx+=2)
		for(int vy=-1; vy<=1; vy+=2)
			for(int vz=-1; vz<=1; vz+=2)
			{
				const PxVec3 v = absPose.transform(PxVec3(PxReal(vx),PxReal(vy),PxReal(vz)).multiply(boxGeom.halfExtents));

				if(worldPlane.distance(v) <= 0.0f)	// PT: objects are defined as closed, so we return 'true' in case of equality
					return true;
			}
	return false;
}

static bool GeomOverlapCallback_PlaneConvex(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	//pxPrintf("in planeConvex\n");
	PX_ASSERT(geom0.getType()==PxGeometryType::ePLANE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCONVEXMESH);
	PX_UNUSED(cache);
	PX_UNUSED(geom0);

//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom0);
	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom1);

	GU_FETCH_CONVEX_DATA(convexGeom);
	
	//find plane normal in shape space of convex:
	const PxTransform plane2convex = transform1.getInverse().transform(transform0);

	const PxPlane shapeSpacePlane = Gu::getPlane(plane2convex);

	PxReal minimum, maximum;
	projectHull_(cm->getHull(), minimum, maximum, shapeSpacePlane.n, convexGeom.scale.toMat33());

	return (minimum <= -shapeSpacePlane.d);
}

static bool GeomOverlapCallback_PlaneMesh(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::ePLANE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_ALWAYS_ASSERT_MESSAGE("NOT SUPPORTED");
	PX_UNUSED(cache);
	PX_UNUSED(transform0);
	PX_UNUSED(transform1);
	PX_UNUSED(geom0);
	PX_UNUSED(geom1);

	return false;
}


// Capsule-vs-shape

static bool GeomOverlapCallback_CapsuleCapsule(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eCAPSULE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCAPSULE);
	PX_UNUSED(cache);

	const PxCapsuleGeometry& capsuleGeom0 = static_cast<const PxCapsuleGeometry&>(geom0);
	const PxCapsuleGeometry& capsuleGeom1 = static_cast<const PxCapsuleGeometry&>(geom1);

	Gu::Segment segment1;
	Gu::getCapsuleSegment(transform0, capsuleGeom0, segment1);

	Gu::Segment segment2;
	Gu::getCapsuleSegment(transform1, capsuleGeom1, segment2);

	const PxReal squareDist = Gu::distanceSegmentSegmentSquared(segment1, segment2);
	return squareDist <= Ps::sqr(capsuleGeom0.radius + capsuleGeom1.radius);	// PT: objects are defined as closed, so we return 'true' in case of equality
}

static bool GeomOverlapCallback_CapsuleBox(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eCAPSULE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eBOX);
	PX_UNUSED(cache);

	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom0);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom1);

	Gu::Segment segment;
	Gu::getCapsuleSegment(transform0, capsuleGeom, segment);

	// PT: it is more efficient to convert to a matrix only once here, rather than 3 times in the code below...
	Gu::Box obb;
	buildFrom(obb, transform1.p, boxGeom.halfExtents, transform1.q);

	// Collision detection
	//  ### is this even useful here ?
	if(Gu::intersectSphereBox(Gu::Sphere(segment.p0, capsuleGeom.radius), obb))	return true;
	if(Gu::intersectSphereBox(Gu::Sphere(segment.p1, capsuleGeom.radius), obb))	return true;
	// 
	// PT: objects are defined as closed, so we return 'true' in case of equality
	return Gu::distanceSegmentBoxSquared(segment.p0, segment.p1, transform1.p, boxGeom.halfExtents, obb.rot) <= Ps::sqr(capsuleGeom.radius);
}

static bool GeomOverlapCallback_CapsuleConvex(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	//pxPrintf("in capsuleConvex\n");
	PX_ASSERT(geom0.getType()==PxGeometryType::eCAPSULE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCONVEXMESH);

	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom0);
	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom1);

	//pxPrintf("before fetch cm=%x\n", PxU32(convexGeom.convexMesh));
	GU_FETCH_CONVEX_DATA(convexGeom);
	//pxPrintf("cm=%x, polys=%x\n", PxU32(cm), PxU32(cm->getHull().mPolygons));
	//pxPrintf("convex vert count=%d\n", cm->getNbVerts());
	//pxPrintf("convex scale=%.5f %.5f %.5f\n", convexGeom.scale.scale.x, convexGeom.scale.scale.y, convexGeom.scale.scale.z);
	//pxPrintf("cap radius, halfHeight=%.5f %.5f\n", capsuleGeom.radius, capsuleGeom.halfHeight);
	//pxPrintf("xform0.p=%.5f %.5f %.5f\n", transform0.p.x, transform0.p.y, transform0.p.z);
	//pxPrintf("xform0.q=%.5f %.5f %.5f %.5f\n", transform0.q.x, transform0.q.y, transform0.q.z, transform0.q.w);
	//pxPrintf("xform1.p=%.5f %.5f %.5f\n", transform1.p.x, transform1.p.y, transform1.p.z);
	//pxPrintf("xform1.q=%.5f %.5f %.5f %.5f\n", transform1.q.x, transform1.q.y, transform1.q.z, transform1.q.w);
	//for (PxU32 iv = 0; iv < cm->getNbVerts(); iv++)
	//	pxPrintf("v[%d] = %.5f %.5f %.5f; ", iv, cm->getVerts()[iv].x, cm->getVerts()[iv].y, cm->getVerts()[iv].z);
	//pxPrintf("\n");

	PxVec3 cachedSepAxis;
	PxVec3* tmp = getCachedAxis(cache);
	if(tmp)
		cachedSepAxis = *tmp;
	else
		cachedSepAxis = PxVec3(0,0,1.0f);

	const bool overlap = Gu::intersectCapsuleConvex(capsuleGeom, transform0, *cm, convexGeom.scale, transform1, &cachedSepAxis);

	if(cache && overlap)
		cache->dir = cachedSepAxis;

	return updateTriggerCache(overlap, cache);
}

// Box-vs-shape

static bool GeomOverlapCallback_BoxBox(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eBOX);
	PX_ASSERT(geom1.getType()==PxGeometryType::eBOX);
	PX_UNUSED(cache);

	const PxBoxGeometry& boxGeom0 = static_cast<const PxBoxGeometry&>(geom0);
	const PxBoxGeometry& boxGeom1 = static_cast<const PxBoxGeometry&>(geom1);

	return Gu::intersectOBBOBB(	boxGeom0.halfExtents, transform0.p, PxMat33(transform0.q), 
								boxGeom1.halfExtents, transform1.p, PxMat33(transform1.q), true);
}

static bool GeomOverlapCallback_BoxConvex(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	//pxPrintf("in boxConvex\n");
	PX_ASSERT(geom0.getType()==PxGeometryType::eBOX);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCONVEXMESH);

	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom0);
	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom1);

	GU_FETCH_CONVEX_DATA(convexGeom);
	//pxPrintf("convex vert count=%d\n", cm->getNbVerts());

	PxVec3 cachedSepAxis;
	PxVec3* tmp = getCachedAxis(cache);
	if(tmp)
		cachedSepAxis = *tmp;
	else
		cachedSepAxis = PxVec3(0,0,1.f);

	const bool overlap = Gu::intersectBoxConvex(boxGeom, transform0, *cm, convexGeom.scale, transform1, &cachedSepAxis);

	if(cache && overlap)
		cache->dir = cachedSepAxis;

	return updateTriggerCache(overlap, cache);
}

// Convex-vs-shape
static bool GeomOverlapCallback_ConvexConvex(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	//pxPrintf("in convexConvex\n");
	using namespace Ps::aos;
	PX_ASSERT(geom0.getType()==PxGeometryType::eCONVEXMESH);
	PX_ASSERT(geom1.getType()==PxGeometryType::eCONVEXMESH);

	const Vec3V zeroV = V3Zero();
	const PxConvexMeshGeometry& convexGeom0 = static_cast<const PxConvexMeshGeometry&>(geom0);
	const PxConvexMeshGeometry& convexGeom1 = static_cast<const PxConvexMeshGeometry&>(geom1);
	const Gu::ConvexMesh *cm0, *cm1;
    GU_FETCH_CONVEX_DATA_NAMED(convexGeom0, cmtemp);
    cm0 = cmtemp;
	bool overlap;
	{ // AP: this scope is important for the SPUPtrRestoreOnDestruct in GU_FETCH_CONVEX_DATA to work properly
		GU_FETCH_CONVEX_DATA(convexGeom1);
		cm1 = cm;

		const ConvexHullData* hullData0 = &cm0->getHull();
		const ConvexHullData* hullData1 = &cm1->getHull();

		const Vec3V vScale0 = Vec3V_From_Vec4V(V4LoadU(&convexGeom0.scale.scale.x));
		const QuatV vQuat0 = QuatVLoadU(&convexGeom0.scale.rotation.x);
		const Vec3V vScale1 = Vec3V_From_Vec4V(V4LoadU(&convexGeom1.scale.scale.x));
		const QuatV vQuat1 = QuatVLoadU(&convexGeom1.scale.rotation.x);

		const QuatV q0 = QuatVLoadU(&transform0.q.x);
		const Vec3V p0 = V3LoadU(&transform0.p.x);

		const QuatV q1 = QuatVLoadU(&transform1.q.x);
		const Vec3V p1 = V3LoadU(&transform1.p.x);

		const PsTransformV transf0(p0, q0);
		const PsTransformV transf1(p1, q1);

		const PsMatTransformV aToB(transf1.transformInv(transf0));

		const Gu::ConvexHullV convexHull0(hullData0, zeroV, vScale0, vQuat0);
		const Gu::ConvexHullV convexHull1(hullData1, zeroV, vScale1, vQuat1);

		const FloatV convexTolerance0 = CalculateConvexTolerance(hullData0, vScale0);
		const FloatV convexTolerance1 = CalculateConvexTolerance(hullData1, vScale1);
		const FloatV tolerance = FAdd(convexTolerance0, convexTolerance1);
		const FloatV sqTolerance = FMul(tolerance, tolerance);
		
		Vec3V contactA, contactB, normal;
		FloatV sqDist;
		PxGJKStatus status = Gu::GJKRelativeTesselation(convexHull0, convexHull1, aToB, sqTolerance, contactA, contactB, normal, sqDist);
		//PxGJKStatus status = Gu::GJKRelative(convexHull0, convexHull1, aToB, contactA, contactB, normal, sqDist);
		overlap = (status == GJK_CONTACT);


		/*if(convexHull0.numVerts != 53)
			PX_PRINTF("CONVEX HULL status = %i, overlap = %i, PxVec3(%f, %f, %f)\n", status, overlap, transform0.p.x, transform0.p.y, transform0.p.z);
		else
			PX_PRINTF("CONVEX HULL status = %i, overlap = %i, PxVec3(%f, %f, %f)\n", status, overlap, transform1.p.x, transform1.p.y, transform1.p.z);*/
	}

	return updateTriggerCache(overlap, cache);
}


// Mesh-vs-shape
static bool GeomOverlapCallback_MeshMesh(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_ALWAYS_ASSERT_MESSAGE("NOT SUPPORTED");
	PX_UNUSED(cache);	
	PX_UNUSED(transform0);	
	PX_UNUSED(transform1);	
	PX_UNUSED(geom0);
	PX_UNUSED(geom1);
	return false;
}

static bool GeomOverlapCallback_MeshHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_ASSERT(geom1.getType()==PxGeometryType::eHEIGHTFIELD);
	PX_ALWAYS_ASSERT_MESSAGE("NOT SUPPORTED");
	PX_UNUSED(cache);	
	PX_UNUSED(transform0);	
	PX_UNUSED(transform1);	
	PX_UNUSED(geom0);
	PX_UNUSED(geom1);
	return false;
}

// Heightfield-vs-shape

static bool GeomOverlapCallback_HeightfieldHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eHEIGHTFIELD);
	PX_ASSERT(geom1.getType()==PxGeometryType::eHEIGHTFIELD);
	PX_ALWAYS_ASSERT_MESSAGE("NOT SUPPORTED");
	PX_UNUSED(cache);	
	PX_UNUSED(transform0);	
	PX_UNUSED(transform1);	
	PX_UNUSED(geom0);
	PX_UNUSED(geom1);

	return false;
}

#if PX_IS_SPU
#define DYNAMIC_OVERLAP_REGISTRATION(x) x
#else
#define DYNAMIC_OVERLAP_REGISTRATION(x) GeomOverlapCallback_HeightfieldUnregistered

static bool GeomOverlapCallback_HeightfieldUnregistered(GEOM_OVERLAP_CALLBACK_PARAMS)
{
	PX_UNUSED(cache);
	PX_UNUSED(geom0);
	PX_UNUSED(geom1);
	PX_UNUSED(transform0);
	PX_UNUSED(transform1);
	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Height Field Overlap test called with height fields unregistered ");
	return false;
}
#endif

bool GeomOverlapCallback_SphereMesh(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_CapsuleMesh(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_BoxMesh(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_ConvexMesh(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_SphereHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_PlaneHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_CapsuleHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_BoxHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS);
bool GeomOverlapCallback_ConvexHeightfield(GEOM_OVERLAP_CALLBACK_PARAMS);

GeomOverlapTableEntry7 gGeomOverlapMethodTable[] = 
{
	//PxGeometryType::eSPHERE
	{
		GeomOverlapCallback_SphereSphere,		//PxGeometryType::eSPHERE
		GeomOverlapCallback_SpherePlane,		//PxGeometryType::ePLANE
		GeomOverlapCallback_SphereCapsule,		//PxGeometryType::eCAPSULE
		GeomOverlapCallback_SphereBox,			//PxGeometryType::eBOX
		GeomOverlapCallback_SphereConvex,		//PxGeometryType::eCONVEXMESH
		GeomOverlapCallback_SphereMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_SphereHeightfield),	//PxGeometryType::eHEIGHTFIELD
		
	},

	//PxGeometryType::ePLANE
	{
		0,										//PxGeometryType::eSPHERE
		GeomOverlapCallback_PlanePlane,			//PxGeometryType::ePLANE
		GeomOverlapCallback_PlaneCapsule,		//PxGeometryType::eCAPSULE
		GeomOverlapCallback_PlaneBox,			//PxGeometryType::eBOX
		GeomOverlapCallback_PlaneConvex,		//PxGeometryType::eCONVEXMESH
		GeomOverlapCallback_PlaneMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_PlaneHeightfield),	//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eCAPSULE
	{
		0,										//PxGeometryType::eSPHERE
		0,										//PxGeometryType::ePLANE
		GeomOverlapCallback_CapsuleCapsule,		//PxGeometryType::eCAPSULE
		GeomOverlapCallback_CapsuleBox,			//PxGeometryType::eBOX
		GeomOverlapCallback_CapsuleConvex,		//PxGeometryType::eCONVEXMESH
		GeomOverlapCallback_CapsuleMesh,		//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_CapsuleHeightfield),	//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eBOX
	{
		0,										//PxGeometryType::eSPHERE
		0,										//PxGeometryType::ePLANE
		0,										//PxGeometryType::eCAPSULE
		GeomOverlapCallback_BoxBox,				//PxGeometryType::eBOX
		GeomOverlapCallback_BoxConvex,			//PxGeometryType::eCONVEXMESH
		GeomOverlapCallback_BoxMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_BoxHeightfield),		//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eCONVEXMESH
	{
		0,										//PxGeometryType::eSPHERE
		0,										//PxGeometryType::ePLANE
		0,										//PxGeometryType::eCAPSULE
		0,										//PxGeometryType::eBOX
		GeomOverlapCallback_ConvexConvex,		//PxGeometryType::eCONVEXMESH
		GeomOverlapCallback_ConvexMesh,			//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_ConvexHeightfield),	//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eTRIANGLEMESH
	{
		0,										//PxGeometryType::eSPHERE
		0,										//PxGeometryType::ePLANE
		0,										//PxGeometryType::eCAPSULE
		0,										//PxGeometryType::eBOX
		0,										//PxGeometryType::eCONVEXMESH
		GeomOverlapCallback_MeshMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_MeshHeightfield),	//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eHEIGHTFIELD
	{
		0,											//PxGeometryType::eSPHERE
		0,											//PxGeometryType::ePLANE
		0,											//PxGeometryType::eCAPSULE
		0,											//PxGeometryType::eBOX
		0,											//PxGeometryType::eCONVEXMESH
		0,											//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_OVERLAP_REGISTRATION(GeomOverlapCallback_HeightfieldHeightfield),	//PxGeometryType::eHEIGHTFIELD
	},
};

GeomOverlapTableEntry7* Gu::GetGeomOverlapMethodTable()
{
	return gGeomOverlapMethodTable;
}

void Gu::registerHeightFields()
{
	gGeomOverlapMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_SphereHeightfield;
	gGeomOverlapMethodTable[PxGeometryType::ePLANE][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_PlaneHeightfield;
	gGeomOverlapMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_CapsuleHeightfield;
	gGeomOverlapMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_BoxHeightfield;
	gGeomOverlapMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_ConvexHeightfield;
	gGeomOverlapMethodTable[PxGeometryType::eTRIANGLEMESH][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_MeshHeightfield;
	gGeomOverlapMethodTable[PxGeometryType::eHEIGHTFIELD][PxGeometryType::eHEIGHTFIELD] = GeomOverlapCallback_HeightfieldHeightfield;
}


extern bool checkOverlapSphere_triangleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Sphere& sphere);
extern bool checkOverlapCapsule_triangleGeom(const PxGeometry& geom, const PxTransform& pose, const Gu::Capsule& worldCapsule);

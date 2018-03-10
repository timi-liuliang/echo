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
#include "PsAlloca.h"
#include "GuRaycastTests.h"

#include "GuGeomUtilsInternal.h"	// For getSegment
#include "PxBoxGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxSphereGeometry.h"
#include "PxPlaneGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxTriangleMeshGeometry.h"
#include "GuIntersectionRayBox.h"
#include "GuIntersectionRayCapsule.h"
#include "GuIntersectionRaySphere.h"
#include "GuIntersectionRayPlane.h"
#include "GuHeightFieldUtil.h"
#include "GuDistancePointSegment.h"
#include "GuBoxConversion.h"

#include "GuCapsule.h"
#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "CmScaling.h"

#include "GuMidphase.h"

#include "PxQueryReport.h"
#include "GuSPUHelpers.h"

using namespace physx;
using namespace Gu;

#ifdef __SPU__
extern CellHeightfieldTileCache g_sampleCache;
#endif

////////////////////////////////////////////////// raycasts //////////////////////////////////////////////////////////////////
PxU32 raycast_box(GU_RAY_FUNC_PARAMS)
{
	PX_UNUSED(userData);
	PX_UNUSED(hitCB);
	PX_UNUSED(anyHit);
	PX_UNUSED(maxHits);
	PX_ASSERT(geom.getType() == PxGeometryType::eBOX);
	PX_ASSERT(maxHits && hits);
	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

	const PxTransform& absPose = pose;

	PxVec3 localOrigin = rayOrigin - absPose.p;
	localOrigin = absPose.q.rotateInv(localOrigin);

	PxVec3 localDir = absPose.q.rotateInv(rayDir);

	PxVec3 localImpact;
	PxReal t;
	PxVec3 dimensions = boxGeom.halfExtents;
	PxU32 rval = Gu::rayAABBIntersect2(-dimensions, dimensions, localOrigin, localDir, localImpact, t);
	if(!rval)
		return 0;

	if(t>maxDist)
		return 0;

	PxHitFlags outFlags = PxHitFlag::eDISTANCE;
	hits->distance	= t; //worldRay.orig.distance(hit.worldImpact);	//should be the same, assuming ray dir was normalized!!
	hits->faceIndex	= 0xffffffff;
	hits->u			= 0.0f;
	hits->v			= 0.0f;

	if((hintFlags & PxHitFlag::ePOSITION))
	{
		outFlags |= PxHitFlag::ePOSITION;
		if(t!=0.0f)
			hits->position = absPose.transform(localImpact);
		else
			hits->position = rayOrigin;
	}

	// Compute additional information if needed
	if(hintFlags & PxHitFlag::eNORMAL)
	{
		outFlags |= PxHitFlag::eNORMAL;

		//Because rayAABBIntersect2 set t = 0 if start point inside shape
		if(t == 0)
		{
			hits->normal = -rayDir;
		}
		else
		{
			//local space normal is:
			rval--;
			PxVec3 n(0,0,0);
			n[rval] = PxReal((localImpact[rval] > 0) ? 1 : -1);
			hits->normal = absPose.q.rotate(n);
		}
	}
	else
	{
		hits->normal = PxVec3(0.0f);
	}
	hits->flags	= outFlags;
	return 1;
}

PxU32 raycast_sphere(GU_RAY_FUNC_PARAMS)
{
	PX_UNUSED(userData);
	PX_UNUSED(hitCB);
	PX_UNUSED(anyHit);
	PX_UNUSED(maxHits);
	PX_ASSERT(geom.getType() == PxGeometryType::eSPHERE);
	PX_ASSERT(maxHits && hits);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

	if(!Gu::intersectRaySphere(rayOrigin, rayDir, maxDist, pose.p, sphereGeom.radius, hits->distance, &hits->position))
		return 0;

	/*	// PT: should be useless now
	hit.distance	= worldRay.orig.distance(hit.worldImpact);
	if(hit.distance>maxDist)
	return false;
	*/
	// PT: we can't avoid computing the position here since it's needed to compute the normal anyway
	PxHitFlags outFlags = PxHitFlag::eDISTANCE|PxHitFlag::ePOSITION;
	hits->faceIndex	= 0xffffffff;
	hits->u			= 0.0f;
	hits->v			= 0.0f;

	// Compute additional information if needed
	if(hintFlags & PxHitFlag::eNORMAL)
	{
		// User requested impact normal
		//Because intersectRaySphere set distance = 0 if start point inside shape
		if(hits->distance == 0.0f)
		{
			hits->normal = -rayDir;
		}
		else
		{
			hits->normal = hits->position - pose.p;
			hits->normal.normalize();
		}
		outFlags |= PxHitFlag::eNORMAL;
	}
	else
	{
		hits->normal = PxVec3(0.0f);
	}
	hits->flags = outFlags;

	return 1;
}

PxU32 raycast_capsule(GU_RAY_FUNC_PARAMS)
{
	PX_UNUSED(userData);
	PX_UNUSED(hitCB);
	PX_UNUSED(anyHit);
	PX_UNUSED(maxHits);
	PX_ASSERT(geom.getType() == PxGeometryType::eCAPSULE);
	PX_ASSERT(maxHits && hits);

	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

	// TODO: PT: could we simplify this ?
	Gu::Capsule capsule;
	getCapsuleSegment(pose, capsuleGeom, capsule);
	capsule.radius = capsuleGeom.radius;

	PxReal s[2];
	PxU32 numISec = Gu::intersectRayCapsule(rayOrigin, rayDir, capsule, s);
	if(!numISec)
		return 0;

	PxReal t;
	if(numISec == 1)
		t = s[0];
	else
	{
		// PT: attempt at fixing TTP 3690. I'm in Sweden and don't have my test cases for the ray-capsule
		// code here, so this fix is a bit uncertain.
		const PxReal epsilon = 1e-6f;
		if(s[0]<-epsilon && s[1]<-epsilon)
			return 0;

		t = (s[0] < s[1]) ? s[0]:s[1];

		if(t<0.0f)
		{
			t=0.0f;
		}
	}

	//	if(t<0.0f || t>maxDist)
	if(t>maxDist)	// PT: this was commented out. Who did it? Why?
		return 0;

	// PT: we can't avoid computing the position here since it's needed to compute the normal anyway
	PxHitFlags outFlags = PxHitFlag::eDISTANCE|PxHitFlag::ePOSITION;
	hits->position	= rayOrigin + rayDir*t;	// PT: will be rayOrigin for t=0.0f (i.e. what the spec wants)
	hits->distance	= t;
	hits->faceIndex	= 0xffffffff;
	hits->u			= 0.0f;
	hits->v			= 0.0f;

	// Compute additional information if needed
	if(hintFlags & PxHitFlag::eNORMAL)
	{
		outFlags |= PxHitFlag::eNORMAL;

		if(t==0.0f)
		{
			hits->normal = -rayDir;
		}
		else
		{
			PxReal capsuleT;
			Gu::distancePointSegmentSquared(capsule, hits->position, &capsuleT);
			capsule.computePoint(hits->normal, capsuleT);
			hits->normal = hits->position - hits->normal;	 //this should never be zero. It should have a magnitude of the capsule radius.
			hits->normal.normalize();
		}
	}
	else
	{
		hits->normal = PxVec3(0.0f);
	}
	hits->flags = outFlags;

	return 1;
}

PxU32 raycast_plane(GU_RAY_FUNC_PARAMS)
{
	PX_UNUSED(hintFlags);
	PX_UNUSED(userData);
	PX_UNUSED(hitCB);
	PX_UNUSED(anyHit);
	PX_UNUSED(maxHits);
	PX_ASSERT(geom.getType() == PxGeometryType::ePLANE);
	PX_ASSERT(maxHits && hits);
	PX_UNUSED(geom);
//	const PxPlaneGeometry& planeGeom = static_cast<const PxPlaneGeometry&>(geom);

	// Perform backface culling so that we can pick objects beyond planes
	PxPlane plane = Gu::getPlane(pose);
	if(rayDir.dot(plane.n)>=0.0f)
		return false;

	PxReal distanceAlongLine;
	if(!Gu::intersectRayPlane(rayOrigin, rayDir, plane, distanceAlongLine, &hits->position))
		return 0;

	/*
	PxReal test = worldRay.orig.distance(hit.worldImpact);

	PxReal dd;
	PxVec3 pp;
	PxSegmentPlaneIntersect(worldRay.orig, worldRay.orig+worldRay.dir*1000.0f, plane, dd, pp);
	*/

	if(distanceAlongLine<0.0f)
		return 0;

	if(distanceAlongLine>maxDist)
		return 0;

	hits->distance	= distanceAlongLine;
	hits->faceIndex	= 0xffffffff;
	hits->u			= 0.0f;
	hits->v			= 0.0f;
	hits->flags		= PxHitFlag::eDISTANCE|PxHitFlag::ePOSITION|PxHitFlag::eNORMAL;
	hits->normal	= plane.n;
	return 1;
}

PxU32 raycast_convexMesh(GU_RAY_FUNC_PARAMS)
{ 
	PX_UNUSED(userData);
	PX_UNUSED(hitCB);
	PX_UNUSED(anyHit);
	PX_UNUSED(maxHits);
	PX_ASSERT(geom.getType() == PxGeometryType::eCONVEXMESH);
	PX_ASSERT(maxHits && hits);
	PX_ASSERT(PxAbs(rayDir.magnitudeSquared()-1)<1e-4);

	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

	//PxMat34Legacy absPose(pose);

	PX_ALIGN_PREFIX(16)  PxU8 convexMeshBuffer[sizeof(Gu::ConvexMesh)+32] PX_ALIGN_SUFFIX(16);
	Gu::ConvexMesh* convexMesh = Cm::memFetchAsync<Gu::ConvexMesh>(convexMeshBuffer, (uintptr_t)(convexGeom.convexMesh), sizeof(Gu::ConvexHullData),1);

	PxRaycastHit& hit = *hits;
	
	//scaling: transform the ray to vertex space
	Cm::Matrix34 world2vertexSkew = convexGeom.scale.getInverse() * pose.getInverse();	

	Cm::memFetchWait(1); // convexMesh	

	//Gu::ConvexMesh* cmesh = static_cast<Gu::ConvexMesh*>(convexGeom.convexMesh);
	PxU32 nPolys = convexMesh->getNbPolygonsFast();
	const Gu::HullPolygonData* PX_RESTRICT polysEA = convexMesh->getPolygons();

#ifdef __SPU__
	const PxU32 polysSize = sizeof(Gu::HullPolygonData)*nPolys;


	PX_COMPILE_TIME_ASSERT(&((Gu::ConvexMesh*)NULL)->getHull()==NULL);

	 //The number of polygons is limited to 256.
	PX_ALIGN_PREFIX(16)  PxU8 hullBuffer[sizeof(Gu::HullPolygonData)*256+32] PX_ALIGN_SUFFIX(16);
	Gu::HullPolygonData* polys = Cm::memFetchAsync<Gu::HullPolygonData>(hullBuffer, (uintptr_t)(polysEA), polysSize, 1);
#else
	const Gu::HullPolygonData* polys = polysEA;
#endif

	PxVec3 vrayOrig = world2vertexSkew.transform( rayOrigin );
	PxVec3 vrayDir = world2vertexSkew.rotate( rayDir );	

	Cm::memFetchWait(1);// polys

	/*
	Purely convex planes based algorithm
	Iterate all planes of convex, with following rules:
	* determine of ray origin is inside them all or not.  
	* planes parallel to ray direction are immediate early out if we're on the outside side (plane normal is sep axis)
	* else 
		- for all planes the ray direction "enters" from the front side, track the one furthest along the ray direction (A)
		- for all planes the ray direction "exits" from the back side, track the one furthest along the negative ray direction (B)
	if the ray origin is outside the convex and if along the ray, A comes before B, the directed line stabs the convex at A
	*/
	bool originInsideAllPlanes = true;
	PxReal latestEntry = -FLT_MAX;
	PxReal earlyestExit = FLT_MAX;
	const PxPlane* bestVertSpacePlane = NULL;
	PxU32 bestPolygonIndex = 0;
	PxU32 currentIndex = 0;

	while (nPolys--)
	{
		const Gu::HullPolygonData& poly = *(polys++);
		const PxPlane& vertSpacePlane = poly.mPlane;

		const PxReal distToPlane = vertSpacePlane.distance(vrayOrig);
		const PxReal dn = vertSpacePlane.n.dot(vrayDir);
		const PxReal distAlongRay = -distToPlane/dn;

		if(distToPlane > 0)	
			originInsideAllPlanes = false;	//origin not behind plane == ray starts outside the convex.

		if (dn > 1E-7f)	//the ray direction "exits" from the back side
		{
			earlyestExit = physx::intrinsics::selectMin(earlyestExit, distAlongRay);
		}
		else if (dn < -1E-7f)	//the ray direction "enters" from the front side
		{
			if (distAlongRay > latestEntry)
			{
				latestEntry = distAlongRay;
				bestVertSpacePlane = &vertSpacePlane;
				bestPolygonIndex = currentIndex;
			}
		}
		else
		{
			//plane normal and ray dir are orthogonal
			if(distToPlane > 0)	
				return 0;	//a plane is parallel with ray -- and we're outside the ray -- we definitely miss the entire convex!
		}

		currentIndex++;
	}

	if (originInsideAllPlanes)	//ray starts inside convex
	{
		hit.distance	= 0.0f;
		hit.faceIndex	= 0xffffffff;
		hit.u			= 0.0f;
		hit.v			= 0.0f;
		hit.position	= rayOrigin;
		hit.normal		= -rayDir;
		hit.flags		= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL|PxHitFlag::ePOSITION;
		return 1;
	}

	// AP: changed to latestEntry < maxDist-1e-5f so that we have a conservatively negative result near end of ray
	if (latestEntry < earlyestExit && latestEntry > 0.0f && latestEntry < maxDist-1e-5f)
	{
		PxHitFlags outFlags = PxHitFlag::eDISTANCE;
		if(hintFlags & PxHitFlag::ePOSITION)
		{
			outFlags |= PxHitFlag::ePOSITION;
			const PxVec3& pointOnPlane = vrayOrig + latestEntry * vrayDir;
			hit.position = pose.transform(convexGeom.scale.toMat33() * pointOnPlane);
		}
		hit.distance	= latestEntry;
		hit.faceIndex	= bestPolygonIndex;	// we do now return the polygon index
		hit.u			= 0.0f;
		hit.v			= 0.0f;
		hit.normal		= PxVec3(0.0f);

		// Compute additional information if needed
		if(hintFlags & PxHitFlag::eNORMAL)
		{
			outFlags |= PxHitFlag::eNORMAL;
			//when we have nonuniform scaling we actually have to transform by the transpose of the inverse of vertex2worldSkew.M == transpose of world2vertexSkew:
			hit.normal = world2vertexSkew.rotateTranspose(bestVertSpacePlane->n);
			hit.normal.normalize();
		}
		hit.flags = outFlags;

		return 1;
	}

	return 0;
}

struct RayMeshColliderCallback  : public MeshHitCallback<PxRaycastHit>
{
	PxRaycastHit*		mDstBase;
	PxU32				mHitNum;
	PxU32				mMaxHits;
	const PxMeshScale*	mScale;
	const PxTransform*	mPose;
	const Cm::Matrix34*	mWorld2vertexSkew;
	PxU32				mHintFlags;
	Gu::RaycastHitFunc	mHitCB;
	void*				mUserData;
	const PxVec3&		mRayDir;
	bool				mIsDoubleSided;
	bool				mOverflow;
	float				mDistCoef;

	RayMeshColliderCallback(
		CallbackMode::Enum mode_, PxRaycastHit* hits, PxU32 maxHits, const PxMeshScale* scale, const PxTransform* pose,
		const Cm::Matrix34* world2vertexSkew, PxU32 hintFlags, Gu::RaycastHitFunc hitCB, void* userData, const PxVec3& rayDir,
		bool isDoubleSided, float distCoef) :
			MeshHitCallback<PxRaycastHit>	(mode_),
			mDstBase						(hits),
			mHitNum							(0),
			mMaxHits						(maxHits),
			mScale							(scale),
			mPose							(pose),
			mWorld2vertexSkew				(world2vertexSkew),
			mHintFlags						(hintFlags),
			mHitCB							(hitCB),
			mUserData						(userData),
			mRayDir							(rayDir),
			mIsDoubleSided					(isDoubleSided),
			mOverflow						(false),
			mDistCoef						(distCoef)
	{
	}

	// return false for early out
	virtual bool processHit(
		const PxRaycastHit& lHit, const PxVec3& lp0, const PxVec3& lp1, const PxVec3& lp2, PxReal&, const PxU32*)
	{
		const PxReal u = lHit.u, v = lHit.v;
		const PxVec3 localImpact = (1.0f - u - v)*lp0 + u*lp1 + v*lp2;

		//not worth concatenating to do 1 transform: PxMat34Legacy vertex2worldSkew = scaling.getVertex2WorldSkew(absPose);
		// PT: TODO: revisit this for N hits
		PxRaycastHit hit = lHit;
		hit.position	= mPose->transform(mScale->transform(localImpact));
		hit.flags		= PxHitFlag::ePOSITION|PxHitFlag::eDISTANCE|PxHitFlag::eUV;
		hit.normal		= PxVec3(0.0f);
		hit.distance	*= mDistCoef;

		// Compute additional information if needed
		if(mHintFlags & PxHitFlag::eNORMAL)
		{
			// User requested impact normal
			const PxVec3 localNormal = (lp1 - lp0).cross(lp2 - lp0);

			if(mWorld2vertexSkew)
				hit.normal = mWorld2vertexSkew->rotateTranspose(localNormal);				
			else
				hit.normal = hit.normal = mPose->rotate(localNormal);
			hit.normal.normalize();

			// PT: figure out correct normal orientation (DE7458)
			// - if the mesh is single-sided the normal should be the regular triangle normal N, regardless of eMESH_BOTH_SIDES.
			// - if the mesh is double-sided the correct normal can be either N or -N. We take the one opposed to ray direction.
			if(mIsDoubleSided && hit.normal.dot(mRayDir) > 0.0f)
				hit.normal = -hit.normal;

			hit.flags |= PxHitFlag::eNORMAL;
		}

		// PT: if a callback is available, use it
		if(mHitCB)
			return (mHitCB)(hit, mUserData);

		// PT: no callback => store results in provided buffer
		if(mHitNum == mMaxHits)
		{
			mOverflow = true;
			return false;
		}

		// AP: the loop below merges hits at the same distance within some arbitrary tolerance
		// PT: TODO: remove that O(n^2) check (DE6085)
		// PT: the check is done independently in the callback when it's used, since we don't store hits
		// locally anymore in that case.
		// distance check
		for(PxU32 i=0; i<mHitNum; i++)
		{
			if(fabsf(mDstBase[i].distance - hit.distance) < 1e-4)
				return true;
		}

		mDstBase[mHitNum++] = hit;
		return true;
	}

private:
	RayMeshColliderCallback& operator=(const RayMeshColliderCallback&);
};

PxU32 raycast_triangleMesh(GU_RAY_FUNC_PARAMS) 
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	PX_ASSERT((maxHits && hits) || hitCB);
	PX_ASSERT(PxAbs(rayDir.magnitudeSquared()-1)<1e-4);

	PxRaycastHit* PX_RESTRICT dst = hits;
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	GU_FETCH_MESH_DATA(meshGeom)

	//scaling: transform the ray to vertex space

	PxVec3 orig, dir;
	Cm::Matrix34 world2vertexSkew;
	Cm::Matrix34* world2vertexSkewP = NULL;
	PxReal distCoeff = 1.0f;
	if(meshGeom.scale.isIdentity())
	{
		orig = pose.transformInv(rayOrigin);
		dir = pose.rotateInv(rayDir);		
	}
	else
	{
		world2vertexSkew = meshGeom.scale.getInverse() * pose.getInverse();
		world2vertexSkewP = &world2vertexSkew;
		orig = world2vertexSkew.transform(rayOrigin);
		dir = world2vertexSkew.rotate(rayDir);
		{
			distCoeff = dir.normalize();
			maxDist *= distCoeff;
			maxDist += 1e-3f;
			distCoeff = 1.0f / distCoeff;
		}
	}

	const bool isDoubleSided = meshGeom.meshFlags.isSet(PxMeshGeometryFlag::eDOUBLE_SIDED);
	RayMeshColliderCallback callback(
		(hitCB || maxHits > 1) ? CallbackMode::eMULTIPLE : (anyHit ? CallbackMode::eANY : CallbackMode::eCLOSEST),
		dst, maxHits, &meshGeom.scale, &pose, world2vertexSkewP, hintFlags, hitCB, userData, rayDir, isDoubleSided, distCoeff);

	const bool bothSides = isDoubleSided || (hintFlags & PxHitFlag::eMESH_BOTH_SIDES);

	RTreeMidphaseData hmd;
	meshData->mCollisionModel.getRTreeMidphaseData(hmd);
	MPT_SET_CONTEXT("raym", pose, meshGeom.scale);
	MeshRayCollider::collide<0, 1>(orig, dir, maxDist, bothSides, hmd, callback);

	return callback.mHitNum;
}

namespace physx
{
namespace Gu
{

	class RayCastCallback 
	{
	public:
		PxVec3 hitPoint;
		PxU32 hitTriangle;
		bool hit;

		RayCastCallback() : hit(false){}

		PX_INLINE bool underFaceHit(
			const Gu::HeightFieldUtil&, const PxVec3&, const PxVec3&,
			PxF32, PxF32, PxF32, PxU32)
		{ return true; } // true means continue traversal

		bool faceHit(const Gu::HeightFieldUtil&, const PxVec3& aHitPoint, PxU32 aTriangleIndex)
		{
			hitPoint = aHitPoint;
			hitTriangle = aTriangleIndex;
			hit = true;
			return false; // first hit, false = stop traversal
		}
		bool onEvent(PxU32 , PxU32* )
		{
			return true;
		}
	};   
} // namespace
}

PxU32 raycast_heightField(GU_RAY_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eHEIGHTFIELD);
	PX_ASSERT(maxHits && hits);
	PX_UNUSED(userData);
	PX_UNUSED(hitCB);
	PX_UNUSED(anyHit);
	PX_UNUSED(maxHits);

	struct LSEASwitcher
	{
		LSEASwitcher(PxHeightFieldGeometry& geo, PxHeightField* LS)
		{
			pGeo = &geo;
			EA = geo.heightField;
			geo.heightField = LS;
		}

		~LSEASwitcher()
		{
			pGeo->heightField = EA;
		}

		PxHeightFieldGeometry* pGeo;
		PxHeightField* EA;
	};
	const PxHeightFieldGeometry& hfGeom = static_cast<const PxHeightFieldGeometry&>(geom);

	PX_ALIGN_PREFIX(16)  PxU8 heightFieldBuffer[sizeof(Gu::HeightField)+32] PX_ALIGN_SUFFIX(16);
	Gu::HeightField* heightField = Cm::memFetchAsync<Gu::HeightField>(heightFieldBuffer, (uintptr_t)(hfGeom.heightField), sizeof(Gu::HeightField), 1);
		
	hits->flags = PxHitFlags(0);

	PxTransform invAbsPose = pose.getInverse();
	PxVec3 localRayOrig = invAbsPose.transform(rayOrigin);
	PxVec3 localRayDir = invAbsPose.rotate(rayDir);
	
	Cm::memFetchWait(1);

#ifdef __SPU__
	physx::g_sampleCache.init((uintptr_t)(heightField->getData().samples), heightField->getData().tilesU);
#endif

	//hfGeom.heightField = heightField;
	LSEASwitcher switcher(const_cast<PxHeightFieldGeometry&>(hfGeom),heightField);

	Gu::HeightFieldUtil hfUtil(hfGeom);
	RayCastCallback callback;

	PxVec3 normRayDir = localRayDir;
	normRayDir.normalizeSafe(); // nothing will happen if length is < PX_NORMALIZATION_EPSILON
	MPT_SET_CONTEXT("rayh", pose, PxMeshScale());
	hfUtil.traceSegment<RayCastCallback, false, false, false>(localRayOrig, localRayOrig + normRayDir * PxMin(maxDist, 1e10f), &callback);

	if (callback.hit)
	{
		hits->faceIndex = callback.hitTriangle;

		//We need the normal for the dot product.
		PxVec3 normal = pose.q.rotate(hfUtil.getNormalAtShapePoint(callback.hitPoint.x, callback.hitPoint.z)); 
		normal.normalize();

		if (hintFlags & PxHitFlag::eNORMAL)
		{
			hits->normal = normal; 
			hits->flags |= PxHitFlag::eNORMAL;
		}
		if (hintFlags & PxHitFlag::eDISTANCE)
		{
			hits->distance = (callback.hitPoint - localRayOrig).dot(localRayDir);
			hits->flags |= PxHitFlag::eDISTANCE;
		}
		if (hintFlags & PxHitFlag::ePOSITION)
		{
			hits->position = pose.transform(callback.hitPoint);
			hits->flags |= PxHitFlag::ePOSITION;
		}
		
		return 1;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gu::RaycastFunc gRaycastMap[7] =
{
	raycast_sphere,
	raycast_plane,
	raycast_capsule,
	raycast_box,
	raycast_convexMesh,
	raycast_triangleMesh,
	raycast_heightField
};

Gu::GeomRaycastTableEntry7& Gu::GetRaycastFuncTable()
{
	return gRaycastMap;
}

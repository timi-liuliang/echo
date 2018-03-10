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
#include "PxMeshQuery.h"

#include "GuGeomUtilsInternal.h"
#include "PxTriangleMeshGeometry.h"
#include "PxSphereGeometry.h"
#include "PxBoxGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxGeometryQuery.h"
#include "GuEntityReport.h"
#include "GuHeightFieldUtil.h"
#include "GuOverlapTests.h"
#include "GuBoxConversion.h"
#include "GuIntersectionTriangleBox.h"
#include "GuTriangleMesh.h"
#include "CmScaling.h"
#include "GuSweepTests.h"

using namespace physx;
using namespace Gu;

namespace {

	class HfTrianglesEntityReport2 : public EntityReport<PxU32>
	{
	public:
		HfTrianglesEntityReport2(
			PxU32* results,
			PxU32 maxResults,
			PxU32 startIndex,
			HeightFieldUtil& hfUtil,
			const PxVec3& boxCenter,
			const PxVec3& boxExtents,
			const PxQuat& boxRot,
			bool aabbOverlap) :

			mHfUtil			(hfUtil),
			mBoxCenter		(boxCenter),
			mBoxExtents		(boxExtents),
			mBoxRot			(boxRot),
			mResults		(results),
			mMaxResults		(maxResults),
			mStartIndex		(startIndex),
			mNbResults		(0),
			mNbSkipped		(0),
			mAABBOverlap	(aabbOverlap),
			mOverflow		(false)
		{
		}

		PX_FORCE_INLINE	bool	add(PxU32 index)
		{
			if(mNbResults>=mMaxResults)
			{
				mOverflow = true;
				return false;
			}

			if(mNbSkipped>=mStartIndex)
				mResults[mNbResults++] = index;
			else
				mNbSkipped++;

			return true;
		}

		virtual bool onEvent(PxU32 nbEntities, PxU32* entities)
		{
			if(mAABBOverlap)
			{
				while(nbEntities--)
					if(!add(*entities++))
						return false;
			}
			else
			{
				// PT: TODO: use a matrix here
				const PxTransform box2Hf(mBoxCenter, mBoxRot);

				for(PxU32 i=0; i < nbEntities; i++)
				{
					PxTriangle tri;
					mHfUtil.getTriangle(PxTransform(PxIdentity), tri, NULL, NULL, entities[i], false, false);  // First parameter not needed if local space triangle is enough

					// Transform triangle vertices to box space
					const PxVec3 v0 = box2Hf.transformInv(tri.verts[0]);
					const PxVec3 v1 = box2Hf.transformInv(tri.verts[1]);
					const PxVec3 v2 = box2Hf.transformInv(tri.verts[2]);

					const PxVec3 zero(0.0f);
					if(intersectTriangleBox(zero, mBoxExtents, v0, v1, v2))
					{
						if(!add(entities[i]))
							return false;
					}
				}
			}

			return true;
		}

			HeightFieldUtil&	mHfUtil;
			PxVec3				mBoxCenter;
			PxVec3				mBoxExtents;
			PxQuat				mBoxRot;
			PxU32*				mResults;
			PxU32				mMaxResults;
			PxU32				mStartIndex;
			PxU32				mNbResults;
			PxU32				mNbSkipped;
			bool				mAABBOverlap;
			bool				mOverflow;

	private:
		HfTrianglesEntityReport2& operator=(const HfTrianglesEntityReport2&);
	};


} // namespace

void physx::PxMeshQuery::getTriangle(const PxTriangleMeshGeometry& triGeom, const PxTransform& globalPose, PxTriangleID triangleIndex, PxTriangle& triangle, PxU32* vertexIndices, PxU32* adjacencyIndices)
{
	TriangleMesh* tm = static_cast<TriangleMesh*>(triGeom.triangleMesh);

	PX_CHECK_AND_RETURN(triangleIndex<tm->getNbTriangles(), "PxMeshQuery::getTriangle: triangle index is out of bounds");

	if(adjacencyIndices && !tm->mMesh.getAdjacencies())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Adjacency information not created. Set buildTriangleAdjacencies on Cooking params.");
	}	

	const Cm::Matrix34 vertex2worldSkew = globalPose * triGeom.scale;
	tm->computeWorldTriangle(triangle, triangleIndex, vertex2worldSkew, vertexIndices, adjacencyIndices);
}

///////////////////////////////////////////////////////////////////////////////

void physx::PxMeshQuery::getTriangle(const PxHeightFieldGeometry& hfGeom, const PxTransform& globalPose, PxTriangleID triangleIndex, PxTriangle& triangle, PxU32* vertexIndices, PxU32* adjacencyIndices)
{
	HeightFieldUtil hfUtil(hfGeom);
	
	hfUtil.getTriangle(globalPose, triangle, vertexIndices, adjacencyIndices, triangleIndex, true, true);
}

///////////////////////////////////////////////////////////////////////////////

PxU32 physx::PxMeshQuery::findOverlapTriangleMesh(
	const PxGeometry& geom, const PxTransform& geomPose,
	const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose,
	PxU32* results, PxU32 maxResults, PxU32 startIndex, bool& overflow)
{
	PX_SIMD_GUARD;
	switch(geom.getType())
	{
		case PxGeometryType::eBOX:
		{
			const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

			Box box;
			buildFrom(box, geomPose.p, boxGeom.halfExtents, geomPose.q);

			TriangleMesh* tm = static_cast<TriangleMesh*>(meshGeom.triangleMesh);
			return findOverlapOBBMesh(box, tm->getCollisionModel(), meshPose, meshGeom.scale, results, maxResults, startIndex, overflow);
		}

		case PxGeometryType::eCAPSULE:
		{
			const PxCapsuleGeometry& capsGeom = static_cast<const PxCapsuleGeometry&>(geom);

			Capsule capsule;
			getCapsule(capsule, capsGeom, geomPose);

			TriangleMesh* tm = static_cast<TriangleMesh*>(meshGeom.triangleMesh);
			return findOverlapCapsuleMesh(
				capsule, tm->getCollisionModel(), meshPose, meshGeom.scale, results, maxResults, startIndex, overflow);
		}

		case PxGeometryType::eSPHERE:
		{
			const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

			TriangleMesh* tm = static_cast<TriangleMesh*>(meshGeom.triangleMesh);
			return findOverlapSphereMesh(Sphere(geomPose.p, sphereGeom.radius), tm->getCollisionModel(), meshPose, meshGeom.scale, results, maxResults, startIndex, overflow);
		}
		case PxGeometryType::ePLANE:
		case PxGeometryType::eCONVEXMESH:
		case PxGeometryType::eTRIANGLEMESH:
		case PxGeometryType::eHEIGHTFIELD:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default:
		{
			overflow = false;
			PX_CHECK_MSG(false, "findOverlapTriangleMesh: Only box, capsule and sphere geometries are supported.");
			return false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

PxU32 physx::PxMeshQuery::findOverlapHeightField(	const PxGeometry& geom, const PxTransform& geomPose,
													const PxHeightFieldGeometry& hfGeom, const PxTransform& hfPose,
													PxU32* results, PxU32 maxResults, PxU32 startIndex, bool& overflow)
{
	PX_SIMD_GUARD;
	const PxTransform localPose0 = hfPose.transformInv(geomPose);
	PxBoxGeometry boxGeom;

	switch(geom.getType())
	{
		case PxGeometryType::eCAPSULE:
		{
			const PxCapsuleGeometry& cap = static_cast<const PxCapsuleGeometry&>(geom);
			boxGeom.halfExtents = PxVec3(cap.halfHeight+cap.radius, cap.radius, cap.radius);
		}
		break;
		case PxGeometryType::eSPHERE:
		{
			const PxSphereGeometry& sph = static_cast<const PxSphereGeometry&>(geom);
			boxGeom.halfExtents = PxVec3(sph.radius, sph.radius, sph.radius);
		}
		break;
		case PxGeometryType::eBOX:
			boxGeom = static_cast<const PxBoxGeometry&>(geom);
		break;
		case PxGeometryType::ePLANE:
		case PxGeometryType::eCONVEXMESH:
		case PxGeometryType::eTRIANGLEMESH:
		case PxGeometryType::eHEIGHTFIELD:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default:
		{
			overflow = false;
			PX_CHECK_AND_RETURN_VAL(false, "findOverlapHeightField: Only box, sphere and capsule queries are supported.", false);
		}
	}

	const bool isAABB = ((localPose0.q.x == 0.0f) && (localPose0.q.y == 0.0f) && (localPose0.q.z == 0.0f));
	
	PxBounds3 bounds;
	if (isAABB)
		bounds = PxBounds3::centerExtents(localPose0.p, boxGeom.halfExtents);
	else
		bounds = PxBounds3::poseExtent(localPose0, boxGeom.halfExtents); // box.halfExtents is really extent

	HeightFieldUtil hfUtil(hfGeom);
	HfTrianglesEntityReport2 entityReport(results, maxResults, startIndex, hfUtil, localPose0.p, boxGeom.halfExtents, localPose0.q, isAABB);

	// PT: TODO: add a helper to expose this number?
/*			const PxU32 maxNbOverlapRows = static_cast<PxU32>( Ps::ceil(((bounds.getDimensions().x * (1.0f / hfGeom.rowScale)) + 1.0f)) );
	const PxU32 maxNbOverlapCols = static_cast<PxU32>( Ps::ceil(((bounds.getDimensions().z * (1.0f / hfGeom.columnScale)) + 1.0f)) );

	PxU32 maxNbTriangles = (maxNbOverlapCols * maxNbOverlapRows) << 1;  // maximum number of height field triangles overlapping the local AABB
	maxNbTriangles = PxMax(maxNbTriangles, (PxU32)8);  // No matter how small the AABB is, it can always have its center at the shared point of 4 cells*/

	MPT_SET_CONTEXT("mqoh", hfPose, PxMeshScale());
	hfUtil.overlapAABBTriangles(hfPose, bounds, 0, &entityReport);
	overflow = entityReport.mOverflow;
	return entityReport.mNbResults;
}

///////////////////////////////////////////////////////////////////////////////

bool physx::PxMeshQuery::sweep(	const PxVec3& unitDir, const PxReal maxDistance,
								const PxGeometry& geom, const PxTransform& pose,
								PxU32 triangleCount, const PxTriangle* triangles,
								PxSweepHit& sweepHit, PxHitFlags hintFlags_,
								const PxU32* cachedIndex, const PxReal inflation, bool doubleSided)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(pose.isValid(), "Gu::GeometryQuery::sweep(): pose is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(unitDir.isFinite(), "Gu::GeometryQuery::sweep(): unitDir is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(PxIsFinite(maxDistance), "Gu::GeometryQuery::sweep(): distance is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(maxDistance > 0, "Gu::GeometryQuery::sweep(): sweep distance must be greater than 0.", false);

	const PxReal distance = PxMin(maxDistance, PX_MAX_SWEEP_DISTANCE);

	// PT: the doc says that validity flags are not used, but internally some functions still check them. So
	// to make sure the code works even when no validity flags are passed, we set them all here.
	const PxHitFlags hintFlags = hintFlags_ | PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE;

	switch(geom.getType())
	{
		case PxGeometryType::eSPHERE:
		{
			const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

			// PT: TODO: technically this capsule with 0.0 half-height is invalid ("isValid" returns false)
			const PxCapsuleGeometry capsuleGeom(sphereGeom.radius, 0.0f);

			return SweepCapsuleTriangles(	triangleCount, triangles, doubleSided, capsuleGeom, pose, unitDir, distance,
											cachedIndex, sweepHit.position, sweepHit.normal, sweepHit.distance, sweepHit.faceIndex, inflation, hintFlags);
		}

		case PxGeometryType::eCAPSULE:
		{
			const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

			return SweepCapsuleTriangles(	triangleCount, triangles, doubleSided, capsuleGeom, pose, unitDir, distance,
											cachedIndex, sweepHit.position, sweepHit.normal, sweepHit.distance, sweepHit.faceIndex, inflation, hintFlags);
		}

		case PxGeometryType::eBOX:
		{
			const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

			if(!PX_IS_SPU && (hintFlags & PxHitFlag::ePRECISE_SWEEP))
			{
				return sweepCCTBoxTriangles(triangleCount, triangles, doubleSided, boxGeom, pose, 
											unitDir, distance, sweepHit.position, sweepHit.normal, sweepHit.distance,
											sweepHit.faceIndex, cachedIndex, inflation, hintFlags);
			}
			else
			{
				return SweepBoxTriangles(	triangleCount, triangles, doubleSided, boxGeom, pose, 
											unitDir, distance, sweepHit.position, sweepHit.normal, sweepHit.distance,
											sweepHit.faceIndex, cachedIndex, inflation, hintFlags);
			}
		}	
		case PxGeometryType::ePLANE:
		case PxGeometryType::eCONVEXMESH:
		case PxGeometryType::eTRIANGLEMESH:
		case PxGeometryType::eHEIGHTFIELD:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default :
			PX_CHECK_MSG(false, "Gu::GeometryQuery::sweep(): geometry object parameter must be sphere, capsule or box geometry.");
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

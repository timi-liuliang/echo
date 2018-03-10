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

#include "NpSceneQueries.h"
#include "NpReadCheck.h"
#include "SqSceneQueryManager.h"
#include "GuBoxConversion.h"
#include "GuGeomUtilsInternal.h"	// for getCapsule
#include "GuIntersectionRayBox.h" // for intersectRayAABB2
#include "PxGeometryQuery.h"
#include "NpBatchQuery.h"
#include "PxFiltering.h"
#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"
#include "NpArticulationLink.h"
#include "NpQueryShared.h"
#include "SqAABBPruner.h"
#include "GuOverlapTests.h"
#include "GuRaycastTests.h"
#include "GuSweepTests.h"

// Synchronous scene queries

using namespace physx;
using namespace Sq;

#if PX_IS_SPU
#undef PX_SUPPORT_VISUAL_DEBUGGER
#endif

#if PX_SUPPORT_VISUAL_DEBUGGER
#include "NpPvdSceneQueryCollector.h"
#endif


///////////////////////////////////////////////////////////////////////////////
bool NpSceneQueries::raycast(
	const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
	PxHitCallback<PxRaycastHit>& hits, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
	const PxQueryCache* cache) const
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(const_cast<Scb::Scene&>(mScene),SceneQuery,raycastMultiple);
	NP_READ_CHECK(this);	
	PX_SIMD_GUARD;

	MultiQueryInput input(origin, unitDir, distance);
	bool result = multiQuery<PxRaycastHit>(input, hits, hitFlags, cache, filterData, filterCall);
	return result;
}

//////////////////////////////////////////////////////////////////////////
bool NpSceneQueries::overlap(
	const PxGeometry& geometry, const PxTransform& pose, PxOverlapCallback& hits,
	const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall) const
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(const_cast<Scb::Scene&>(mScene),SceneQuery,overlapMultiple);
	NP_READ_CHECK(this);	
	PX_SIMD_GUARD;

	MultiQueryInput input(&geometry, &pose);
	// we are not supporting cache for overlaps for some reason
	bool result = multiQuery<PxOverlapHit>(input, hits, PxHitFlags(), NULL, filterData, filterCall);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
bool NpSceneQueries::sweep(
	const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
	PxHitCallback<PxSweepHit>& hits, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
	const PxQueryCache* cache, const PxReal inflation) const
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(const_cast<Scb::Scene&>(mScene),SceneQuery,sweepMultiple);
	NP_READ_CHECK(this);	
	PX_SIMD_GUARD;

	if((hitFlags & PxHitFlag::ePRECISE_SWEEP) && (hitFlags & PxHitFlag::eMTD))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, " Precise sweep doesn't support MTD. Perform MTD with default sweep");
		hitFlags &= ~PxHitFlag::ePRECISE_SWEEP;
	}

	if((hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP) && (hitFlags & PxHitFlag::eMTD))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, " eMTD cannot be used in conjunction with eASSUME_NO_INITIAL_OVERLAP. eASSUME_NO_INITIAL_OVERLAP will be ignored");
		hitFlags &= ~PxHitFlag::eASSUME_NO_INITIAL_OVERLAP;
	}

	PxReal realInflation = inflation;
	if((hitFlags & PxHitFlag::ePRECISE_SWEEP)&& inflation > 0.f)
	{
		realInflation = 0.f;
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, " Precise sweep doesn't support inflation, inflation will be overwritten to be zero");
	}
	MultiQueryInput input(&geometry, &pose, unitDir, distance, realInflation);
	bool result = multiQuery<PxSweepHit>(input, hits, hitFlags, cache, filterData, filterCall);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//========================================================================================================================
NpQuerySpuContext* gSpuContext = NULL; // only not NULL on SPU

static PX_FORCE_INLINE bool applyAllPreFiltersSQ(
	const PxActorShape2* as, PxQueryHitType::Enum& hitType, const PxQueryFlags& inFilterFlags,
	const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
	const NpSceneQueries& scene, BatchQueryFilterData* bfd, PxHitFlags& queryFlags, PxU32 /*maxNbTouches*/)
{
	//pxPrintf("prefilters 0\n");
	if (filterData.clientId != as->scbActor->getOwnerClient())
	{
		//pxPrintf("prefilters 0a\n");
		const bool passForeignShapes = PX_IS_SPU ? gSpuContext->scenePassForeignShapes // no valid scene ptr on SPU
			: scene.getClientBehaviorFlags(filterData.clientId) & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_SCENE_QUERY;
		//pxPrintf("prefilters 0a, as = 0x%x\n", PxU32(as));
		//pxPrintf("prefilters 0a, as.scb = 0x%x\n", PxU32(as->scbActor));
		const bool reportToForeignClients = as->scbActor->getClientBehaviorFlags() & PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_SCENE_QUERY;
		// was:
		//const bool reportToForeignClients = (prunable.scbActor->getClientBehaviorBits() & PxActorClientBehaviorBit::eREPORT_TO_FOREIGN_CLIENTS_SCENE_QUERY) != 0;
		if (!(passForeignShapes && reportToForeignClients))
		{
			//pxPrintf("prefilters 0a returning null\n");
			return false;
		}
	}
	//pxPrintf("prefilters 1\n");

	// if the filterData field is non-zero, and the bitwise-AND value of filterData AND the shape's
	// queryFilterData is zero, the shape is skipped.
	const PxFilterData& queryFd = filterData.data;

	// AP: the !bfd clause is here because there's no other way to pass data to BQ pre/post filter shaders
	// For normal query the data can be passed with inherited callback instance
	// So if for BQ SPU filter shader the user tries to pass data via FD, the equation will always cut it out
	// AP scaffold TODO: once SPU is officially phased out we can remove the !bfd clause, fix broken UTs (that are wrong)
	// and also remove support for filter shaders
	if(!bfd && (queryFd.word0 | queryFd.word1 | queryFd.word2 | queryFd.word3) != 0)
	{
		const PxFilterData& objFd = as->scbShape->getScShape().getQueryFilterData();
		PxU32 keep = (queryFd.word0 & objFd.word0) | (queryFd.word1 & objFd.word1) | (queryFd.word2 & objFd.word2) | (queryFd.word3 & objFd.word3);
		if (!keep)
			return false;
	}
	//pxPrintf("prefilters 2\n");

	if((inFilterFlags & PxQueryFlag::ePREFILTER) && (filterCall || bfd))
	{
		PxHitFlags outQueryFlags = queryFlags;

		if (filterCall)
			hitType = filterCall->preFilter(queryFd, as->shape, as->actor, outQueryFlags);
		else if (bfd->preFilterShader)
			hitType = bfd->preFilterShader(
				filterData.data, as->scbShape->getScShape().getQueryFilterData(),
				bfd->filterShaderData, bfd->filterShaderDataSize, outQueryFlags);

		// AP: at this point the callback might return eTOUCH but the touch buffer can be empty, the hit will be discarded
		//PX_CHECK_MSG(hitType == PxQueryHitType::eTOUCH ? maxNbTouches > 0 : true,
		//	"SceneQuery: preFilter returned eTOUCH but empty touch buffer was provided, hit discarded.");

		queryFlags = (queryFlags & ~PxHitFlag::eMODIFIABLE_FLAGS) | (outQueryFlags & PxHitFlag::eMODIFIABLE_FLAGS);
	}
	// test passed, continue to return as;
	//pxPrintf("prefilters 3\n");

	return true;
}

//========================================================================================================================
// performs a single geometry query for any HitType (PxSweepHit, PxOverlapHit, PxRaycastHit)
template<typename HitType>
struct GeomQueryAny
{
	static PX_FORCE_INLINE PxU32 geomHit(
		const SceneQueryManager& sqm, const MultiQueryInput& input, const ShapeData& sd,
		const PxGeometry& sceneGeom, const PxTransform& pose, PxHitFlags hitFlags,
		PxU32 maxHits, HitType* hits, const PxReal shrunkMaxDistance, bool anyHit, PxBounds3* precomputedBounds)
	{
		const PxGeometry& geom0 = *input.geometry;
		const PxTransform& pose0 = *input.pose;
		const PxGeometry& geom1 = sceneGeom;
		const PxTransform& pose1 = pose;

		// Handle raycasts
		if (HitTypeSupport<HitType>::IsRaycast)
		{
			// the test for mesh AABB is archived in //sw/physx/dev/apokrovsky/graveyard/sqMeshAABBTest.cpp
			// TODO: investigate performance impact (see US12801)
			PX_CHECK_AND_RETURN_VAL(input.getDir().isFinite(), "PxScene::raycast(): rayDir is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL(input.getOrigin().isFinite(), "PxScene::raycast(): rayOrigin is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL(pose1.isValid(), "PxScene::raycast(): pose is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL(shrunkMaxDistance >= 0.0f, "PxScene::raycast(): maxDist is negative.", false);
			PX_CHECK_AND_RETURN_VAL(PxIsFinite(shrunkMaxDistance), "PxScene::raycast(): maxDist is not valid.", false);
			PX_CHECK_AND_RETURN_VAL(PxAbs(input.getDir().magnitudeSquared()-1)<1e-4,
				"PxGeometryQuery::raycast(): ray direction must be unit vector.", false);

			Gu::RaycastFunc func = PX_IS_SPU ? Gu::GetRaycastFuncTable()[geom1.getType()] : sqm.mCachedRaycastFuncs[geom1.getType()];
			return func(geom1, pose1, input.getOrigin(), input.getDir(), shrunkMaxDistance,
						hitFlags, maxHits, (PxRaycastHit*)hits, anyHit, NULL, NULL);
		}
		// Handle sweeps
		else if (HitTypeSupport<HitType>::IsSweep)
		{
			PX_ASSERT(precomputedBounds != NULL);
			// b0 = query shape bounds
			// b1 = scene shape bounds
			// AP: Here we clip the sweep to bounds with sum of extents. This is needed for GJK stability.
			// because sweep is equivalent to a raycast vs a scene shape with inflated bounds.
			// This also may (or may not) provide an optimization for meshes because top level of rtree has multiple boxes
			// and there is no bounds test for the whole mesh elsewhere
			PxBounds3 b0 = *precomputedBounds, b1;
			// compute the scene geometry bounds
			reinterpret_cast<const Gu::GeometryUnion&>(sceneGeom).computeBounds(b1, pose, 0.0f, NULL);
			const PxVec3 combExt = (b0.getExtents() + b1.getExtents())*1.01f;
			PxF32 tnear, tfar;
			if (!Gu::intersectRayAABB2(-combExt, combExt, b0.getCenter() - b1.getCenter(), input.getDir(), shrunkMaxDistance, tnear, tfar)) // returns (tnear<tfar)
				if(tnear>tfar) // this second test is needed because shrunkMaxDistance can be 0 for 0 length sweep
					return 0;
			PX_ASSERT(input.getDir().isNormalized());
			// tfar is now the t where the ray exits the AABB. input.getDir() is normalized

			const PxVec3& unitDir = input.getDir();
			PxSweepHit& sweepHit = (PxSweepHit&)hits[0];
			const PxReal distance = PxMin(tfar, shrunkMaxDistance);
			const PxReal inflation = input.inflation;
			PX_CHECK_AND_RETURN_VAL(pose0.isValid(), "PxScene::sweep(): pose0 is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL(pose1.isValid(), "PxScene::sweep(): pose1 is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL(unitDir.isFinite(), "PxScene::sweep(): unitDir is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL(PxIsFinite(distance), "PxScene::sweep(): distance is not valid.", 0);
			PX_CHECK_AND_RETURN_VAL((distance >= 0.0f && !(hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP)) || distance > 0.0f,
				"PxScene::sweep(): sweep distance must be >=0 or >0 with eASSUME_NO_INITIAL_OVERLAP.", 0);

			const Gu::GeomSweepFuncs &sf = PX_IS_SPU ? Gu::GetGeomSweepFuncs() : sqm.mCachedSweepFuncs;
			switch(geom0.getType())
			{
				case PxGeometryType::eSPHERE:
				{
					const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
					const Gu::Capsule worldCapsule(pose0.p, pose0.p, sphereGeom.radius); // AP: precompute?
					//pxPrintf("sweep sphere vs %d\n", geom1.getType());
					const bool precise = (!PX_IS_SPU && (hitFlags & PxHitFlag::ePRECISE_SWEEP));
					Gu::SweepCapsuleFunc func = precise ? sf.cctCapsuleMap[geom1.getType()] : sf.capsuleMap[geom1.getType()];
					return (PxU32)func(geom1, pose1, worldCapsule, unitDir, distance, sweepHit, hitFlags, inflation);
				}

				case PxGeometryType::eCAPSULE:
				{
					const bool precise = (!PX_IS_SPU && (hitFlags & PxHitFlag::ePRECISE_SWEEP));
					Gu::SweepCapsuleFunc func = precise ? sf.cctCapsuleMap[geom1.getType()] : sf.capsuleMap[geom1.getType()];
					return (PxU32)func(geom1, pose1, sd.getNPGuCapsule(), unitDir, distance, sweepHit, hitFlags, inflation);
				}

				case PxGeometryType::eBOX:
				{
					const bool precise = (!PX_IS_SPU && (hitFlags & PxHitFlag::ePRECISE_SWEEP));
					Gu::SweepBoxFunc func = precise ? sf.cctBoxMap[geom1.getType()] : sf.boxMap[geom1.getType()];
					const Gu::Box& npGuBox = sd.getNPGuBox();
					return (PxU32)func(geom1, pose1, npGuBox, unitDir, distance, sweepHit, hitFlags, inflation);
				}

				case PxGeometryType::eCONVEXMESH:
				{
					const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom0);
					Gu::SweepConvexFunc func = sf.convexMap[geom1.getType()];
					return (PxU32)func(geom1, pose1, convexGeom, pose0, unitDir, distance, sweepHit, hitFlags, inflation);
				}
				case PxGeometryType::ePLANE:
				case PxGeometryType::eTRIANGLEMESH:
				case PxGeometryType::eHEIGHTFIELD:
				case PxGeometryType::eGEOMETRY_COUNT:
				case PxGeometryType::eINVALID:
				default:
					if (!PX_IS_SPU)
						physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__,
							"PxGeometryQuery::sweep(): first geometry object parameter must be sphere, capsule, box or convex geometry.");
			}
			return 0;
		}
		// Handle overlaps
		else if (HitTypeSupport<HitType>::IsOverlap)
		{
			PX_CHECK_AND_RETURN_VAL(pose0.isValid(), "PxScene::overlap(): pose0 is not valid.", false);
			PX_CHECK_AND_RETURN_VAL(pose1.isValid(), "PxScene::overlap(): pose1 is not valid.", false);
			
			Gu::GeomOverlapTableEntry7* overlapFuncs = PX_IS_SPU ? Gu::GetGeomOverlapMethodTable() : sqm.mCachedOverlapFuncs;
			if(geom0.getType() > geom1.getType())
			{
				Gu::GeomOverlapFunc overlapFunc = overlapFuncs[geom1.getType()][geom0.getType()];
				PX_ASSERT(overlapFunc);
				return (PxU32)overlapFunc(geom1, pose1, geom0, pose0, NULL);
			}
			else
			{
				Gu::GeomOverlapFunc overlapFunc = overlapFuncs[geom0.getType()][geom1.getType()];
				PX_ASSERT(overlapFunc);
				return (PxU32)overlapFunc(geom0, pose0, geom1, pose1, NULL);
			}
		}
		else
		{
			PX_ALWAYS_ASSERT_MESSAGE("Unexpected template expansion in GeomQueryAny::geomHit");
			return 0;
		}
	}
};

// struct to access protected data members in the public PxHitCallback API
template<typename HitType>
struct MultiQueryCallback : public PrunerCallback
{
	const NpSceneQueries&			scene;
	const MultiQueryInput&			input;
	PxHitCallback<HitType>&			hitCall; // local storage on SPU
	PxHitFlags						hitFlags;
	const PxQueryFilterData&		filterData;
	PxQueryFilterCallback*			filterCall;
	PxReal							shrunkDistance;
	BatchQueryFilterData*			bfd; // only not NULL for batch queries
	bool							reportTouchesAgain;
	bool							farBlockFound; // this is to prevent repeated searches for far block
	bool							noBlock;
	bool							anyHit;
	bool							meshAnyHit;
	bool							isCached; // is this call coming as a callback from the pruner or a single item cached callback?

	// The reason we need these bounds is because we need to know combined(inflated shape) bounds to clip the sweep path
	// to be tolerable by GJK precision issues. This test is done for (queryShape vs touchedShapes)
	// So it makes sense to cache the bounds for sweep query shape, otherwise we'd have to recompute them every time
	// Currently only used for sweeps.
	PxBounds3						queryShapeBounds;
	bool							queryShapeBoundsValid;
	ShapeData*						shapeData;

	MultiQueryCallback(
		const NpSceneQueries& scene_, const MultiQueryInput& input_, bool aAnyHit_,
		PxHitCallback<HitType>& hitCall_, PxHitFlags hitFlags_,
		const PxQueryFilterData& filterData_, PxQueryFilterCallback* filterCall_,
		PxReal shrunkDistance_, BatchQueryFilterData* aBfd_) : 
			scene(scene_), input(input_), hitCall(hitCall_), hitFlags(hitFlags_), filterData(filterData_), filterCall(filterCall_),
			shrunkDistance(shrunkDistance_), bfd(aBfd_), reportTouchesAgain(true), farBlockFound(false), queryShapeBoundsValid(false),
			shapeData(NULL)
	{
		isCached = false;
		anyHit = aAnyHit_;
		meshAnyHit = (hitFlags_.isSet(PxHitFlag::eMESH_ANY)) || anyHit;
		noBlock = (filterData_.flags & PxQueryFlag::eNO_BLOCK);
		farBlockFound = noBlock;
	}
	
	virtual PxAgain invoke(PxReal& aDist, const PrunerPayload* aPayload, PxU32 aCount)
	{
		PX_ASSERT(aCount > 0);
		const PxU32 tempCount = PX_IS_SPU ? 16 : 1;
		HitType tempBuf[tempCount];

		//pxPrintf("z nth=%d\n", count);
		// ------------------------ iterate over all the shapes we got from the pruner ----------------------------
		for (PxU32 iShape = 0; iShape < aCount; iShape++)
		{
			PxActorShape2 as;
			Sq::populate(aPayload[iShape], as);

			//pxPrintf("start pr loop\n");
			const PxQueryFlags filterFlags = filterData.flags;
			//pxPrintf("as->actor = %x\n", PxU32(as->actor));
			// for no filter callback, default to eTOUCH for MULTIPLE, eBLOCK otherwise
			// also always treat as eBLOCK if currently tested shape is cached
			// Using eRESERVED flag as a special condition to default to eTOUCH hits while only looking for a single blocking hit
			// from a nested query (see other comments containing #LABEL1)
			PxQueryHitType::Enum shapeHitType =
				((hitCall.maxNbTouches || (filterData.flags & PxQueryFlag::eRESERVED)) && !isCached)
					? PxQueryHitType::eTOUCH
					: PxQueryHitType::eBLOCK;

			// apply pre-filter
			PxHitFlags filteredHitFlags = hitFlags;
			if (!isCached) // don't run filters on single item cache
				if (!applyAllPreFiltersSQ(&as, shapeHitType/*in&out*/, filterFlags, filterData, filterCall,
						scene, bfd, filteredHitFlags, hitCall.maxNbTouches))
					continue; // skip this shape from reporting if prefilter said to do so
			//pxPrintf("after applyAllPreFiltersSQ, sht=%d\n", shapeHitType);
			if(shapeHitType == PxQueryHitType::eNONE)
				continue;

			PX_ASSERT(as.actor && as.shape);
			const Scb::Shape* shape = as.scbShape;
			const Scb::Actor* actor = as.scbActor;

			//pxPrintf("before getGlobalPose\n");
			// compute the global pose for the cached shape and actor
			PxTransform globalPose = Sq::getGlobalPose(*shape, *actor);
			//pxPrintf("done getGlobalPose\n");

			const PxGeometry& shapeGeom = shape->getGeometry();

			// Here we decide whether to use the user provided buffer in place or a local stack buffer
			// see if we have more room left in the callback results buffer than in the parent stack buffer
			// if so get subHits in-place in the hit buffer instead of the parent stack buffer
			// nbTouches is the number of accumulated touch hits so far
			// maxNbTouches is the size of the user buffer
			PxU32 maxSubHits1 = hitCall.maxNbTouches - hitCall.nbTouches; // how much room is left in the user buffer
			HitType* subHits1 = hitCall.touches + hitCall.nbTouches; // pointer to the first free hit in the user buffer
			if (hitCall.nbTouches >= hitCall.maxNbTouches || PX_IS_SPU)
			// if there's no room left in the user buffer or running on SPU, use a stack buffer
			{
				// tried using 64 here - causes check stack code to get generated on xbox, perhaps because of guard page
				// need this buffer in case the input buffer is full but we still want to correctly merge results from later hits
				maxSubHits1 = tempCount;
				subHits1 = reinterpret_cast<HitType*>(tempBuf);
			}

			// limit number of hits to 1 for meshes if eMESH_MULTIPLE wasn't specified. this tells geomQuery to only look for a closest hit
			if (shapeGeom.getType() == PxGeometryType::eTRIANGLEMESH && !(filteredHitFlags & PxHitFlag::eMESH_MULTIPLE))
				maxSubHits1 = 1; // required to only receive 1 hit to pass UTs
			// call the geometry specific intersection template
			//pxPrintf("pose.p=%.3f %.3f %.3f, shd=%.3f\n", globalPose.p.x, globalPose.p.y, globalPose.p.z, shrunkDistance);
			//pxPrintf("ray.p=%.3f %.3f %.3f, ray.d=%.3f %.3f %.3f\n", input.getOrigin().x, input.getOrigin().y, input.getOrigin().z, input.getDir().x, input.getDir().y, input.getDir().z);
			//pxPrintf("before geomHit\n");
			//pxPrintf("maxSubHits1=%d, filtered flags=%d\n", maxSubHits1, PxU32(filteredHitFlags));
			PxU32 nbSubHits = GeomQueryAny<HitType>::geomHit(
				scene.mSceneQueryManager, input, *shapeData, shapeGeom, globalPose,
				// AP: need to force eDISTANCE to be able to sort regardless of whether the user requested it, unless eANY_HIT.
				// Perf benefit is questionable, should we consider eliminating the flag?
				filteredHitFlags | (anyHit ? PxHitFlags(PxHitFlag::eMESH_ANY) : PxHitFlags(PxHitFlag::eDISTANCE)),
				maxSubHits1, subHits1, shrunkDistance, meshAnyHit, queryShapeBoundsValid ? &queryShapeBounds : NULL);
			//pxPrintf("after geomHit (shape type=%d), nbSubHits = %d\n", shapeGeom.getType(), nbSubHits);


			// ------------------------- iterate over geometry subhits -----------------------------------
			for (PxU32 iSubHit = 0; iSubHit < nbSubHits; iSubHit++)
			{
				HitType& hit = subHits1[iSubHit];
				//pxPrintf("iSubHitDist = %.3f\n", HITDIST(hit));
				//pxPrintf("as->actor=%x\n", PxU32(as->actor));
				hit.actor = as.actor;
				hit.shape = as.shape;

				// some additional processing only for sweep hits with initial overlap
				if(HitTypeSupport<HitType>::IsSweep && HITDIST(hit) == 0.0f && !((!PX_IS_SPU) && filteredHitFlags & PxHitFlag::eMTD))
					// PT: necessary as some leaf routines are called with reversed params, thus writing +unitDir there.
					// AP: apparently still necessary to also do in Gu because Gu can be used standalone (without SQ)
					((PxSweepHit&)hit).normal = -input.getDir();

				// start out with hitType for this cached shape set to a pre-filtered hit type
				PxQueryHitType::Enum hitType = shapeHitType;

				// run the post-filter if specified in filterFlags and filterCall is non-NULL
				if(!isCached && (filterCall || bfd) && (filterFlags & PxQueryFlag::ePOSTFILTER))
				{
					if (filterCall)
						hitType = filterCall->postFilter(filterData.data, hit);
					else if (bfd->postFilterShader)
						hitType = bfd->postFilterShader(
							filterData.data, as.scbShape->getScShape().getQueryFilterData(),
							bfd->filterShaderData, bfd->filterShaderDataSize, hit);
				}

				// early out on any hit if eANY_HIT was specified, regardless of hit type
				if (anyHit && hitType != PxQueryHitType::eNONE)
				{
					// block or touch qualifies for qType=ANY type hit => return it as blocking according to spec. Ignore eNONE.
					hitCall.block = hit;
					hitCall.hasBlock = true;
					return false; // found a hit for ANY qType, can early exit now
				}

				if (noBlock)
					hitType = PxQueryHitType::eTOUCH;

				PX_WARN_ONCE(HitTypeSupport<HitType>::IsOverlap && hitType == PxQueryHitType::eBLOCK, 
					"eBLOCK returned from user filter for overlap() query. This may cause undesired behavior. "
					"Consider using PxQueryFlag::eNO_BLOCK for overlap queries.");

				//pxPrintf("pr before switch\n");
				if (hitType == PxQueryHitType::eTOUCH)
				{
					// -------------------------- handle eTOUCH hits ---------------------------------
					// for qType=multiple, store the hit. For other qTypes ignore it.
					// <= is important for initially overlapping sweeps
					#ifdef PX_CHECKED
					if (hitCall.maxNbTouches == 0 && !bfd && !filterData.flags.isSet(PxQueryFlag::eRESERVED))
						// issue a warning if eTOUCH was returned by the prefilter, we have 0 touch buffer and not a batch query
						// not doing for BQ because the touches buffer can be overflown and thats ok by spec
						// eRESERVED to avoid a warning from nested callback (closest blocking hit recursive search)
						Ps::getFoundation().getErrorHandler().reportError(PxErrorCode::eINVALID_OPERATION,
							"User filter returned PxQueryHitType::eTOUCH but the touches buffer was empty. Hit was discarded.",
							__FILE__, __LINE__);
					#endif

					if (hitCall.maxNbTouches && reportTouchesAgain && HITDIST(hit) <= shrunkDistance)
					{
						// Buffer full: need to find the closest blocking hit, clip touch hits and flush the buffer
						if (hitCall.nbTouches == hitCall.maxNbTouches)
						{
							// issue a second nested query just looking for the closest blocking hit
							// could do better perf-wise by saving traversal state (start looking for blocking from this point)
							// but this is not a perf critical case because users can provide a bigger buffer
							// that covers non-degenerate cases
							// far block search doesn't apply to overlaps because overlaps don't work with blocking hits
							if (HitTypeSupport<HitType>::IsOverlap == 0)
							{
								// AP: the use of eRESERVED is a bit tricky, see other comments containing #LABEL1
								PxQueryFilterData fd1 = filterData; fd1.flags |= PxQueryFlag::eRESERVED;
								PxHitBuffer<HitType> buf1; // create a temp callback buffer for a single blocking hit
								if (!farBlockFound && hitCall.maxNbTouches > 0 && scene.NpSceneQueries::multiQuery<HitType>(
									input, buf1, hitFlags, NULL, fd1, filterCall, bfd))
								{
									hitCall.block = buf1.block;
									hitCall.hasBlock = true;
									hitCall.nbTouches =
										clipHitsToNewMaxDist<HitType>(hitCall.touches, hitCall.nbTouches, HITDIST(buf1.block));
									shrunkDistance = HITDIST(buf1.block);
									aDist = shrunkDistance;
								}
								farBlockFound = true;
							}
							//pxPrintf("touch buffer overflow, nbTouches = %d\n", hitCall.maxNbTouches);
							if (hitCall.nbTouches == hitCall.maxNbTouches)
							{
								// on SPU hitCall is PxOverflowBuffer, so still local on SPU, no modifications needed
								reportTouchesAgain = hitCall.processTouches(hitCall.touches, hitCall.nbTouches);
								if (!reportTouchesAgain)
									return false; // optimization - buffer is full 
								else
									hitCall.nbTouches = 0; // reset nbTouches so we can continue accumulating again
							}
						}

						//pxPrintf("writing touch, dist=%.3f, actor=%d\n", HITDIST(hit), PxU32(hit.actor));
						//if (hitCall.nbTouches < hitCall.maxNbTouches) // can be true if maxNbTouches is 0
						writeHit<HitType>(hitCall.touches, hitCall.nbTouches++, hit);
					} // if (hitCall.maxNbTouches && reportTouchesAgain && HITDIST(hit) <= shrunkDistance)
				} // if (hitType == PxQueryHitType::eTOUCH)
				else if (hitType == PxQueryHitType::eBLOCK)
				{
					// -------------------------- handle eBLOCK hits ----------------------------------
					// only eBLOCK qualifies as a closest hit candidate => compare against best distance and store
					// <= is needed for eTOUCH hits to be recorded correctly vs same eBLOCK distance for overlaps
					if (HITDIST(hit) <= shrunkDistance)
					{
						if (HitTypeSupport<HitType>::IsOverlap == 0)
						{
							shrunkDistance = HITDIST(hit);
							aDist = shrunkDistance;
						}
						hitCall.block = hit;
						hitCall.hasBlock = true;
					}
				} // if (hitType == eBLOCK)
				else {
					PX_ASSERT(hitType == PxQueryHitType::eNONE);
				}
			} // for iSubHit
		} // for iShape


		//pxPrintf("returning stabContinue, nth=%d\n", numTotalHits);
		return true;
	}

private:
	MultiQueryCallback<HitType>& operator=(const MultiQueryCallback<HitType>&);
};

//========================================================================================================================
#if PX_SUPPORT_VISUAL_DEBUGGER
template<typename HitType>
struct CapturePvdOnReturn : public PxHitCallback<HitType>
{
	// copy the arguments of multiQuery into a struct, this is strictly for PVD recording
	const NpSceneQueries* sq;
	const MultiQueryInput& input;
	PxHitFlags hitFlags;
	const PxQueryCache* cache;
	const PxQueryFilterData& filterData;
	PxQueryFilterCallback* filterCall;
	BatchQueryFilterData* bfd;
	Array<HitType> allHits;
	PxHitCallback<HitType>& parentCallback;

	CapturePvdOnReturn(
		const NpSceneQueries* sq_, const MultiQueryInput& input_, PxHitFlags hitFlags_,
		const PxQueryCache* cache_, const PxQueryFilterData& filterData_, PxQueryFilterCallback* filterCall_,
		BatchQueryFilterData* bfd_, PxHitCallback<HitType>& parentCallback_)
			: PxHitCallback<HitType>(parentCallback_.touches, parentCallback_.maxNbTouches), sq(sq_), input(input_), hitFlags(hitFlags_),
			cache(cache_), filterData(filterData_), filterCall(filterCall_), bfd(bfd_), parentCallback(parentCallback_)
	{}

	virtual PxAgain processTouches(const HitType* hits, PxU32 nbHits)
	{
		PxAgain again = parentCallback.processTouches(hits, nbHits);
		for (PxU32 i = 0; i < nbHits; i++)
			allHits.pushBack(hits[i]);
		return again;
	}

	~CapturePvdOnReturn()
	{
		if(!(sq->mScene.getSceneVisualDebugger().isConnected(true) && IS_PVD_SQ_ENABLED))
			return;

		Pvd::PvdSceneQueryCollector& collector = bfd ? sq->getBatchedSqCollector() : sq->getSingleSqCollector();

		if(parentCallback.nbTouches)
		{
			for(PxU32 i = 0; i < parentCallback.nbTouches; i++)
				allHits.pushBack(parentCallback.touches[i]);
		}

		if(parentCallback.hasBlock)
			allHits.pushBack( parentCallback.block );

		bool retBool = allHits.size() > 0;

		QType::Enum qType = (filterData.flags & PxQueryFlag::eANY_HIT) ? QType::eANY : (this->maxNbTouches ? QType::eMULTIPLE : QType::eSINGLE);
		// raycasts
		if (qType == QType::eANY && HitTypeSupport<HitType>::IsRaycast)
			collector.raycastAnyWithLock(
				input.getOrigin(), input.getDir(), input.maxDistance, (PxRaycastHit*)allHits.begin(), retBool, filterData.data, filterData.flags);
		else if (qType == QType::eSINGLE && HitTypeSupport<HitType>::IsRaycast)
			collector.raycastSingleWithLock(
				input.getOrigin(), input.getDir(), input.maxDistance, (PxRaycastHit*)allHits.begin(), retBool, filterData.data, filterData.flags);
		else if (qType == QType::eMULTIPLE && HitTypeSupport<HitType>::IsRaycast)
			collector.raycastMultipleWithLock(
			input.getOrigin(), input.getDir(), input.maxDistance, (PxRaycastHit*)allHits.begin(), (PxU32)allHits.size(), filterData.data, filterData.flags);

		// all overlaps down the same path
		else if (HitTypeSupport<HitType>::IsOverlap)
			collector.overlapMultipleWithLock(*input.geometry, *input.pose, (PxOverlapHit*)allHits.begin(), (PxU32)allHits.size(), filterData.data, filterData.flags);

		// sweeps
		else if (qType == QType::eANY && HitTypeSupport<HitType>::IsSweep)
			collector.sweepAnyWithLock(
				*input.geometry, *input.pose, input.getDir(), input.maxDistance, (PxSweepHit*)allHits.begin(), retBool, filterData.data, filterData.flags);
		else if (qType == QType::eSINGLE && HitTypeSupport<HitType>::IsSweep)
			collector.sweepSingleWithLock(
				*input.geometry, *input.pose, input.getDir(), input.maxDistance, (PxSweepHit*)allHits.begin(), retBool, filterData.data, filterData.flags);
		else if (qType == QType::eMULTIPLE && HitTypeSupport<HitType>::IsSweep)
			collector.sweepMultipleWithLock(
				*input.geometry, *input.pose, input.getDir(), input.maxDistance, (PxSweepHit*)allHits.begin(), (PxU32)allHits.size(), filterData.data, filterData.flags);
	}

private:
	CapturePvdOnReturn<HitType>& operator=(const CapturePvdOnReturn<HitType>&);
};
#endif // PX_SUPPORT_VISUAL_DEBUGGER

//========================================================================================================================
template<typename HitType>
struct IssueCallbacksOnReturn
{
	PxHitCallback<HitType>& hits; // local storage on SPU
	PxAgain again;	// query was stopped by previous processTouches. This means that nbTouches is still non-zero
					// but we don't need to issue processTouches again
	PX_FORCE_INLINE IssueCallbacksOnReturn(PxHitCallback<HitType>& aHits) : hits(aHits) // aHits = local storage on SPU
	{
		again = true;
	}

	~IssueCallbacksOnReturn()
	{
		if (again)
			// only issue processTouches if query wasn't stopped
			// this is because nbTouches doesn't get reset to 0 in this case (according to spec)
			// and the touches in touches array were already processed by the callback
		{
			if (hits.hasBlock && hits.nbTouches)
				hits.nbTouches = clipHitsToNewMaxDist<HitType>(hits.touches, hits.nbTouches, HITDIST(hits.block));
			if (hits.nbTouches)
			{
				bool again_ = hits.processTouches(hits.touches, hits.nbTouches);
				if (again_)
					hits.nbTouches = 0;
			}
		}
		hits.finalizeQuery();
	}

private:
	IssueCallbacksOnReturn<HitType>& operator=(const IssueCallbacksOnReturn<HitType>&);
};

#undef HITDIST

//========================================================================================================================
template<typename HitType>
bool NpSceneQueries::multiQuery(
	const MultiQueryInput& input, PxHitCallback<HitType>& hits, PxHitFlags hitFlags, const PxQueryCache* cache,
	const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall, BatchQueryFilterData* bfd) const
{
	const bool anyHit = (filterData.flags & PxQueryFlag::eANY_HIT) == PxQueryFlag::eANY_HIT;

	PxI32 retval = 0; PX_UNUSED(retval);

	if (HitTypeSupport<HitType>::IsRaycast == 0)
	{
		PX_CHECK_AND_RETURN_VAL(input.pose != NULL, "NpSceneQueries::overlap/sweep pose is NULL.", 0);
		PX_CHECK_AND_RETURN_VAL(input.pose->isValid(), "NpSceneQueries::overlap/sweep pose is not valid.", 0);
	}
	else
	{
		PX_CHECK_AND_RETURN_VAL(input.getOrigin().isFinite(), "NpSceneQueries::raycast pose is not valid.", 0);
	}

	if (HitTypeSupport<HitType>::IsOverlap == 0)
	{
		PX_CHECK_AND_RETURN_VAL(input.getDir().isFinite(), "NpSceneQueries multiQuery input check: unitDir is not valid.", 0);
		PX_CHECK_AND_RETURN_VAL(input.getDir().isNormalized(), "NpSceneQueries multiQuery input check: direction must be normalized", 0);
	}

	if (HitTypeSupport<HitType>::IsRaycast)
	{
		PX_CHECK_AND_RETURN_VAL(input.maxDistance > 0.0f, "NpSceneQueries::multiQuery input check: distance cannot be negative or zero", 0);
	}

	if (HitTypeSupport<HitType>::IsOverlap && !anyHit)
	{
		PX_CHECK_AND_RETURN_VAL(hits.maxNbTouches > 0, "PxScene::overlap() and PxBatchQuery::overlap() calls without eANY_HIT flag require a touch hit buffer for return results.", 0);
	}

	if (HitTypeSupport<HitType>::IsSweep)
	{
		PX_CHECK_AND_RETURN_VAL(input.maxDistance >= 0.0f, "NpSceneQueries multiQuery input check: distance cannot be negative", 0);
		PX_CHECK_AND_RETURN_VAL(input.maxDistance != 0.0f || !(hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP),
			"NpSceneQueries multiQuery input check: zero-length sweep only valid without the PxHitFlag::eASSUME_NO_INITIAL_OVERLAP flag", 0);
	}

	PX_CHECK_MSG(!cache || (cache && cache->shape && cache->actor), "Raycast cache specified but shape or actor pointer is NULL!");	
	// TODO: cache is disabled on SPU
	Sq::ActorShape* cacheData = cache && !PX_IS_SPU ? NpActor::getShapeManager(*cache->actor).findSceneQueryData(*static_cast<NpShape*>(cache->shape)) : NULL;

	#if !PX_IS_SPU
		// this function is logically const for the SDK user, as flushUpdates() will not have an API-visible effect on this object
		// internally however, flushUpdates() changes the states of the Pruners in mSceneQueryManager
		// because here is the only place we need this, const_cast instead of making SQM mutable
		const_cast<NpSceneQueries*>(this)->mSceneQueryManager.flushUpdates();
	#endif

	#if PX_SUPPORT_VISUAL_DEBUGGER
	CapturePvdOnReturn<HitType> pvdCapture(this, input, hitFlags, cache, filterData, filterCall, bfd, hits);
	#endif

	IssueCallbacksOnReturn<HitType> cbr(hits); // destructor will execute callbacks on return from this function
	hits.hasBlock = false;
	hits.nbTouches = 0;

	PxReal shrunkDistance = HitTypeSupport<HitType>::IsOverlap ? PX_MAX_REAL : input.maxDistance; // can be progressively shrunk as we go over the list of shapes
	if (HitTypeSupport<HitType>::IsSweep)
		shrunkDistance = PxMin(shrunkDistance, PX_MAX_SWEEP_DISTANCE);
	MultiQueryCallback<HitType> pcb(*this, input, anyHit, hits, hitFlags, filterData, filterCall, shrunkDistance, bfd);

#if PX_IS_SPU
	PX_UNUSED(cacheData);
	const Sq::AABBPruner* staticPruner = gSpuContext->staticPruner;
	const Sq::AABBPruner* dynamicPruner = gSpuContext->dynamicPruner;
#else
	if (cacheData && hits.maxNbTouches == 0) // don't use cache for queries that can return touch hits
	{
		// this block is only executed for single shape cache
		cacheData = populate(cacheData);
		const PrunerPayload& cachedPayload = mSceneQueryManager.getPayload(cacheData);
		pcb.isCached = true;
		PxReal dummyDist;
	
		PxAgain againAfterCache;
		if(HitTypeSupport<HitType>::IsSweep)
		{
			// AP: for sweeps we cache the bounds because we need to know them for the test to clip the sweep to bounds
			// otherwise GJK becomes unstable. The bounds can be used multiple times so this is an optimization.
			ShapeData sd(*input.geometry, *input.pose, input.inflation);
			pcb.queryShapeBounds = sd.getPrunerInflatedWorldAABB();
			pcb.queryShapeBoundsValid = true;
			pcb.shapeData = &sd;
			againAfterCache = pcb.invoke(dummyDist, &cachedPayload, 1);
			pcb.shapeData = NULL;
		} else
			againAfterCache = pcb.invoke(dummyDist, &cachedPayload, 1);
		pcb.isCached = false;
		if (!againAfterCache) // if PxAgain result for cached shape was false (abort query), return here
			return hits.hasAnyHits();
	}

	Pruner* staticPruner = PX_IS_SPU ? (Pruner*)gSpuContext->staticPruner : (Pruner*)mSceneQueryManager.getStaticPruner();
	Pruner* dynamicPruner = PX_IS_SPU ? (Pruner*)gSpuContext->dynamicPruner : (Pruner*)mSceneQueryManager.getDynamicPruner();
#endif

	const PxU32 doStatics = filterData.flags & PxQueryFlag::eSTATIC;
	const PxU32 doDynamics = filterData.flags & PxQueryFlag::eDYNAMIC;

	if (HitTypeSupport<HitType>::IsRaycast)
	{
		bool again = doStatics ? staticPruner->raycast(input.getOrigin(), input.getDir(), pcb.shrunkDistance, pcb) : true;
		if (!again)
			return hits.hasAnyHits();
		
		if (doDynamics)
			again = dynamicPruner->raycast(input.getOrigin(), input.getDir(), pcb.shrunkDistance, pcb);

		cbr.again = again; // update the status to avoid duplicate processTouches()
		return hits.hasAnyHits();
	}
	else if (HitTypeSupport<HitType>::IsOverlap)
	{
		PX_ASSERT(input.geometry);

		ShapeData sd(*input.geometry,*input.pose,input.inflation);
		pcb.shapeData = &sd;
		PxAgain again = doStatics ? staticPruner->overlap(sd, pcb) : true;
		if (!again) // && (filterData.flags & PxQueryFlag::eANY_HIT))
			return hits.hasAnyHits();
		
		if (doDynamics)
			again = dynamicPruner->overlap(sd, pcb);
		
		cbr.again = again; // update the status to avoid duplicate processTouches()
		return hits.hasAnyHits();
	}
	else
	{
		PX_ASSERT(HitTypeSupport<HitType>::IsSweep);
		PX_ASSERT(input.geometry);


		//pxPrintf("multiQ sweep static, pcbnhits=%d, sweepFunc=%x\n", pcb.numTotalHits, *(PxU32*)(&staticPruner->mSweepOBBFunc));
		ShapeData sd(*input.geometry, *input.pose, input.inflation);
		pcb.queryShapeBounds = sd.getPrunerInflatedWorldAABB();
		pcb.queryShapeBoundsValid = true;
		pcb.shapeData = &sd;
		PxAgain again = doStatics ? staticPruner->sweep(sd, input.getDir(), pcb.shrunkDistance, pcb) : true;
		if (!again)
			return hits.hasAnyHits();
		
		//pxPrintf("multiQ sweep dynamic\n");
		if (doDynamics)
			again = dynamicPruner->sweep(sd, input.getDir(), pcb.shrunkDistance, pcb);
		
		cbr.again = again; // update the status to avoid duplicate processTouches()
		return hits.hasAnyHits();
	}
}


// explicit instantiations for multiQuery to fix link errors on android
#if !(PX_IS_WINDOWS | PX_IS_X360 | PX_IS_SPU)
#define TMQ(hittype) \
	template bool NpSceneQueries::multiQuery<hittype>( \
		const MultiQueryInput& input, PxHitCallback<hittype>& hits, PxHitFlags hitFlags, \
		const PxQueryCache* cache, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall, \
		BatchQueryFilterData* bfd) const;

TMQ(PxRaycastHit)
TMQ(PxOverlapHit)
TMQ(PxSweepHit)

#undef TMQ
#endif

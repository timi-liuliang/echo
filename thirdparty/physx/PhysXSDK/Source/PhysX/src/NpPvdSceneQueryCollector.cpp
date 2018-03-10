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

#include "ScbScene.h"
#include "NpScene.h"
#include "PxBatchQueryDesc.h"
#include "NpPvdSceneQueryCollector.h"
#include "PxVisualDebugger.h"
#include "PvdDataStream.h"

#if PX_SUPPORT_VISUAL_DEBUGGER

namespace physx
{
	using namespace Sq;
namespace Pvd
{

PvdSceneQueryCollector::PvdSceneQueryCollector(Scb::Scene& scene, bool isBatched)
: mScene(scene), mInUse(0),mIsBatched(isBatched)
{	
}

PvdSceneQueryCollector::~PvdSceneQueryCollector()
{}

void	PvdSceneQueryCollector::release()
{
	physx::debugger::comm::PvdDataStream* stream = mScene.getSceneVisualDebugger().getPvdDataStream();
	if(stream != NULL && stream->isConnected())
	{
		const Array<PxGeometryHolder>& geoms = getPrevFrameGeometries();
		for (PxU32 k = 0; k < geoms.size(); ++k)
			stream->destroyInstance(&geoms[k]);

		mGeometries[0].clear();
		mGeometries[1].clear();
	}
}

void	PvdSceneQueryCollector::raycastAny (
			const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
			const PxQueryHit* hit,
			bool hasHit,
			PxFilterData filterData,
			PxU32 filterFlags)
{


		PvdRaycast raycastQuery;
		
		raycastQuery.type = QueryID::QUERY_RAYCAST_ANY_OBJECT;
		raycastQuery.origin = origin;
		raycastQuery.unitDir = unitDir;
		raycastQuery.distance = distance;
		raycastQuery.filterData = filterData;
		raycastQuery.filterFlags = filterFlags;

		raycastQuery.hits.arrayName = getArrayName( mPvdSqHits );
		raycastQuery.hits.baseIndex = mPvdSqHits.size();
		raycastQuery.hits.count = (PxU32)hasHit;

		if(hasHit)
		{
			mPvdSqHits.pushBack( PvdSqHit( *hit ) );
		}
	
		mAccumulatedRaycastQueries.pushBack(raycastQuery);
}

void	PvdSceneQueryCollector::raycastSingle (
			const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
			const PxRaycastHit* hit,
			bool hasHit,
			PxFilterData filterData,
			PxU32 filterFlags)
{

		PvdRaycast raycastQuery;
		
		raycastQuery.type = QueryID::QUERY_RAYCAST_CLOSEST_OBJECT;
		raycastQuery.origin = origin;
		raycastQuery.unitDir = unitDir;
		raycastQuery.distance = distance;
		raycastQuery.filterData = filterData;
		raycastQuery.filterFlags = filterFlags;

		raycastQuery.hits.arrayName = getArrayName( mPvdSqHits );
		raycastQuery.hits.baseIndex = mPvdSqHits.size();
		raycastQuery.hits.count = (PxU32)hasHit;

		if(hasHit)
		{
			mPvdSqHits.pushBack( PvdSqHit( *hit ) );
		}
	
		mAccumulatedRaycastQueries.pushBack(raycastQuery);
}

void	PvdSceneQueryCollector::raycastMultiple(
			const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
			const PxRaycastHit* hit,
			PxU32 hitsNum,
			PxFilterData filterData,
			PxU32 filterFlags)
{

		PvdRaycast raycastQuery;
		
		raycastQuery.type = QueryID::QUERY_RAYCAST_ALL_OBJECTS;
		raycastQuery.origin = origin;
		raycastQuery.unitDir = unitDir;
		raycastQuery.distance = distance;
		raycastQuery.filterData = filterData;
		raycastQuery.filterFlags = filterFlags;

		raycastQuery.hits.arrayName = getArrayName( mPvdSqHits );
		raycastQuery.hits.baseIndex = mPvdSqHits.size();
		raycastQuery.hits.count = hitsNum;

		mAccumulatedRaycastQueries.pushBack(raycastQuery);

		if ( PxU32(-1) != hitsNum )
		{
			for(PxU32 i = 0; i < hitsNum; i++ )
			{
				mPvdSqHits.pushBack( PvdSqHit( hit[i] ) );			
			}
		}
}


static PxU32 getSweepType(PxGeometryType::Enum geoType, bool isSingle)
{
	switch(geoType)
	{
	case PxGeometryType::eBOX:
		{
			
			return isSingle ? QueryID::QUERY_LINEAR_OBB_SWEEP_CLOSEST_OBJECT:
				QueryID::QUERY_LINEAR_OBB_SWEEP_ALL_OBJECTS;

		}
	case PxGeometryType::eSPHERE:
	case PxGeometryType::eCAPSULE:
		{
			return isSingle ? QueryID::QUERY_LINEAR_CAPSULE_SWEEP_CLOSEST_OBJECT:
				QueryID::QUERY_LINEAR_CAPSULE_SWEEP_ALL_OBJECTS;
		}
	case PxGeometryType::eCONVEXMESH:
		{
			return isSingle ? QueryID::QUERY_LINEAR_CONVEX_SWEEP_CLOSEST_OBJECT:
				QueryID::QUERY_LINEAR_CONVEX_SWEEP_ALL_OBJECTS;
		}
	case PxGeometryType::ePLANE:
	case PxGeometryType::eTRIANGLEMESH:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		PX_ASSERT(0);
	}
	return 0;
}

void	PvdSceneQueryCollector::sweepAny (
			const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
			const PxQueryHit* hit,
			bool hasHit,
			PxFilterData filterData,
			PxU32 filterFlags)
{	
	
	PvdSweep sweepQuery;

	pushBack( sweepQuery.geometries, geometry );
	pushBack( sweepQuery.poses, pose );
	pushBack( sweepQuery.filterData, filterData );

	sweepQuery.type = getSweepType( geometry.getType(), true );
	sweepQuery.filterFlags = filterFlags;
	sweepQuery.unitDir = unitDir;
	sweepQuery.distance = distance;

	sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
	sweepQuery.hits.baseIndex = mPvdSqHits.size();
	sweepQuery.hits.count = (PxU32)hasHit;

	if(hasHit)
	{
		mPvdSqHits.pushBack( PvdSqHit( *hit ) );
	}

	mAccumulatedSweepQueries.pushBack(sweepQuery);	
}

void	PvdSceneQueryCollector::sweepAny (
			const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
			const PxVec3& unitDir, const PxReal distance,
			const PxQueryHit* hit,
			bool hasHit,
			PxU32 filterFlags)
{
	
	PvdSweep sweepQuery;

	pushBack( sweepQuery.geometries, geometryList, geometryCount );
	pushBack( sweepQuery.poses, poseList, geometryCount );
	pushBack( sweepQuery.filterData, filterDataList, geometryCount );

	sweepQuery.type = QueryID::QUERY_LINEAR_COMPOUND_GEOMETRY_SWEEP_CLOSEST_OBJECT;
	sweepQuery.filterFlags = filterFlags;
	sweepQuery.unitDir = unitDir;
	sweepQuery.distance = distance;

	sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
	sweepQuery.hits.baseIndex = mPvdSqHits.size();
	sweepQuery.hits.count = (PxU32)hasHit;

	if(hasHit)
	{
		mPvdSqHits.pushBack( PvdSqHit( *hit ) );
	}

	mAccumulatedSweepQueries.pushBack(sweepQuery);	
}

void	PvdSceneQueryCollector::sweepSingle (
			const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
			const PxSweepHit* hit,
			bool hasHit,
			PxFilterData filterData,
			PxU32 filterFlags)
{
	
	PvdSweep sweepQuery;

	sweepQuery.type = getSweepType( geometry.getType(), true );
	pushBack( sweepQuery.geometries, geometry );
	pushBack( sweepQuery.poses, pose );
	pushBack( sweepQuery.filterData, filterData );

	sweepQuery.filterFlags = filterFlags;
	sweepQuery.unitDir = unitDir;
	sweepQuery.distance = distance;

	sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
	sweepQuery.hits.baseIndex = mPvdSqHits.size();
	sweepQuery.hits.count = (PxU32)hasHit;

	if(hasHit)
	{
		mPvdSqHits.pushBack( PvdSqHit( *hit ) );
	}

	mAccumulatedSweepQueries.pushBack(sweepQuery);	
}

void	PvdSceneQueryCollector::sweepSingle (
			const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
			const PxVec3& unitDir, const PxReal distance,
			const PxSweepHit* hit,
			bool hasHit,
			PxU32 filterFlags)
{
	
	PvdSweep sweepQuery;
	pushBack( sweepQuery.geometries, geometryList, geometryCount );
	pushBack( sweepQuery.poses, poseList, geometryCount );
	pushBack( sweepQuery.filterData, filterDataList, geometryCount );

	sweepQuery.type = QueryID::QUERY_LINEAR_COMPOUND_GEOMETRY_SWEEP_CLOSEST_OBJECT;
	sweepQuery.filterFlags = filterFlags;
	sweepQuery.unitDir = unitDir;
	sweepQuery.distance = distance;

	sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
	sweepQuery.hits.baseIndex = mPvdSqHits.size();
	sweepQuery.hits.count = (PxU32)hasHit;

	if(hasHit)
	{
		mPvdSqHits.pushBack( PvdSqHit( *hit ) );
	}

	mAccumulatedSweepQueries.pushBack(sweepQuery);
}


void	PvdSceneQueryCollector::sweepMultiple (
			const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
			const PxSweepHit* hit,
			PxU32 hitsNum,
			PxFilterData filterData,
			PxU32 filterFlags)
{
	
	PvdSweep sweepQuery;

	sweepQuery.type = getSweepType( geometry.getType(), true );
	pushBack( sweepQuery.geometries, geometry );
	pushBack( sweepQuery.poses, pose );
	pushBack( sweepQuery.filterData, filterData );

	sweepQuery.filterFlags = filterFlags;
	sweepQuery.unitDir = unitDir;
	sweepQuery.distance = distance;

	sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
	sweepQuery.hits.baseIndex = mPvdSqHits.size();
	sweepQuery.hits.count = hitsNum;

	mAccumulatedSweepQueries.pushBack(sweepQuery);	

	if ( PxU32(-1) != hitsNum )
	{
		for(PxU32 i = 0; i < hitsNum; i++ )
		{
			mPvdSqHits.pushBack( PvdSqHit( hit[i] ) );			
		}
	}
}

void	PvdSceneQueryCollector::sweepMultiple (
			const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
			const PxVec3& unitDir, const PxReal distance,
			const PxSweepHit* hit,
			PxU32 hitsNum,
			PxU32 filterFlags)
{
	
	PvdSweep sweepQuery;

	pushBack( sweepQuery.geometries, geometryList, geometryCount );
	pushBack( sweepQuery.poses, poseList, geometryCount );
	pushBack( sweepQuery.filterData, filterDataList, geometryCount );

	sweepQuery.type = QueryID::QUERY_LINEAR_COMPOUND_GEOMETRY_SWEEP_ALL_OBJECTS;
	sweepQuery.filterFlags = filterFlags;
	sweepQuery.unitDir = unitDir;
	sweepQuery.distance = distance;

	sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
	sweepQuery.hits.baseIndex = mPvdSqHits.size();
	sweepQuery.hits.count = hitsNum;

	mAccumulatedSweepQueries.pushBack(sweepQuery);	

	if ( PxU32(-1) != hitsNum )
	{
		for(PxU32 i = 0; i < hitsNum; i++ )
		{
			mPvdSqHits.pushBack( PvdSqHit( hit[i] ) );			
		}
	}
}

static PxU32 getOverlapType(PxGeometryType::Enum geoType, const PxQuat& rot)
{
	switch(geoType)
	{
	case PxGeometryType::eBOX:
		{
			if ((rot.x != 0) || (rot.y != 0) || (rot.z != 0) || (rot.w != 1))
				return QueryID::QUERY_OVERLAP_OBB_ALL_OBJECTS;
			else
				return QueryID::QUERY_OVERLAP_AABB_ALL_OBJECTS;
		}
	case PxGeometryType::eSPHERE:
		return QueryID::QUERY_OVERLAP_SPHERE_ALL_OBJECTS;
	case PxGeometryType::eCAPSULE:
		return QueryID::QUERY_OVERLAP_CAPSULE_ALL_OBJECTS;
	case PxGeometryType::eCONVEXMESH:
		return QueryID::QUERY_OVERLAP_CONVEX_ALL_OBJECTS;
	case PxGeometryType::ePLANE:
	case PxGeometryType::eTRIANGLEMESH:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		PX_ASSERT(0);
	}
	return 0;
}

void	PvdSceneQueryCollector::overlapMultiple(
			const PxGeometry& geometry,
			const PxTransform& pose,
			const PxOverlapHit* hit,
			PxU32 hitsNum,
			PxFilterData filterData,
			PxU32 filterFlags)
{
	
	PvdOverlap overlapQuery;

	overlapQuery.type = getOverlapType(geometry.getType(), pose.q);
	pushBack( overlapQuery.geometries, geometry );
	overlapQuery.pose = pose;
	overlapQuery.filterData = filterData;
	overlapQuery.filterFlags = filterFlags;

	overlapQuery.hits.arrayName = getArrayName( mPvdSqHits );
	overlapQuery.hits.baseIndex = mPvdSqHits.size();
	overlapQuery.hits.count = hitsNum;

	mAccumulatedOverlapQueries.pushBack(overlapQuery);	

	if ( PxU32(-1) != hitsNum )
	{
		for(PxU32 i = 0; i < hitsNum; i++ )
		{
			mPvdSqHits.pushBack( PvdSqHit( hit[i] ) );			
		}
	}
}

void	PvdSceneQueryCollector::collectBatchedRaycastHits(
			const PxRaycastQueryResult* pResults, 
			PxU32 nbRaycastResults, 
			PxU32 batchedRayQstartIdx 
			)
{
	for( PxU32 i = 0; i < nbRaycastResults; i++ )
	{
		
		const PxRaycastQueryResult& raycastResult = pResults[i];
		PvdRaycast& raycastQuery = mAccumulatedRaycastQueries[batchedRayQstartIdx+i];

		if( raycastResult.queryStatus == PxBatchQueryStatus::eSUCCESS &&
			raycastQuery.hits.count != raycastResult.getNbAnyHits())
		{			
			raycastQuery.hits.arrayName = getArrayName( mPvdSqHits );
			raycastQuery.hits.baseIndex = mPvdSqHits.size();
			raycastQuery.hits.count = raycastResult.getNbAnyHits();
			//Maybe this is a raycastAny which do not return hits buffer
			for( PxU32 j = 0; j < raycastResult.getNbAnyHits(); j++)
				mPvdSqHits.pushBack( PvdSqHit( raycastResult.getAnyHit(j) ) );
		}

	}
}

void	PvdSceneQueryCollector::collectBatchedOverlapHits(
			const PxOverlapQueryResult* pResults, 
			PxU32 nbOverlapResults, 
			PxU32 batchedOverlapQstartIdx 
			)
{
	for( PxU32 i = 0; i < nbOverlapResults; i++ )
	{		
		const PxOverlapQueryResult& overlapResult = pResults[i];
		if( overlapResult.queryStatus != PxBatchQueryStatus::eSUCCESS )
			continue;

		PvdOverlap& overlapQuery = mAccumulatedOverlapQueries[batchedOverlapQstartIdx+i];
		
		if( overlapQuery.hits.count != overlapResult.getNbAnyHits() )
		{			
			overlapQuery.hits.arrayName = getArrayName( mPvdSqHits );
			overlapQuery.hits.baseIndex = mPvdSqHits.size();
			overlapQuery.hits.count = overlapResult.getNbAnyHits();
			for( PxU32 j = 0; j < overlapResult.getNbAnyHits(); j++)
				mPvdSqHits.pushBack( PvdSqHit( overlapResult.getAnyHit(j) ) );
		}

	}
}

void	PvdSceneQueryCollector::collectBatchedSweepHits(
			const PxSweepQueryResult* pResults, 
			PxU32 nbSweepResults, 
			PxU32 batchedSweepQstartIdx 
			)
{
	for( PxU32 i = 0; i < nbSweepResults; i++ )
	{		
		const PxSweepQueryResult& sweepResult = pResults[i];
		PvdSweep& sweepQuery = mAccumulatedSweepQueries[batchedSweepQstartIdx+i];

		if( sweepResult.queryStatus == PxBatchQueryStatus::eSUCCESS &&
			sweepQuery.hits.count != sweepResult.getNbAnyHits() )
		{			
			sweepQuery.hits.arrayName = getArrayName( mPvdSqHits );
			sweepQuery.hits.baseIndex = mPvdSqHits.size();
			sweepQuery.hits.count = sweepResult.getNbAnyHits();
			for( PxU32 j = 0; j < sweepResult.getNbAnyHits(); j++)
				mPvdSqHits.pushBack( PvdSqHit( sweepResult.getAnyHit(j) ) );
		}

	}
}

void	PvdSceneQueryCollector::raycastAnyWithLock(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
						const PxQueryHit* hit,
						bool hasHit,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	raycastAny(origin, unitDir, distance, hit, hasHit, filterData, filterFlags);
}

void	PvdSceneQueryCollector::raycastSingleWithLock(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
						const PxRaycastHit* hit,
						bool hasHit,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	raycastSingle(origin, unitDir, distance, hit, hasHit, filterData, filterFlags);
}

void	PvdSceneQueryCollector::raycastMultipleWithLock(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
						const PxRaycastHit* hit,
						PxU32 hitsNum,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	raycastMultiple(origin, unitDir, distance, hit, hitsNum, filterData, filterFlags);
}

void	PvdSceneQueryCollector::sweepAnyWithLock(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
						const PxQueryHit* hit,
						bool hasHit,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	sweepAny(geometry, pose, unitDir, distance, hit, hasHit, filterData, filterFlags);
}

void	PvdSceneQueryCollector::sweepAnyWithLock(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
						const PxVec3& unitDir, const PxReal distance,
						const PxQueryHit* hit,
						bool hasHit,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	sweepAny(geometryList, poseList, filterDataList, geometryCount, unitDir, distance, hit, hasHit, filterFlags);
}

void	PvdSceneQueryCollector::sweepSingleWithLock(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
						const PxSweepHit* hit,
						bool hasHit,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	sweepSingle(geometry, pose, unitDir, distance, hit, hasHit, filterData, filterFlags);
}

void	PvdSceneQueryCollector::sweepSingleWithLock(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
						const PxVec3& unitDir, const PxReal distance,
						const PxSweepHit* hit,
						bool hasHit,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	sweepSingle(geometryList, poseList, filterDataList, geometryCount, unitDir, distance, hit, hasHit, filterFlags);
}


void	PvdSceneQueryCollector::sweepMultipleWithLock(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
						const PxSweepHit* hit,
						PxU32 hitsNum,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	sweepMultiple(geometry, pose, unitDir, distance, hit, hitsNum, filterData, filterFlags);
}

void	PvdSceneQueryCollector::sweepMultipleWithLock(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
						const PxVec3& unitDir, const PxReal distance,
						const PxSweepHit* hit,
						PxU32 hitsNum,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	sweepMultiple(geometryList, poseList, filterDataList, geometryCount, unitDir, distance, hit, hitsNum, filterFlags);
}

void	PvdSceneQueryCollector::overlapMultipleWithLock(const PxGeometry& geometry,
						const PxTransform& pose,
						const PxOverlapHit* hit,
						PxU32 hitsNum,
						PxFilterData filterData,
						PxU32 filterFlags)
{
	Ps::Mutex::ScopedLock lock(mMutex);
	overlapMultiple(geometry, pose, hit, hitsNum, filterData, filterFlags);
}

void	PvdSceneQueryCollector::pushBack( PvdReference& ref, const PxTransform& pose )
{
	ref.arrayName = getArrayName( mPoses );
	ref.baseIndex = mPoses.size();
	ref.count = 1;

	mPoses.pushBack( pose );
}

void	PvdSceneQueryCollector::pushBack( PvdReference& ref, const PxTransform* poseList, PxU32 count )
{
	ref.arrayName = getArrayName( mPoses );
	ref.baseIndex = mPoses.size();
	ref.count = count;

	for (PxU32 i = 0; i < count; ++i)
		mPoses.pushBack( poseList[i] );
}

void	PvdSceneQueryCollector::pushBack( PvdReference& ref, const PxFilterData& filterData )
{
	ref.arrayName = getArrayName( mFilterData );
	ref.baseIndex = mFilterData.size();
	ref.count = 1;

	mFilterData.pushBack( filterData );
}

void	PvdSceneQueryCollector::pushBack( PvdReference& ref, const PxFilterData* filterDataList, PxU32 count )
{
	ref.arrayName = getArrayName( mFilterData );
	ref.baseIndex = mFilterData.size();
	ref.count = count;

	for (PxU32 i = 0; i < count; ++i)
		mFilterData.pushBack( (filterDataList == NULL)?
		PxFilterData():
		filterDataList[i] );
}

void	PvdSceneQueryCollector::pushBack( PvdReference& ref, const PxGeometry& geometry )
{
	ref.arrayName = getArrayName( mGeometries[0] );
	ref.baseIndex = mGeometries[mInUse].size();
	ref.count = 1;

	doPushBack( geometry );
}

void	PvdSceneQueryCollector::pushBack( PvdReference& ref, const PxGeometry** geometryList, PxU32 count )
{
	ref.arrayName = getArrayName( mGeometries[mInUse] );
	ref.baseIndex = mGeometries[mInUse].size();
	ref.count = count;

	for (PxU32 i = 0; i < count; ++i)
		doPushBack( *geometryList[i] );
}

void	PvdSceneQueryCollector::doPushBack( const PxGeometry& geometry )
{
	PxGeometryHolder	holder;
	holder.any() = geometry;
	switch ( geometry.getType() )
	{
	case PxGeometryType::eBOX:
		holder.box() = static_cast<const PxBoxGeometry&>( geometry );
		break;
	case PxGeometryType::eSPHERE:
		holder.sphere() = static_cast<const PxSphereGeometry&>( geometry );
		break;
	case PxGeometryType::eCAPSULE:
		holder.capsule() = static_cast<const PxCapsuleGeometry&>( geometry );
		break;
	case PxGeometryType::eCONVEXMESH:
		holder.convexMesh() = static_cast<const PxConvexMeshGeometry&>( geometry );
		break;
	case PxGeometryType::ePLANE:
	case PxGeometryType::eTRIANGLEMESH:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		PX_ALWAYS_ASSERT_MESSAGE("unsupported geometry type" );
		break;
	}

	mGeometries[mInUse].pushBack( holder );
}

}
}
#endif

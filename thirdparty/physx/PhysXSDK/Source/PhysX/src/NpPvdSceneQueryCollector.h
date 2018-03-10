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

#ifndef NP_PVD_SCENEQUERYCOLLECTOR_H
#define NP_PVD_SCENEQUERYCOLLECTOR_H

#include "PsArray.h"
#include "PxFiltering.h"
#include "PxGeometryHelpers.h"
#include "PxQueryReport.h"
#include "PxQueryFiltering.h"
#include "PxBatchQueryDesc.h"

#if PX_SUPPORT_VISUAL_DEBUGGER

namespace physx
{
class	PxScene;
struct	BatchQueryStream;

namespace Scb
{
	class Scene;
}
	
namespace Pvd
{
	struct PvdReference
	{
		const char*				arrayName;
		PxU32					baseIndex;
		PxU32					count;
	};

	struct PvdRaycast
	{
		PxU32					type;
		PxFilterData			filterData;					
		PxU32					filterFlags;
		PxVec3					origin;
		PxVec3					unitDir;
		PxReal					distance;
		PvdReference			hits;
	};

	struct PvdOverlap
	{
		PxU32					type;
		PxFilterData			filterData;					
		PxU32					filterFlags;
		PxTransform				pose;
		PvdReference			geometries;
		PvdReference			hits;
	};

	struct PvdSweep
	{
		PxU32					type;
		PxU32					filterFlags;
		PxVec3					unitDir;
		PxReal					distance;
		PvdReference			geometries;
		PvdReference			poses;
		PvdReference			filterData;
		PvdReference			hits;
	};

	struct PvdSqHit
	{
		const void*	shape;
		const void*	actor;
		PxU32		faceIndex;
		PxU32		flags;

		PxVec3		impact;
		PxVec3		normal;
		PxF32		distance;

		PxF32		u;
		PxF32		v;

		PxU32		sweepGeometryIndex;

		PvdSqHit() { setDefaults( PxQueryHit() ); }

		explicit PvdSqHit( const PxQueryHit& hit )
		{
			setDefaults( hit );
		}

		explicit PvdSqHit( const PxRaycastHit& hit )
		{
			setDefaults( hit );

			impact = hit.position;
			normal = hit.normal;
			distance = hit.distance;

			u = hit.u;
			v = hit.v;
		}

		explicit PvdSqHit( const PxSweepHit& hit )
		{
			setDefaults( hit );

			impact = hit.position;
			normal = hit.normal;
			distance = hit.distance;

			sweepGeometryIndex = 0; // unused
		}

	private:
		void setDefaults( const PxQueryHit& hit )
		{
			shape = hit.shape;
			actor = hit.actor;
			faceIndex = hit.faceIndex;
			flags = 0;

			impact = normal = PxVec3(0.0f);
			distance = u = v = 0.0f;
			sweepGeometryIndex = 0;
		}
	};

	template <typename T, bool isBatched> inline const char* PvdGetArrayName()			{ return T::template getArrayName<isBatched>(); }
	template <>           inline const char* PvdGetArrayName<PxGeometryHolder,false> () { return "SceneQueries.GeometryList"; }
	template <>           inline const char* PvdGetArrayName<PxTransform,false> ()		{ return "SceneQueries.PoseList"; }
	template <>           inline const char* PvdGetArrayName<PxFilterData,false> ()		{ return "SceneQueries.FilterDataList"; }
	template <>           inline const char* PvdGetArrayName<PvdRaycast,false> ()		{ return "SceneQueries.Raycasts"; }
	template <>           inline const char* PvdGetArrayName<PvdOverlap,false> ()		{ return "SceneQueries.Overlaps"; }
	template <>           inline const char* PvdGetArrayName<PvdSweep,false> ()			{ return "SceneQueries.Sweeps"; }
	template <>           inline const char* PvdGetArrayName<PvdSqHit,false> ()			{ return "SceneQueries.Hits"; }
	template <>           inline const char* PvdGetArrayName<PxGeometryHolder,true> ()	{ return "BatchedQueries.GeometryList"; }
	template <>           inline const char* PvdGetArrayName<PxTransform,true> ()		{ return "BatchedQueries.PoseList"; }
	template <>           inline const char* PvdGetArrayName<PxFilterData,true> ()		{ return "BatchedQueries.FilterDataList"; }
	template <>           inline const char* PvdGetArrayName<PvdRaycast,true> ()		{ return "BatchedQueries.Raycasts"; }
	template <>           inline const char* PvdGetArrayName<PvdOverlap,true> ()		{ return "BatchedQueries.Overlaps"; }
	template <>           inline const char* PvdGetArrayName<PvdSweep,true> ()			{ return "BatchedQueries.Sweeps"; }
	template <>           inline const char* PvdGetArrayName<PvdSqHit,true> ()			{ return "BatchedQueries.Hits"; }


	class PvdSceneQueryCollector
	{
		PX_NOCOPY(PvdSceneQueryCollector)
	public:
		PvdSceneQueryCollector(Scb::Scene& scene, bool isBatched);
		~PvdSceneQueryCollector();

		void	clear()
		{
			Ps::Mutex::ScopedLock lock(mMutex);

			mAccumulatedRaycastQueries.clear();
			mAccumulatedOverlapQueries.clear();
			mAccumulatedSweepQueries.clear();
			mPvdSqHits.clear();
			mPoses.clear();
			mFilterData.clear();
		}

		void	clearGeometryArrays()
		{
			mGeometries[0].clear();
			mGeometries[1].clear();
		}

		void	release();

		void	raycastAny		(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								const PxQueryHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	raycastSingle	(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								const PxRaycastHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	raycastMultiple(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								const PxRaycastHit* hit,
								PxU32 hitsNum,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepAny		(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								const PxQueryHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepAny		(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
								const PxVec3& unitDir, const PxReal distance,
								const PxQueryHit* hit,
								bool hasHit,
								PxU32 filterFlags);

		void	sweepSingle		(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepSingle		(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
								const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								bool hasHit,
								PxU32 filterFlags);


		void	sweepMultiple	(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								PxU32 hitsNum,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepMultiple	(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
								const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								PxU32 hitsNum,
								PxU32 filterFlags);

		void	overlapMultiple(const PxGeometry& geometry,
								const PxTransform& pose,
								const PxOverlapHit* hit,
								PxU32 hitsNum,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	raycastAnyWithLock(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								const PxQueryHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	raycastSingleWithLock(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								const PxRaycastHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	raycastMultipleWithLock(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								const PxRaycastHit* hit,
								PxU32 hitsNum,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepAnyWithLock(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								const PxQueryHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepAnyWithLock(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
								const PxVec3& unitDir, const PxReal distance,
								const PxQueryHit* hit,
								bool hasHit,
								PxU32 filterFlags);

		void	sweepSingleWithLock(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								bool hasHit,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepSingleWithLock(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
								const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								bool hasHit,
								PxU32 filterFlags);


		void	sweepMultipleWithLock(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								PxU32 hitsNum,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	sweepMultipleWithLock(const PxGeometry** geometryList, const PxTransform* poseList, const PxFilterData* filterDataList, PxU32 geometryCount, 
								const PxVec3& unitDir, const PxReal distance,
								const PxSweepHit* hit,
								PxU32 hitsNum,
								PxU32 filterFlags);

		void	overlapMultipleWithLock(const PxGeometry& geometry,
								const PxTransform& pose,
								const PxOverlapHit* hit,
								PxU32 hitsNum,
								PxFilterData filterData,
								PxU32 filterFlags);

		void	collectBatchedRaycastHits(
								const PxRaycastQueryResult* pResults, 
								PxU32 nbRaycastResults, 
								PxU32 batchedRayQstartIdx 
								);

		void	collectBatchedOverlapHits(
								const PxOverlapQueryResult* pResults, 
								PxU32 nbOverlapResults, 
								PxU32 batchedOverlapQstartIdx 
								);

		void	collectBatchedSweepHits(
								const PxSweepQueryResult* pResults, 
								PxU32 nbSweepResults, 
								PxU32 batchedSweepQstartIdx 
								);

		Ps::Mutex&				getLock() { return mMutex; }

	public:
		const Array<PxGeometryHolder>&	getCurrentFrameGeometries() const	{ return mGeometries[mInUse]; }
		const Array<PxGeometryHolder>&	getPrevFrameGeometries() const		{ return mGeometries[mInUse ^ 1]; }
		void							prepareNextFrameGeometries()		{ mInUse ^= 1; mGeometries[mInUse].clear(); }
		template <typename T>
		const char*				getArrayName( const Array<T>& ) { return mIsBatched?PvdGetArrayName<T, 1>():PvdGetArrayName<T, 0>(); }

	protected:
		void					pushBack( PvdReference& ref, const PxGeometry& geometry );
		void					pushBack( PvdReference& ref, const PxGeometry** geometryList, PxU32 geometryCount );
		void					pushBack( PvdReference& ref, const PxTransform& pose );
		void					pushBack( PvdReference& ref, const PxTransform* poseList, PxU32 count );
		void					pushBack( PvdReference& ref, const PxFilterData& filterData );
		void					pushBack( PvdReference& ref, const PxFilterData* filterDataList, PxU32 count );

		void					doPushBack( const PxGeometry& geometry );

	public:
		//Scene query and hits for pvd, collected in current frame
		Array<PvdRaycast>		mAccumulatedRaycastQueries;
		Array<PvdSweep>			mAccumulatedSweepQueries;
		Array<PvdOverlap>		mAccumulatedOverlapQueries;
		Array<PvdSqHit>			mPvdSqHits;
		Array<PxTransform>		mPoses;
		Array<PxFilterData>		mFilterData;

	private:
		Scb::Scene&				mScene;
		Ps::Mutex				mMutex;
		Array<PxGeometryHolder>	mGeometries[2];
		PxU32					mInUse;
		bool					mIsBatched;
	};
}
}

#define IS_PVD_SQ_ENABLED \
	(static_cast<Pvd::VisualDebugger*>(NpPhysics::getInstance().getVisualDebugger()) ? \
	static_cast<Pvd::VisualDebugger*>(NpPhysics::getInstance().getVisualDebugger())->getTransmitSceneQueriesFlag() : false)

#endif //PX_SUPPORT_VISUAL_DEBUGGER

#endif //NP_PVD_SCENEQUERYCOLLECTOR_H

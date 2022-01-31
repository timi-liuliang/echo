#include "physx_vehicle_scene_query.h"

namespace physx
{
	PxQueryHitType::Enum PxWheelSceneQueryPreFilterBlocking(PxFilterData filterData0, PxFilterData filterData1, const void* constantBlock, PxU32 constantBlockSize, PxHitFlags& queryFlags)
	{
		//filterData0 is the vehicle suspension query.
		//filterData1 is the shape potentially hit by the query.
		PX_UNUSED(filterData0);
		PX_UNUSED(constantBlock);
		PX_UNUSED(constantBlockSize);
		PX_UNUSED(queryFlags);
		return ((0 == (filterData1.word3 & DRIVABLE_SURFACE)) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK);
	}

	//Drivable surface types.
	enum
	{
		SURFACE_TYPE_TARMAC,
		MAX_NUM_SURFACE_TYPES
	};

	//Tire types.
	enum
	{
		TIRE_TYPE_NORMAL = 0,
		TIRE_TYPE_WORN,
		MAX_NUM_TIRE_TYPES
	};

	PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const PxMaterial* defaultMaterial)
	{
		//Tire model friction for each combination of drivable surface type and tire type.
		static PxF32 TireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES] =
		{
			//NORMAL,	WORN
			{1.00f,		0.1f}//TARMAC
		};

		PxVehicleDrivableSurfaceType surfaceTypes[1];
		surfaceTypes[0].mType = SURFACE_TYPE_TARMAC;

		const PxMaterial* surfaceMaterials[1];
		surfaceMaterials[0] = defaultMaterial;

		PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceTirePairs = PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES);

		surfaceTirePairs->setup(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES, surfaceMaterials, surfaceTypes);

		for (PxU32 i = 0; i < MAX_NUM_SURFACE_TYPES; i++)
		{
			for (PxU32 j = 0; j < MAX_NUM_TIRE_TYPES; j++)
			{
				surfaceTirePairs->setTypePairFriction(i, j, TireFrictionMultipliers[i][j]);
			}
		}

		return surfaceTirePairs;
	}

	PxVehicleSceneQueryData::PxVehicleSceneQueryData()
		: mNumQueriesPerBatch(0)
		, mNumHitResultsPerQuery(0)
		, mRaycastResults(NULL)
		, mRaycastHitBuffer(NULL)
		, mPreFilterShader(NULL)
		, mPostFilterShader(NULL)
	{
	}

	PxVehicleSceneQueryData::~PxVehicleSceneQueryData()
	{
	}

	PxVehicleSceneQueryData* PxVehicleSceneQueryData::allocate(
		const PxU32 maxNumVehicles, const PxU32 maxNumWheelsPerVehicle, const PxU32 maxNumHitPointsPerWheel, const PxU32 numVehiclesInBatch,
		PxBatchQueryPreFilterShader preFilterShader, PxBatchQueryPostFilterShader postFilterShader,
		PxAllocatorCallback& allocator)
	{
		const PxU32 sqDataSize = ((sizeof(PxVehicleSceneQueryData) + 15) & ~15);

		const PxU32 maxNumWheels = maxNumVehicles * maxNumWheelsPerVehicle;
		const PxU32 raycastResultSize = ((sizeof(PxRaycastQueryResult) * maxNumWheels + 15) & ~15);
		const PxU32 sweepResultSize = ((sizeof(PxSweepQueryResult) * maxNumWheels + 15) & ~15);

		const PxU32 maxNumHitPoints = maxNumWheels * maxNumHitPointsPerWheel;
		const PxU32 raycastHitSize = ((sizeof(PxRaycastHit) * maxNumHitPoints + 15) & ~15);
		const PxU32 sweepHitSize = ((sizeof(PxSweepHit) * maxNumHitPoints + 15) & ~15);

		const PxU32 size = sqDataSize + raycastResultSize + raycastHitSize + sweepResultSize + sweepHitSize;
		PxU8* buffer = static_cast<PxU8*>(allocator.allocate(size, NULL, NULL, 0));

		PxVehicleSceneQueryData* sqData = new(buffer) PxVehicleSceneQueryData();
		sqData->mNumQueriesPerBatch = numVehiclesInBatch * maxNumWheelsPerVehicle;
		sqData->mNumHitResultsPerQuery = maxNumHitPointsPerWheel;
		buffer += sqDataSize;

		sqData->mRaycastResults = reinterpret_cast<PxRaycastQueryResult*>(buffer);
		buffer += raycastResultSize;

		sqData->mRaycastHitBuffer = reinterpret_cast<PxRaycastHit*>(buffer);
		buffer += raycastHitSize;

		sqData->mSweepResults = reinterpret_cast<PxSweepQueryResult*>(buffer);
		buffer += sweepResultSize;

		sqData->mSweepHitBuffer = reinterpret_cast<PxSweepHit*>(buffer);
		buffer += sweepHitSize;

		for (PxU32 i = 0; i < maxNumWheels; i++)
		{
			new(sqData->mRaycastResults + i) PxRaycastQueryResult();
			new(sqData->mSweepResults + i) PxSweepQueryResult();
		}

		for (PxU32 i = 0; i < maxNumHitPoints; i++)
		{
			new(sqData->mRaycastHitBuffer + i) PxRaycastHit();
			new(sqData->mSweepHitBuffer + i) PxSweepHit();
		}

		sqData->mPreFilterShader = preFilterShader;
		sqData->mPostFilterShader = postFilterShader;

		return sqData;
	}

	void PxVehicleSceneQueryData::free(PxAllocatorCallback& allocator)
	{
		allocator.deallocate(this);
	}

	PxBatchQuery* PxVehicleSceneQueryData::setUpBatchedSceneQuery(const PxU32 batchId, const PxVehicleSceneQueryData& vehicleSceneQueryData, PxScene* scene)
	{
		const PxU32 maxNumQueriesInBatch = vehicleSceneQueryData.mNumQueriesPerBatch;
		const PxU32 maxNumHitResultsInBatch = vehicleSceneQueryData.mNumQueriesPerBatch * vehicleSceneQueryData.mNumHitResultsPerQuery;

		PxBatchQueryDesc sqDesc(maxNumQueriesInBatch, maxNumQueriesInBatch, 0);

		sqDesc.queryMemory.userRaycastResultBuffer = vehicleSceneQueryData.mRaycastResults + batchId * maxNumQueriesInBatch;
		sqDesc.queryMemory.userRaycastTouchBuffer = vehicleSceneQueryData.mRaycastHitBuffer + batchId * maxNumHitResultsInBatch;
		sqDesc.queryMemory.raycastTouchBufferSize = maxNumHitResultsInBatch;

		sqDesc.queryMemory.userSweepResultBuffer = vehicleSceneQueryData.mSweepResults + batchId * maxNumQueriesInBatch;
		sqDesc.queryMemory.userSweepTouchBuffer = vehicleSceneQueryData.mSweepHitBuffer + batchId * maxNumHitResultsInBatch;
		sqDesc.queryMemory.sweepTouchBufferSize = maxNumHitResultsInBatch;

		sqDesc.preFilterShader = vehicleSceneQueryData.mPreFilterShader;

		sqDesc.postFilterShader = vehicleSceneQueryData.mPostFilterShader;

		return scene->createBatchQuery(sqDesc);
	}

	PxRaycastQueryResult* PxVehicleSceneQueryData::getRaycastQueryResultBuffer(const PxU32 batchId)
	{
		return (mRaycastResults + batchId * mNumQueriesPerBatch);
	}

	PxSweepQueryResult* PxVehicleSceneQueryData::getSweepQueryResultBuffer(const PxU32 batchId)
	{
		return (mSweepResults + batchId * mNumQueriesPerBatch);
	}

	PxU32 PxVehicleSceneQueryData::getQueryResultBufferSize() const
	{
		return mNumQueriesPerBatch;
	}
}
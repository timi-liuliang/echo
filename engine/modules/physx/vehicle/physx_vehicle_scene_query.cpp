#include "physx_vehicle_scene_query.h"

namespace physx
{
	PhysxVehicleSceneQueryData::PhysxVehicleSceneQueryData()
		: mNumQueriesPerBatch(0)
		, mNumHitResultsPerQuery(0)
		, mRaycastResults(NULL)
		, mRaycastHitBuffer(NULL)
		, mPreFilterShader(NULL)
		, mPostFilterShader(NULL)
	{
	}

	PhysxVehicleSceneQueryData::~PhysxVehicleSceneQueryData()
	{
	}

	PhysxVehicleSceneQueryData* PhysxVehicleSceneQueryData::allocate(
		const PxU32 maxNumVehicles, const PxU32 maxNumWheelsPerVehicle, const PxU32 maxNumHitPointsPerWheel, const PxU32 numVehiclesInBatch,
		PxBatchQueryPreFilterShader preFilterShader, PxBatchQueryPostFilterShader postFilterShader,
		PxAllocatorCallback& allocator)
	{
		const PxU32 sqDataSize = ((sizeof(PhysxVehicleSceneQueryData) + 15) & ~15);

		const PxU32 maxNumWheels = maxNumVehicles * maxNumWheelsPerVehicle;
		const PxU32 raycastResultSize = ((sizeof(PxRaycastQueryResult) * maxNumWheels + 15) & ~15);
		const PxU32 sweepResultSize = ((sizeof(PxSweepQueryResult) * maxNumWheels + 15) & ~15);

		const PxU32 maxNumHitPoints = maxNumWheels * maxNumHitPointsPerWheel;
		const PxU32 raycastHitSize = ((sizeof(PxRaycastHit) * maxNumHitPoints + 15) & ~15);
		const PxU32 sweepHitSize = ((sizeof(PxSweepHit) * maxNumHitPoints + 15) & ~15);

		const PxU32 size = sqDataSize + raycastResultSize + raycastHitSize + sweepResultSize + sweepHitSize;
		PxU8* buffer = static_cast<PxU8*>(allocator.allocate(size, NULL, NULL, 0));

		PhysxVehicleSceneQueryData* sqData = new(buffer) PhysxVehicleSceneQueryData();
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

	void PhysxVehicleSceneQueryData::free(PxAllocatorCallback& allocator)
	{
		allocator.deallocate(this);
	}

	PxBatchQuery* PhysxVehicleSceneQueryData::setUpBatchedSceneQuery(const PxU32 batchId, const PhysxVehicleSceneQueryData& vehicleSceneQueryData, PxScene* scene)
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

	PxRaycastQueryResult* PhysxVehicleSceneQueryData::getRaycastQueryResultBuffer(const PxU32 batchId)
	{
		return (mRaycastResults + batchId * mNumQueriesPerBatch);
	}

	PxSweepQueryResult* PhysxVehicleSceneQueryData::getSweepQueryResultBuffer(const PxU32 batchId)
	{
		return (mSweepResults + batchId * mNumQueriesPerBatch);
	}

	PxU32 PhysxVehicleSceneQueryData::getQueryResultBufferSize() const
	{
		return mNumQueriesPerBatch;
	}
}
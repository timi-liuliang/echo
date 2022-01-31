#pragma once

#include "../physx_base.h"

namespace physx
{
	enum
	{
		DRIVABLE_SURFACE = 0xffff0000,
		UNDRIVABLE_SURFACE = 0x0000ffff
	};

	class PxVehicleSceneQueryData
	{
	public:
		PxVehicleSceneQueryData();
		~PxVehicleSceneQueryData();

		//Allocate scene query data for up to maxNumVehicles and up to maxNumWheelsPerVehicle with numVehiclesInBatch per batch query.
		static PxVehicleSceneQueryData* allocate(
			const PxU32 maxNumVehicles, const PxU32 maxNumWheelsPerVehicle, const PxU32 maxNumHitPointsPerWheel, const PxU32 numVehiclesInBatch,
			PxBatchQueryPreFilterShader preFilterShader, PxBatchQueryPostFilterShader postFilterShader,
			PxAllocatorCallback& allocator
		);

		//Free allocated buffers.
		void free(PxAllocatorCallback& allocator);

		//Create a PxBatchQuery instance that will be used for a single specified batch.
		static PxBatchQuery* setUpBatchedSceneQuery(const PxU32 batchId, const PxVehicleSceneQueryData& vehicleSceneQueryData, PxScene* scene);

		//Return an array of scene query results for a single specified batch.
		PxRaycastQueryResult* getRaycastQueryResultBuffer(const PxU32 batchId);

		//Return an array of scene query results for a single specified batch.
		PxSweepQueryResult* getSweepQueryResultBuffer(const PxU32 batchId);

		//Get the number of scene query results that have been allocated for a single batch.
		PxU32 getQueryResultBufferSize() const;

	private:
		//Number of queries per batch
		PxU32 mNumQueriesPerBatch;

		//Number of hit results per query
		PxU32 mNumHitResultsPerQuery;

		//One result for each wheel.
		PxRaycastQueryResult* mRaycastResults;
		PxSweepQueryResult* mSweepResults;

		//One hit for each wheel.
		PxRaycastHit* mRaycastHitBuffer;
		PxSweepHit* mSweepHitBuffer;

		//Filter shader used to filter drivable and non-drivable surfaces
		PxBatchQueryPreFilterShader mPreFilterShader;

		//Filter shader used to reject hit shapes that initially overlap sweeps.
		PxBatchQueryPostFilterShader mPostFilterShader;
	};

	PxQueryHitType::Enum PxWheelSceneQueryPreFilterBlocking(PxFilterData filterData0, PxFilterData filterData1,const void* constantBlock, PxU32 constantBlockSize,PxHitFlags& queryFlags);

	PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const PxMaterial* defaultMaterial);
}
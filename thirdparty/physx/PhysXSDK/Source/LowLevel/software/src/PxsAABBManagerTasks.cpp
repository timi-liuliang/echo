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

#include "PxsAABBManagerTasks.h"
#include "PxsAABBManagerAux.h"
#include "PxsAABBManager.h"
#include "PxvBroadPhase.h"
#include "PxAssert.h"
#include "CmEventProfiler.h"
#include "PxProfileEventId.h"

#ifdef PX_PS3
#include "CellComputeAABBTask.h"
#include "CellTimerMarker.h"
#endif

using namespace physx;


#ifndef NDEBUG
bool allWorkItemsAccounted(const PxU32* starts, const PxU32* counts, const PxU32 numTasks, const PxU32 numUpdated)
{
	//Add up the count and test it matches the expected value.
	PxU32 totalCount = 0;
	for(PxU32 i = 0; i < numTasks; i++)
	{
		totalCount += counts[i];
	}
	if(numUpdated != totalCount)
	{
		return false;
	}

	//starts[0] must start at zero.
	if(0 != starts[0])
	{
		return false;
	}

	//Make sure they run consecutively.
	for(PxU32 i = 1; i < numTasks; i++)
	{
		if(starts[i] != (starts[i-1] + counts[i-1]))
		{
			return false;
		}
	}

	return true;
}
#endif

template <PxU32 eTASK_UNIT_WORK_SIZE, PxU32 eMAX_NUM_TASKS> void computeTaskWork(const PxU32 numUpdated, const PxU32 numTasks, PxU32* starts, PxU32* counts)
{
	PX_ASSERT(numUpdated>eTASK_UNIT_WORK_SIZE);
	PX_ASSERT(numTasks>1);
	PX_ASSERT(numTasks<=eMAX_NUM_TASKS);

	const PxU32 numBlocks = (numUpdated + (eTASK_UNIT_WORK_SIZE-1))/eTASK_UNIT_WORK_SIZE;
	const PxU32 numBlocksPerTask = numBlocks/numTasks;
	const PxU32 remainderBlocks = numBlocks - numBlocksPerTask*numTasks;

	PxU32 count=0;
	PxU32 lastStart=0;

	//Some tasks don't take on an extra block.
	PxU32 numToProcess=eTASK_UNIT_WORK_SIZE*numBlocksPerTask;
	for(PxU32 i=0;i<(numTasks-remainderBlocks);i++)
	{
		starts[i]=count;
		counts[i]=numToProcess;
		lastStart=count;
		count+=numToProcess;
	}
	//Some tasks need to do one extra block.
	numToProcess=eTASK_UNIT_WORK_SIZE*(numBlocksPerTask+1);
	for(PxU32 i=(numTasks-remainderBlocks);i<numTasks;i++)
	{
		starts[i]=count;
		counts[i]=numToProcess;
		lastStart=count;
		count+=numToProcess;
	}
	PX_ASSERT((count>=numUpdated) && (count<=(numUpdated+eTASK_UNIT_WORK_SIZE)));
	counts[numTasks-1] = numUpdated - lastStart;

	//Some tasks do no work at all.
	for(PxU32 i=numTasks;i<eMAX_NUM_TASKS;i++)
	{
		starts[i]=0;
		counts[i]=0;
	}

	PX_ASSERT(allWorkItemsAccounted(starts, counts, numTasks, numUpdated));
}

void SingleAABBUpdateTask::runInternal()
{
	PxcBpHandle updatedElemIdStatics[2048];
	PxcBpHandle updatedElemIdDynamics[2048];

	PxU32 numFastMovingShapes=0;

	const PxcBpHandle* _updatedElemIds[2] = {NULL, };
	const PxcBpHandle* _groups[2] = {NULL, NULL};
	const PxcBpHandle* _aabbDataHandles[2] = {NULL, NULL};
	const PxcAABBDataStatic* _staticAABBData[2] = {NULL, NULL};
	const PxcAABBDataDynamic* _dynamicAABBData[2] = {NULL, NULL};
	IntegerAABB* _bounds[2] = {NULL, NULL};
	PxU32 _boundsCapacities[2] = {0, 0};
	PxU32 _starts[2] = {0, 0};
	PxU32 _counts[2] = {0, 0};

	PxU32 updateTypeCount  = 0;
	if(eUPDATE_TYPE_SHAPE_SINGLE & mUpdateType)
	{
		_updatedElemIds[updateTypeCount] = mAABBMgr->mBPUpdatedElemIds.mBuffer;
		_groups[updateTypeCount] = mAABBMgr->mBPElems.getGroups();
		_aabbDataHandles[updateTypeCount] = mAABBMgr->mBPElems.getAABBDataHandles();
		_staticAABBData[updateTypeCount] = mAABBMgr->mBPElems.getStaticAABBDataArray();
		_dynamicAABBData[updateTypeCount] = mAABBMgr->mBPElems.getDynamicAABBDataArray();
		_bounds[updateTypeCount] = mAABBMgr->mBPElems.getBounds();
		_boundsCapacities[updateTypeCount] = mAABBMgr->mBPElems.getCapacity();
		_starts[updateTypeCount] = mBPStart;
		_counts[updateTypeCount] = mBPCount;
		updateTypeCount++;
	}
	if(eUPDATE_TYPE_SHAPE_AGGREGATE & mUpdateType)
	{
		_updatedElemIds[updateTypeCount] = mAABBMgr->mAggregateUpdatedElemIds.mBuffer;
		_groups[updateTypeCount] = mAABBMgr->mAggregateElems.getGroups();
		_aabbDataHandles[updateTypeCount] = mAABBMgr->mAggregateElems.getAABBDataHandles();
		_staticAABBData[updateTypeCount] = mAABBMgr->mAggregateElems.getStaticAABBDataArray();
		_dynamicAABBData[updateTypeCount] = mAABBMgr->mAggregateElems.getDynamicAABBDataArray();
		_bounds[updateTypeCount] = mAABBMgr->mAggregateElems.getBounds();
		_boundsCapacities[updateTypeCount] = mAABBMgr->mAggregateElems.getCapacity();
		_starts[updateTypeCount] = mAggregateStart;
		_counts[updateTypeCount] = mAggregateCount;
		updateTypeCount++;
	}

	for(PxU32 k = 0; k < updateTypeCount; k++)
	{
		PxU32 updatedElemIdsStaticSize=0;
		PxU32 updatedElemIdsDynamicSize=0;

		const PxcBpHandle* updatedElemIds = _updatedElemIds[k];
		const PxcBpHandle* groups = _groups[k];
		const PxcBpHandle* aabbDataHandles = _aabbDataHandles[k];
		const PxcAABBDataStatic* staticAABBData = _staticAABBData[k];
		const PxcAABBDataDynamic* dynamicAABBData = _dynamicAABBData[k];
		IntegerAABB* bounds = _bounds[k];
		PxU32 boundsCapacity = _boundsCapacities[k];
		const PxU32 start = _starts[k];
		const PxU32 end = _starts[k] + _counts[k];

		for(PxU32 i=start;i<end;i++)
		{
			const PxcBpHandle id=updatedElemIds[i];
			if(0==groups[id])
			{
				updatedElemIdStatics[updatedElemIdsStaticSize]=id;
				updatedElemIdsStaticSize++;
				if(2048==updatedElemIdsStaticSize)
				{
					updateBodyShapeAABBs(updatedElemIdStatics, updatedElemIdsStaticSize, aabbDataHandles, staticAABBData, bounds, boundsCapacity);
					updatedElemIdsStaticSize=0;
				}
			}
			else
			{
				updatedElemIdDynamics[updatedElemIdsDynamicSize]=id;
				updatedElemIdsDynamicSize++;
				if(2048==updatedElemIdsDynamicSize)
				{
					numFastMovingShapes+=updateBodyShapeAABBs(updatedElemIdDynamics, updatedElemIdsDynamicSize, aabbDataHandles, dynamicAABBData, mParams.secondBroadPhase, bounds, boundsCapacity);
					updatedElemIdsDynamicSize=0;
				}
			}
		}

		if(updatedElemIdsStaticSize>0)
		{
			updateBodyShapeAABBs(updatedElemIdStatics, updatedElemIdsStaticSize, aabbDataHandles, staticAABBData, bounds, boundsCapacity);
		}

		if(updatedElemIdsDynamicSize>0)
		{
			numFastMovingShapes+=updateBodyShapeAABBs(updatedElemIdDynamics, updatedElemIdsDynamicSize, aabbDataHandles, dynamicAABBData, mParams.secondBroadPhase, bounds, boundsCapacity);
		}
	}

	mNumFastMovingShapes+=numFastMovingShapes;
}

void ActorAABBUpdateTask::runInternal()
{
	PxcBpHandle actorIds[MAX_AGGREGATE_BOUND_SIZE];
	PxBounds3 localSpaceBounds[MAX_AGGREGATE_BOUND_SIZE];

	const PxU32 startId = mStart;
	const PxU32 endId = mStart + mCount;
	for(PxU32 i = startId; i < endId; i++)
	{
		//Get the aggregate
		const PxcBpHandle aggregateId = mDirtyActorAggregateIds[i];
		const Aggregate* aggregate = mAABBMgr->mAggregateManager.getAggregate(aggregateId);
		PX_ASSERT(!aggregate->selfCollide);

		//Iterate over all shapes of the aggregate and note the shape and actor ids for any actors that are dirty.
		//Set the bounds of each dirty actor to empty. We're going to recompute the bounds later.
		PxU32 numShapes = 0;
		PxcBpHandle shapeId = aggregate->elemHeadID;
		while(shapeId != PX_INVALID_BP_HANDLE)
		{
			const PxcBpHandle actorId = mAABBMgr->mAggregateElems.getAggregateActorId(shapeId);
			if(mDirtyActorBitmap->test(actorId))
			{
				mAABBMgr->mAggregateActorManager.setEmptyBounds(actorId);
				if(mAABBMgr->mAggregateElems.getGroup(shapeId) != PX_INVALID_BP_HANDLE)
				{
					PX_ASSERT(mAABBMgr->mAggregateActorManager.getShapeCount(actorId) > 0);

					//Store the actor id of the shape.
					actorIds[numShapes] = actorId;

					//Compute the local-space bounds of the shape.
					if(NULL == mAABBMgr->mAggregateElems.getBodyAtom(shapeId))
					{
						//Static geometry
						const PxcAABBDataStatic& aabbData = mAABBMgr->mAggregateElems.getStaticAABBData(shapeId);
						PxsComputeAABBLocalSpace(aabbData, localSpaceBounds[numShapes]);
					}
					else
					{
						//Dynamic geometry
						const PxcAABBDataDynamic& aabbData = mAABBMgr->mAggregateElems.getDynamicAABBData(shapeId);
						PxsComputeAABBLocalSpace(aabbData, localSpaceBounds[numShapes]);
					}

					numShapes++;
				}
			}
			shapeId = mAABBMgr->mAggregateElems.getNextId(shapeId);
		}

		//Compute the union of the local space bounds of each actor.
		for(PxU32 j = 0; j < numShapes; j++)
		{
			const PxcBpHandle actorId = actorIds[j];
			mAABBMgr->mAggregateActorManager.expandBounds(actorId, localSpaceBounds[j]);
		}
	}
}

void AggregateAABBUpdateTask::runInternal()
{
	PxF32 numFastMovingShapes=0;

	if(mCount>0)
	{
		const PxcBpHandle* PX_RESTRICT updatedAggregateIds = mAABBMgr->mAggregatesUpdated.getElems();
		IntegerAABB* PX_RESTRICT bpBounds = mAABBMgr->mBPElems.getBounds();
		IntegerAABB* PX_RESTRICT aggregateElemBounds = mAABBMgr->mAggregateElems.getBounds();

		const PxU32 start = mStart;
		const PxU32 end = mStart + mCount;
		for(PxU32 i = start; i < end; i++)
		{
			const PxcBpHandle aggregateId = updatedAggregateIds[i];
			const Aggregate* aggregate = mAABBMgr->mAggregateManager.getAggregate(aggregateId);

			if(aggregate->selfCollide || mAABBMgr->mAggregateManager.getHasBroadphaseOverlap(aggregateId))
			{
				//Bound computed from union of all world-space shape bounds.
				IntegerAABB aggregateBounds;
				aggregateBounds.setEmpty();
				PxcBpHandle elemId = aggregate->elemHeadID;
				while(PX_INVALID_BP_HANDLE != elemId)
				{
					if(mAABBMgr->mAggregateElems.getGroup(elemId) != PX_INVALID_BP_HANDLE)
					{
						const IntegerAABB& iaabb = aggregateElemBounds[elemId];
						aggregateBounds.include(iaabb);
					}
					elemId = mAABBMgr->mAggregateElems.getNextId(elemId);
				}

				const PxcBpHandle bpElemId = aggregate->bpElemId;
				PX_ASSERT(bpElemId < mAABBMgr->mBPElems.getCapacity());
				bpBounds[bpElemId] = aggregateBounds;		
			}
			else
			{
				//Bounds computed from union of all world-space actor bounds.

				//Iterate over aggregate and get any elemId for each actor so we can look up the bodyAtom/rigidCore for each actor.
				{
				PxcBpHandle elemId = aggregate->elemHeadID;
				while(PX_INVALID_BP_HANDLE != elemId)
				{
					if(mAABBMgr->mAggregateElems.getGroup(elemId) != PX_INVALID_BP_HANDLE)
					{
						const PxcBpHandle actorId = mAABBMgr->mAggregateElems.getAggregateActorId(elemId);
						mActorToElemIds[actorId] = elemId;
					}
					elemId = mAABBMgr->mAggregateElems.getNextId(elemId);
				}
				}

				PxBounds3 aggregateBounds;
				aggregateBounds.setEmpty();

				//Iterate over each actor in the aggregate
				PxcBpHandle actorId = aggregate->actorHeadID;
				while(actorId != PX_INVALID_BP_HANDLE)
				{
					//Only add actors with non-empty bounds (an actor could have no shapes but still be in the aggregate)
					const PxU32 shapeCount = mAABBMgr->mAggregateActorManager.getShapeCount(actorId);
					if(shapeCount > 0)
					{
						PxBounds3 worldSpaceActorBounds;
						const PxBounds3& localSpaceActorBounds = mAABBMgr->mAggregateActorManager.getBounds(actorId);
						const PxcBpHandle elemId = mActorToElemIds[actorId];
						if(NULL == mAABBMgr->mAggregateElems.getBodyAtom(elemId))
						{
							const PxcAABBDataStatic& aabbData = mAABBMgr->mAggregateElems.getStaticAABBData(elemId);
							const PxsRigidCore* rigidCore = aabbData.mRigidCore;
							PxsComputeAABB(rigidCore, localSpaceActorBounds, worldSpaceActorBounds);
						}
						else
						{
							const PxcAABBDataDynamic& aabbData = mAABBMgr->mAggregateElems.getDynamicAABBData(elemId);
							const PxsRigidCore* rigidCore = aabbData.mRigidCore;
							const PxcRigidBody* bodyAtom = aabbData.mBodyAtom;
							numFastMovingShapes += PxsComputeAABB(mParams.secondBroadPhase, bodyAtom, rigidCore, localSpaceActorBounds, worldSpaceActorBounds);
						}

						//If the actor has only a single shape then the shape bounds is equal to the actor bounds.
						//We can set the shape bounds directly here and avoid computing it later.
						if(1 == shapeCount)
						{
							IntegerAABB iaabb;
							iaabb.encode(worldSpaceActorBounds);
							aggregateElemBounds[elemId] = iaabb;
						}
						aggregateBounds.include(worldSpaceActorBounds);
					}

					actorId = mAABBMgr->mAggregateActorManager.getNextId(actorId);
				}

				const PxcBpHandle bpElemId = aggregate->bpElemId;
				IntegerAABB iaabb;
				iaabb.encode(aggregateBounds);
				PX_ASSERT(bpElemId < mAABBMgr->mBPElems.getCapacity());
				bpBounds[bpElemId] = iaabb;
			}
		}
	}

	mNumFastMovingShapes = (PxU32)numFastMovingShapes;
}

void SingleAABBTask::updateNumFastMovingShapes() const
{
	PxU32 numFastMovingShapes=0;
	for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
	{
		numFastMovingShapes+=mAABBUpdateTask[i].getNumFastMovingShapes();
	}
	*mParams.numFastMovingShapes += (PxI32)numFastMovingShapes;
}

void SingleAABBTask::setNumFastMovingShapes(const PxU32 task, const PxU32 numFastMovingShapes)
{
	PX_ASSERT(task<eMAX_NUM_TASKS);
	mAABBUpdateTask[task].setNumFastMovingShapes(numFastMovingShapes);
}

void AggregateAABBTask::updateNumFastMovingShapes() const
{
	PxU32 numFastMovingShapes=0;
	for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
	{
		numFastMovingShapes+=mAABBUpdateTask[i].getNumFastMovingShapes();
	}
	*mParams.numFastMovingShapes += (PxI32)numFastMovingShapes;
}

void AggregateAABBTask::setNumFastMovingShapes(const PxU32 task, const PxU32 numFastMovingShapes)
{
	PX_ASSERT(task<eMAX_NUM_TASKS);
	mAABBUpdateTask[task].setNumFastMovingShapes(numFastMovingShapes);
}

SingleAABBTask::SingleAABBTask()
: mUpdateType(SingleAABBUpdateTask::eUPDATE_TYPE_UNKNOWN),
  mAABBMgr(NULL)
{
#ifdef PX_PS3
	mAABBUpdateTaskSPU = (SingleAABBTaskSPU*)physx::shdfnd::AlignedAllocator<128>().allocate(sizeof(SingleAABBTaskSPU), __FILE__, __LINE__);
#endif
}

SingleAABBTask::~SingleAABBTask()
{
#ifdef PX_PS3
	physx::shdfnd::AlignedAllocator<128>().deallocate(mAABBUpdateTaskSPU);
#endif
}

void SingleAABBTask::runInternal()
{
#ifdef PX_PROFILE
	if(mUpdateType & SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_SINGLE)
	{
		CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateAggLocalSpaceBounds());
		CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateShapeAABBs());
	}
	else 
	{
		CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetprocessBPResults());
		CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateLazyAggShapeAABBs());
	}
#ifdef PX_PS3
	if(mUpdateType & SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_SINGLE)
	{
		stopTimerMarker(eCOMPUTE_AGG_LOCAL_SPACE_BOUNDS);
		startTimerMarker(eCOMPUTE_SHAPE_AABBS);
	}
	else
	{
		stopTimerMarker(ePROCESS_BP_RESULTS);
		startTimerMarker(eCOMPUTE_LAZY_AGG_SHAPE_AABBS);
	}
#endif
#endif

#if SPU_AABB
#if FORCE_SINGLE_SPU_AABB
	const PxU32 numSpus = 1;
#else
	const PxU32 numSpus = mParams.numSpusAABB;
#endif
#endif

	//Reset the number of fast moving shapes for each task.
	for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
	{
		mAABBUpdateTask[i].setNumFastMovingShapes(0);
	}

#if SPU_AABB

	//Do we need to run the bp elems on ppu?
	bool needsToRunBPOnPPU  = false;
	if(SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_SINGLE & mUpdateType)
	{
		needsToRunBPOnPPU = 
			(mAABBMgr->mBPUpdatedElemIdsSize > MAX_NUM_BP_SPU_SAP_AABB) ||
			(mAABBMgr->mBPElems.getCapacity() > MAX_NUM_BP_SPU_SAP_AABB);
	}

	//Do we need to run the aggregate elems on ppu?
	bool needsToRunAggregateOnPPU = false;
	if(SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_AGGREGATE & mUpdateType)
	{
		needsToRunAggregateOnPPU = 
			(mAABBMgr->mAggregateUpdatedElemIdsSize > MAX_NUM_BP_SPU_SAP_AABB) ||
			(mAABBMgr->mAggregateElems.getCapacity() > MAX_NUM_BP_SPU_SAP_AABB);
	}

	//If we have no spus or no work to do or one of the above has to run on ppu then we need to run on ppu.
	const bool runOnPPU = (!numSpus || !(mAABBMgr->mBPUpdatedElemIdsSize + mAABBMgr->mAggregateUpdatedElemIdsSize) || needsToRunBPOnPPU || needsToRunAggregateOnPPU);

	if(!runOnPPU)
	{
		// Reinitialise aabb update task
		new(mAABBUpdateTaskSPU) SingleAABBTaskSPU(mAABBMgr, numSpus, mParams, mUpdateType);

		// Set continuation and run immediately
		mAABBUpdateTaskSPU->setContinuation(mCont);
		mAABBUpdateTaskSPU->removeReference();
	}
	else
#endif
	{
		//Reset all tasks.
		for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
		{
			new(&mAABBUpdateTask[i]) SingleAABBUpdateTask(mAABBMgr, mParams, mUpdateType);
		}

		const PxU32 numTasks = PxMin((PxU32)eMAX_NUM_TASKS, mParams.numCpuTasks);

		//Divide bp aabbs up among all tasks.
		PxU32 numUpdatedBPElems = 0;
		if((SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_SINGLE & mUpdateType) && (mAABBMgr->mBPUpdatedElemIdsSize > 0))
		{
			numUpdatedBPElems = mAABBMgr->mBPUpdatedElemIdsSize;
			if((numTasks > 1) && (numUpdatedBPElems > eTASK_UNIT_WORK_SIZE))
			{
				PxU32 starts[eMAX_NUM_TASKS];
				PxU32 counts[eMAX_NUM_TASKS];
				computeTaskWork<eTASK_UNIT_WORK_SIZE, eMAX_NUM_TASKS>(numUpdatedBPElems, numTasks, starts, counts);
				for(PxU32 i = 0; i < numTasks; i++)
				{
					mAABBUpdateTask[i].setBPCount(starts[i],counts[i]);
				}
			}
			else
			{
				mAABBUpdateTask[0].setBPCount(0, numUpdatedBPElems);
			}
		}
		//Divide aggregate aabbs up among all tasks.
		PxU32 numUpdatedAggregateElems = 0;
		if((SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_AGGREGATE & mUpdateType) && (mAABBMgr->mAggregateUpdatedElemIdsSize > 0))
		{
			numUpdatedAggregateElems = mAABBMgr->mAggregateUpdatedElemIdsSize;
			if((numTasks > 1) && (numUpdatedAggregateElems > eTASK_UNIT_WORK_SIZE))
			{
				PxU32 starts[eMAX_NUM_TASKS];
				PxU32 counts[eMAX_NUM_TASKS];
				computeTaskWork<eTASK_UNIT_WORK_SIZE, eMAX_NUM_TASKS>(numUpdatedAggregateElems, numTasks, starts, counts);
				for(PxU32 i = 0; i < numTasks; i++)
				{
					mAABBUpdateTask[i].setAggregateCount(starts[i],counts[i]);
				}
			}
			else
			{
				mAABBUpdateTask[0].setAggregateCount(0,numUpdatedAggregateElems);
			}
		}

		//If we have more than 1 thread and have more than one block of work to perform then run in parallel.
		//If we have only 1 thread or just a single block of work then run without spawning another task.
		if((numTasks > 1) && (numUpdatedBPElems > eTASK_UNIT_WORK_SIZE || numUpdatedAggregateElems > eTASK_UNIT_WORK_SIZE))
		{
			//Run in parallel with tasks.
			for(PxU32 i=0;i<numTasks;i++)
			{
				mAABBUpdateTask[i].setContinuation(mCont);
			}
			for(PxU32 i=0;i<numTasks;i++)
			{
				mAABBUpdateTask[i].removeReference();
			}
		}
		else
		{
			//Do all the work immediately without spawning another task.
			mAABBUpdateTask[0].run();
		}
	}
}

#ifdef PX_PS3

SingleAABBTaskSPU::SingleAABBTaskSPU()
:  PxSpuTask(gPS3GetElfImage(SPU_ELF_COMPUTEAABB_TASK), gPS3GetElfSize(SPU_ELF_COMPUTEAABB_TASK), 0),
   mAABBMgr(NULL),
   mUpdateType(SingleAABBUpdateTask::eUPDATE_TYPE_UNKNOWN)
{
}

SingleAABBTaskSPU::SingleAABBTaskSPU(PxsAABBManager* aabbMgr, const PxU32 numSpus, const PxsComputeAABBParams& params, const PxU32 updateType) 
:  PxSpuTask(gPS3GetElfImage(SPU_ELF_COMPUTEAABB_TASK), gPS3GetElfSize(SPU_ELF_COMPUTEAABB_TASK), numSpus),
   mAABBMgr(aabbMgr),
   mUpdateType(updateType)
{
	mCellComputeAABBSPUInput.mParams							= params;
	mCellComputeAABBSPUInput.mUpdateType						= updateType;

	if(SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_SINGLE & updateType)
	{
		mCellComputeAABBSPUInput.mBPUpdatedElems				= aabbMgr->mBPUpdatedElemIds.mBuffer;
		mCellComputeAABBSPUInput.mBPUpdatedElemsSize			= aabbMgr->mBPUpdatedElemIdsSize;

		mCellComputeAABBSPUInput.mBPElemsWordStarts				= aabbMgr->mBPUpdatedElemWordStarts.mBuffer;
		mCellComputeAABBSPUInput.mBPElemsWordEnds				= aabbMgr->mBPUpdatedElemWordEnds.mBuffer;
		mCellComputeAABBSPUInput.mBPElemsWordCount				= aabbMgr->mBPUpdatedElemWordCount;

		mCellComputeAABBSPUInput.mBPElemsBoxMinMaxXYZ			= aabbMgr->mBPElems.getBounds();		
		mCellComputeAABBSPUInput.mBPElemsAABBDataHandles		= aabbMgr->mBPElems.getAABBDataHandles();
		mCellComputeAABBSPUInput.mBPElemsCapacity				= aabbMgr->mBPElems.getCapacity();

		mCellComputeAABBSPUInput.mBPElemsAABBDataStatic			= aabbMgr->mBPElems.getStaticAABBDataArray();
		mCellComputeAABBSPUInput.mBPElemsAABBDataStaticCapacity	= aabbMgr->mBPElems.getStaticAABBDataArrayCapacity();
		mCellComputeAABBSPUInput.mBPElemsAABBDataDynamic		= aabbMgr->mBPElems.getDynamicAABBDataArray();
		mCellComputeAABBSPUInput.mBPElemsAABBDataDynamicCapacity= aabbMgr->mBPElems.getDynamicAABBDataArrayCapacity();
	}

	if(SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_AGGREGATE & updateType)
	{
		mCellComputeAABBSPUInput.mAggregateUpdatedElems			= aabbMgr->mAggregateUpdatedElemIds.mBuffer;
		mCellComputeAABBSPUInput.mAggregateUpdatedElemsSize		= aabbMgr->mAggregateUpdatedElemIdsSize;

		mCellComputeAABBSPUInput.mAggregateElemsWordStarts		= aabbMgr->mAggregateUpdatedElemWordStarts.mBuffer;
		mCellComputeAABBSPUInput.mAggregateElemsWordEnds		= aabbMgr->mAggregateUpdatedElemWordEnds.mBuffer;
		mCellComputeAABBSPUInput.mAggregateElemsWordCount		= aabbMgr->mAggregateUpdatedElemWordCount;

		mCellComputeAABBSPUInput.mAggregateElemsBounds			= aabbMgr->mAggregateElems.getBounds();		
		mCellComputeAABBSPUInput.mAggregateElemsAABBDataHandles	= aabbMgr->mAggregateElems.getAABBDataHandles();
		mCellComputeAABBSPUInput.mAggregateElemsCapacity		= aabbMgr->mAggregateElems.getCapacity();

		mCellComputeAABBSPUInput.mAggregateElemsAABBDataStatic	= aabbMgr->mAggregateElems.getStaticAABBDataArray();
		mCellComputeAABBSPUInput.mAggregateElemsAABBDataStaticCapacity = aabbMgr->mAggregateElems.getStaticAABBDataArrayCapacity();
		mCellComputeAABBSPUInput.mAggregateElemsAABBDataDynamic	= aabbMgr->mAggregateElems.getDynamicAABBDataArray();
		mCellComputeAABBSPUInput.mAggregateElemsAABBDataDynamicCapacity = aabbMgr->mAggregateElems.getDynamicAABBDataArrayCapacity();
	}

	mCellComputeAABBSPUInput.mActiveTaskCount					= numSpus;	
	mCellComputeAABBSPUInput.mTotalTaskCount					= numSpus;	

#if SPU_PROFILE
	for(PxU32 i=0;i<MAX_NUM_SPU_PROFILE_ZONES;i++)
	{
		mProfileCounters[i]=0;
	}
	mCellComputeAABBSPUInput.mProfileZones					= mProfileCounters;
#endif

	// set SPU arguments
	for (PxU32 uiTask=0; uiTask < numSpus; uiTask++) 
	{
		mCellComputeAABBSPUOutput[uiTask].mNumFastMovingShapes	=	0;
		setArgs(uiTask, uiTask | (unsigned int)&mCellComputeAABBSPUOutput[uiTask], (unsigned int)&mCellComputeAABBSPUInput);
	}
}

void SingleAABBTaskSPU::release()
{
	for (PxU32 uiTask=0; uiTask < getSpuCount(); uiTask++) 
	{
		mAABBMgr->mSingleShapeAABBTask.setNumFastMovingShapes(uiTask, mCellComputeAABBSPUOutput[uiTask].mNumFastMovingShapes);
	}

	PxSpuTask::release();
}

#endif //PX_PS3


ActorAABBTask::ActorAABBTask()
: mAABBMgr(NULL)
{
}

ActorAABBTask::~ActorAABBTask()
{
}

void ActorAABBTask::runInternal()
{
#ifdef PX_PROFILE
	CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateAggLocalSpaceBounds());
#ifdef PX_PS3
	startTimerMarker(eCOMPUTE_AGG_LOCAL_SPACE_BOUNDS);
#endif
#endif

	//Get the bitmap of dirty actors.
	const Cm::BitMap& dirtyActorsBitmap = mAABBMgr->mAggregateActorManager.getDirtyBitmap();
	//Get the list of aggregates with dirty actors.
	const PxcBpHandle* dirtyActorAggregates = mAABBMgr->mDirtyActorAggregates.mBuffer;
	const PxU32 numDirtyActorAggregates = mAABBMgr->mNumDirtyActorAggregates;

	//Reset all tasks.
	for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
	{
		new(&mAABBUpdateTask[i]) ActorAABBUpdateTask(mAABBMgr, dirtyActorAggregates, &dirtyActorsBitmap);
	}

	const PxU32 numTasks = PxMin((PxU32)eMAX_NUM_TASKS, mParams.numCpuTasks);

	//Divide work among all tasks.
	if(numDirtyActorAggregates > 0)
	{
		//If there are is only a single thread or just a few aabbs to update then we can
		//proceed immediately with the aabb computation on the current thread.
		if(numTasks > 1 && numDirtyActorAggregates > eTASK_UNIT_WORK_SIZE)
		{
			PxU32 starts[eMAX_NUM_TASKS];
			PxU32 counts[eMAX_NUM_TASKS];
			computeTaskWork<eTASK_UNIT_WORK_SIZE, eMAX_NUM_TASKS>(numDirtyActorAggregates, numTasks, starts, counts);
			for(PxU32 i=0;i<numTasks;i++)
			{
				mAABBUpdateTask[i].setCount(starts[i],counts[i]);
			}
		}
		else
		{
			mAABBUpdateTask[0].setCount(0, numDirtyActorAggregates);
		}
	}

	//If we have more than 1 thread and have more than one block of work to perform then run in parallel.
	//If we have only 1 thread or just a single block of work then run without spawning another task.
	if((numTasks > 1) && (numDirtyActorAggregates > eTASK_UNIT_WORK_SIZE))
	{
		//Run in parallel with tasks.
		for(PxU32 i=0;i<numTasks;i++)
		{
			mAABBUpdateTask[i].setContinuation(mCont);
		}
		for(PxU32 i=0;i<numTasks;i++)
		{
			mAABBUpdateTask[i].removeReference();
		}
	}
	else
	{
		//Do all the work immediately without spawning another task.
		mAABBUpdateTask[0].run();
	}
}


AggregateAABBTask::AggregateAABBTask()
: mAABBMgr(NULL)
{
}

AggregateAABBTask::~AggregateAABBTask()
{
}

void AggregateAABBTask::runInternal()
{
#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateShapeAABBs());
	CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateAggAABBs());
#ifdef PX_PS3
	stopTimerMarker(eCOMPUTE_SHAPE_AABBS);
	startTimerMarker(eCOMPUTE_AGG_AABBS);
#endif
#endif 

	//Reset the number of fast moving shapes for each task.
	for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
	{
		mAABBUpdateTask[i].setNumFastMovingShapes(0);
	}

	//Reset all tasks.
	for(PxU32 i=0;i<eMAX_NUM_TASKS;i++)
	{
		new(&mAABBUpdateTask[i]) AggregateAABBUpdateTask(mAABBMgr, mParams, mAABBMgr->mAggregateActorShapeIds.mBuffer);
	}

	const PxU32 numTasks = PxMin((PxU32)eMAX_NUM_TASKS, mParams.numCpuTasks);

	//Divide bp aabbs up among all tasks.
	const PxU32 numUpdatedAggregates = mAABBMgr->mAggregatesUpdated.getElemsSize();
	if(numUpdatedAggregates > 0)
	{
		//If there are is only a single thread or just a few aabbs to update then we can
		//proceed immediately with the aabb computation on the current thread.
		if(numTasks > 1 && numUpdatedAggregates > eTASK_UNIT_WORK_SIZE)
		{
			PxU32 starts[eMAX_NUM_TASKS];
			PxU32 counts[eMAX_NUM_TASKS];
			computeTaskWork<eTASK_UNIT_WORK_SIZE, eMAX_NUM_TASKS>(numUpdatedAggregates, numTasks, starts, counts);
			for(PxU32 i=0;i<numTasks;i++)
			{
				mAABBUpdateTask[i].setCount(starts[i],counts[i]);
			}
		}
		else
		{
			mAABBUpdateTask[0].setCount(0, numUpdatedAggregates);
		}
	}

	//If we have more than 1 thread and have more than one block of work to perform then run in parallel.
	//If we have only 1 thread or just a single block of work then run without spawning another task.
	if((numTasks > 1) && (numUpdatedAggregates > eTASK_UNIT_WORK_SIZE))
	{
		//Run in parallel with tasks.
		for(PxU32 i=0;i<numTasks;i++)
		{
			mAABBUpdateTask[i].setContinuation(mCont);
		}
		for(PxU32 i=0;i<numTasks;i++)
		{
			mAABBUpdateTask[i].removeReference();
		}
	}
	else
	{
		//Do all the work immediately without spawning another task.
		mAABBUpdateTask[0].run();
	}
}

void BPWorkTask::runInternal()
{
#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateAggAABBs());
	CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateBroadphase());
#ifdef PX_PS3
	stopTimerMarker(eCOMPUTE_AGG_AABBS);
	startTimerMarker(eUPDATE_BROADPHASE);
#endif
#endif

	//Tidy up any loose ends needed to finish off the aabb update calculations.
	//(eg compute the aggregate bounds from the shape bounds).
	mAABBMgr->prepareBP();

	//Prepare a data structure to pass to the broadphase.
	PxcBroadPhaseUpdateData updateData(
		mAABBMgr->mBPCreatedElems.getElems(), mAABBMgr->mBPCreatedElems.getElemsSize(),
		mAABBMgr->mBPUpdatedElems.getElems(), mAABBMgr->mBPUpdatedElems.getElemsSize(),
		mAABBMgr->mBPRemovedElems.getElems(), mAABBMgr->mBPRemovedElems.getElemsSize(),
		mAABBMgr->mBPElems.getBounds(), mAABBMgr->mBPElems.getGroups(), mAABBMgr->mBPElems.getCapacity());

	//Update the broadphase.
	mAABBMgr->mBP->update(mParams.numCpuTasks, mParams.numSpusBP, updateData, mCont);
}

void ProcessBPResultsTask::runInternal()
{
#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateBroadphase());
	CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetprocessBPResults());
#ifdef PX_PS3
	stopTimerMarker(eUPDATE_BROADPHASE); 
	startTimerMarker(ePROCESS_BP_RESULTS);
#endif
#endif

	mAABBMgr->processBPResults(mParams);
}

void AggregateOverlapWorkTask::setAABBMgr(PxsAABBManager* AABBMgr)
{
	mAABBMgr = AABBMgr;

	mCreatedPairs = (PxvBroadPhaseOverlap*)mAABBMgr->mScratchAllocator.alloc(sizeof(PxvBroadPhaseOverlap)*1024, true);
	mDeletedPairs = (PxvBroadPhaseOverlap*)mAABBMgr->mScratchAllocator.alloc(sizeof(PxvBroadPhaseOverlap)*1024, true);
	mCreatedPairsCapacity = 1024;
	mDeletedPairsCapacity = 1024;
}

void AggregateOverlapWorkTask::runInternal()
{
	mAABBMgr->selfCollideAggregates
		(mAggSelfStartId, mAggSelfNbWorkItems, mAggregateSortedData,
		mCreatedPairs, mCreatedPairsSize, mCreatedPairsCapacity,
		mDeletedPairs, mDeletedPairsSize, mDeletedPairsCapacity);

	mAABBMgr->processAggregatePairs
		(mAggAggStartId, mAggAggNbWorkItems, mAggregateSortedData,
		 mCreatedPairs, mCreatedPairsSize, mCreatedPairsCapacity,
		 mDeletedPairs, mDeletedPairsSize, mDeletedPairsCapacity);
}


void AggregateOverlapTask::runInternal()
{
#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetupdateLazyAggShapeAABBs());
	CM_PROFILE_START_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetcomputeAggAggShapePairs());
#ifdef PX_PS3
	stopTimerMarker(eCOMPUTE_LAZY_AGG_SHAPE_AABBS);
	startTimerMarker(eCOMPUTE_AGGAGG_SHAPE_PAIRS);
#endif
#endif

	if(mAABBMgr->mNumAggregatesToSort > 0)
	{
		mRankIds.resize(mAABBMgr->mAggregateElems.getCapacity());
		mElemIds.resize(mAABBMgr->mAggregateElems.getCapacity());
		mAggregateSortedData.resize(mAABBMgr->mAggregateManager.getAggregatesCapacity());

		//Sort the aggregate shape bounds serially.  Could be done in parallel.
		mAABBMgr->sortAggregates(mRankIds.mBuffer, mRankIds.mBufferSize, mElemIds.mBuffer, mElemIds.mBufferSize, mAggregateSortedData.mBuffer);

		processSelfCollideAndAggregatePairs();
	}
}

void AggregateOverlapTask::processSelfCollideAndAggregatePairs()
{
	PX_ASSERT(mAABBMgr->mNumAggregatesToSort > 0);

	//Reset all tasks.
	for(PxU32 i = 0; i < eMAX_NUM_TASKS; i++)
	{
		new(&mAggregateOverlapWorkTasks[i]) AggregateOverlapWorkTask();
		mAggregateOverlapWorkTasks[i].setAABBMgr(mAABBMgr);
		mAggregateOverlapWorkTasks[i].setAggregateSortedData(mAggregateSortedData.mBuffer);
	}

	const PxU32 numSelfCollide = mAABBMgr->mAggregatesUpdated.getElemsSize();
	const PxU32 numAggPairs = mAABBMgr->mNumAggregatePairsToOverlap;

	//Allocate all memory serially.
	if(numAggPairs > 0)
	{
		mAABBMgr->initialiseAggregateAggregateBitmaps();
	}

	//Divide work up among all tasks.
	const PxU32 numTasks = PxMin((PxU32)eMAX_NUM_TASKS, mParams.numCpuTasks);
	if((numTasks > 1) && ((numAggPairs > eTASK_UNIT_WORK_SIZE) || (numSelfCollide > eTASK_UNIT_WORK_SIZE)))
	{
		if(numSelfCollide > eTASK_UNIT_WORK_SIZE)
		{
			//Divide the work among the available tasks.
			PxU32 starts[eMAX_NUM_TASKS];
			PxU32 counts[eMAX_NUM_TASKS];
			computeTaskWork<eTASK_UNIT_WORK_SIZE, eMAX_NUM_TASKS>(numSelfCollide, numTasks, starts, counts);
			for(PxU32 i = 0; i < numTasks; i++)
			{
				mAggregateOverlapWorkTasks[i].setAggSelfWorkStartAndCount(starts[i],counts[i]);
			}
		}
		else
		{
			mAggregateOverlapWorkTasks[0].setAggSelfWorkStartAndCount(0, numSelfCollide);
		}

		if(numAggPairs > eTASK_UNIT_WORK_SIZE)
		{
			PxU32 starts[eMAX_NUM_TASKS];
			PxU32 counts[eMAX_NUM_TASKS];
			computeTaskWork<eTASK_UNIT_WORK_SIZE, eMAX_NUM_TASKS>(numAggPairs, numTasks, starts, counts);
			for(PxU32 i = 0; i < numTasks; i++)
			{
				mAggregateOverlapWorkTasks[i].setAggAggWorkStartAndCount(starts[i],counts[i]);
			}
		}
		else
		{
			mAggregateOverlapWorkTasks[0].setAggAggWorkStartAndCount(0, numAggPairs);
		}

		//Run in parallel using the tasks.
		for(PxU32 i=0;i<numTasks;i++)
		{
			mAggregateOverlapWorkTasks[i].setContinuation(mCont);
		}
		for(PxU32 i=0;i<numTasks;i++)
		{
			mAggregateOverlapWorkTasks[i].removeReference();
		}
	}
	else
	{
		mAggregateOverlapWorkTasks[0].setAggSelfWorkStartAndCount(0, numSelfCollide);
		mAggregateOverlapWorkTasks[0].setAggAggWorkStartAndCount(0, numAggPairs);
		mAggregateOverlapWorkTasks[0].run();
	}
}

void AggregateOverlapTask::complete()
{
	if(mAABBMgr->mNumAggregatesToSort > 0)
	{
		mAggregateSortedData.free();
		mRankIds.free();
		mElemIds.free();

		//Copy created and deleted pairs from tasks back to aabbmgr.
		PxvBroadPhaseOverlap* outPairs[2] = {mAABBMgr->mCreatedPairs, mAABBMgr->mDeletedPairs};
		PxU32 outSizes[2] = {mAABBMgr->mCreatedPairsSize, mAABBMgr->mDeletedPairsSize};
		PxU32 outCapacities[2] = {mAABBMgr->mCreatedPairsCapacity, mAABBMgr->mDeletedPairsCapacity};
		{
			PxcScratchAllocator& scratchAllocator = mAABBMgr->mScratchAllocator;

			//Count the number of created and deleted pairs.
			PxvBroadPhaseOverlap* modifiedPairs[eMAX_NUM_TASKS][2];
			PxU32 modifiedSizes[eMAX_NUM_TASKS][2];
			PxU32 totalModifiedSizes[2] = {0, 0};
			for(PxU32 i = 0; i < eMAX_NUM_TASKS; i++)
			{
				const AggregateOverlapWorkTask& workTask = mAggregateOverlapWorkTasks[i];
				modifiedPairs[i][0] = workTask.mCreatedPairs;
				modifiedPairs[i][1] = workTask.mDeletedPairs;
				modifiedSizes[i][0] = workTask.mCreatedPairsSize;
				modifiedSizes[i][1] = workTask.mDeletedPairsSize;
				totalModifiedSizes[0] += workTask.mCreatedPairsSize;
				totalModifiedSizes[1] += workTask.mDeletedPairsSize;
			}

			for(PxU32 i = 0; i < 2; i++)
			{
				PxvBroadPhaseOverlap* pairs = outPairs[i];
				PxU32 pairsSize = outSizes[i];
				PxU32 pairsCapacity = outCapacities[i];
				const PxU32 totalModifiedSize = totalModifiedSizes[i];

				//Allocate memory for new pairs if required.
				if(((pairsSize + totalModifiedSize) > pairsCapacity))
				{
					const PxU32 newCapacity = ((pairsSize + totalModifiedSize + 31) & ~31);
					PxvBroadPhaseOverlap* newPairs = (PxvBroadPhaseOverlap*)scratchAllocator.alloc(sizeof(PxvBroadPhaseOverlap)*newCapacity, true);
					PxMemCopy(newPairs, pairs, sizeof(PxvBroadPhaseOverlap)*pairsSize);
					scratchAllocator.free(pairs);
					outPairs[i] = newPairs;
					outCapacities[i] = newCapacity;
				}

				//Copy new pairs to aabbmgr.
				for(PxU32 k = 0; k < eMAX_NUM_TASKS; k++)
				{
					PxMemCopy(outPairs[i] + outSizes[i], modifiedPairs[k][i], sizeof(PxvBroadPhaseOverlap)*modifiedSizes[k][i]);
					outSizes[i] += modifiedSizes[k][i];
					scratchAllocator.free(modifiedPairs[k][i]);
				}
			}
		}
		mAABBMgr->mCreatedPairs = outPairs[0];
		mAABBMgr->mCreatedPairsSize = outSizes[0];
		mAABBMgr->mCreatedPairsCapacity = outCapacities[0];
		mAABBMgr->mDeletedPairs = outPairs[1];
		mAABBMgr->mDeletedPairsSize = outSizes[1];
		mAABBMgr->mDeletedPairsCapacity = outCapacities[1];
	}
}

void FinishTask::runInternal()
{
#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mAABBMgr->mEventProfiler, Cm::ProfileEventId::BroadPhase::GetcomputeAggAggShapePairs());
	Cm::EventProfiler* profiler = &mAABBMgr->mEventProfiler;
	CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::Getfinish());
#ifdef PX_PS3
	stopTimerMarker(eCOMPUTE_AGGAGG_SHAPE_PAIRS);
	startTimerMarker(eCOMPUTE_BPFINISH);
#endif
#endif

	//Need to aggregate the work of all the aggregate overlap tasks.
	{
		mAABBMgr->mAggregateOverlapTask.complete();
	}

	//Now free all buffers allocated during the update of the abbb manager
	{
		// Don't need the arrays of created/deleted bp pairs any longer.
		mAABBMgr->mBP->freeBuffers();

		// Don't need the arrays of created/updated/removed bp elems any longer.
		mAABBMgr->mBPCreatedElems.free();
		mAABBMgr->mBPUpdatedElems.free();
		mAABBMgr->mBPRemovedElems.free();
		mAABBMgr->mAggregatesUpdated.free();

		//Don't need the arrays of updated bp and aggregate shapes.
		mAABBMgr->mBPUpdatedElemIds.free();
		mAABBMgr->mBPUpdatedElemIdsSize = 0;
		mAABBMgr->mAggregateUpdatedElemIds.free();
		mAABBMgr->mAggregateUpdatedElemIdsSize = 0;
		
#ifdef PX_PS3
		//Don't need the lists of spu work to update bp elems.
		mAABBMgr->mBPUpdatedElemWordStarts.free();
		mAABBMgr->mBPUpdatedElemWordEnds.free();
		mAABBMgr->mBPUpdatedElemWordCount = 0;
		//Don't need the lists of spu work to update aggregate elems.
		mAABBMgr->mAggregateUpdatedElemWordStarts.free();
		mAABBMgr->mAggregateUpdatedElemWordEnds.free();
		mAABBMgr->mAggregateUpdatedElemWordCount = 0;
#endif

		//Don't need the arrays of actor shape ids and dirty actor aggregate ids
		mAABBMgr->mAggregateActorShapeIds.free();
		mAABBMgr->mDirtyActorAggregates.free();
		mAABBMgr->mNumDirtyActorAggregates = 0;

		//Don't need the list of aggregates to sort.
		mAABBMgr->mAggregatesToSort.free();
		mAABBMgr->mNumAggregatesToSort = 0;
		//Don't need the list of aggregate pairs to overlap.
		mAABBMgr->mAggregatePairsToOverlap.free();
		mAABBMgr->mNumAggregatePairsToOverlap = 0;
	}

	{
		//Clear the dirty actor bitmap
		mAABBMgr->mAggregateActorManager.resetDirtyBitmap();
	}

	{
		//Shapes in aggregates that have been release can be reused now.
		mAABBMgr->mAggregateManager.markReleasedAggregateElemsAsAvailable();
	}

	{
		//Update the record of aggregates that have a broadphase overlap.
		mAABBMgr->updateAggregateBPOverlapRecords();
	}

#ifdef PX_PROFILE
	CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::Getfinish());
#ifdef PX_PS3
	stopTimerMarker(eCOMPUTE_BPFINISH);
#endif
#endif
}

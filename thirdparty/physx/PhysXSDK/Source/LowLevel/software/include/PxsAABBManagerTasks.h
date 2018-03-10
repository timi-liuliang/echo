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


#ifndef PXS_AABB_MANAGER_TASKS_H
#define PXS_AABB_MANAGER_TASKS_H

#include "CmTask.h"
#include "PxsAABBManagerAux.h"

#ifdef PX_PS3
#include "CellComputeAABBTask.h"
#include "CellSPUProfiling.h"
#include "PS3Support.h"
#include "PxSpuTask.h"
#endif

namespace physx
{

class PxsAABBManager;

#ifdef PX_PS3
#define SPU_AABB 1
#define FORCE_SINGLE_SPU_AABB 0
#define SPU_PROFILE 0
#else
#define SPU_AABB 0
#endif //PX_PS3

//Update a batch of shape world-space bounds using a single task.
//Multiple tasks running in parallel on different batches 
//together perform all necessary shape world-space bound updates.
//The shapes can be individual shapes with one broadphase entry
//per shape or shapes of aggregates with one broadphase entry per
//aggregate.
class SingleAABBUpdateTask: public Cm::Task
{
	PX_NOCOPY(SingleAABBUpdateTask)

public:

	enum
	{
		eUPDATE_TYPE_SHAPE_SINGLE		= (1 << 0),
		eUPDATE_TYPE_SHAPE_AGGREGATE	= (1 << 1),
		eUPDATE_TYPE_UNKNOWN			= (1 << 2)
	};

	SingleAABBUpdateTask()
		: mAABBMgr(NULL),
		  mUpdateType(eUPDATE_TYPE_UNKNOWN),
		  mBPStart(0),
		  mBPCount(0),
		  mAggregateStart(0),
		  mAggregateCount(0),
		  mNumFastMovingShapes(0)
	{
	}

	SingleAABBUpdateTask(PxsAABBManager* AABBMgr, const PxsComputeAABBParams& params, const PxU32 updateType)
		: mAABBMgr(AABBMgr),
		  mParams(params),
		  mUpdateType(updateType),
		  mBPStart(0),
		  mBPCount(0),
		  mAggregateStart(0),
		  mAggregateCount(0),
		  mNumFastMovingShapes(0)
	{
	}

	virtual ~SingleAABBUpdateTask()
	{}

	virtual const char* getName() const { return "PxsAABBManager.singleAABBUpdate"; }

	virtual void runInternal();

	void setBPCount(const PxU32 start, const PxU32 count)
	{
		mBPStart = start;
		mBPCount = count;
	}

	void setAggregateCount(const PxU32 start, const PxU32 count)
	{
		mAggregateStart = start;
		mAggregateCount = count;
	}

	PX_FORCE_INLINE PxU32 getNumFastMovingShapes() const
	{
		return mNumFastMovingShapes;
	}

	PX_FORCE_INLINE void setNumFastMovingShapes(const PxU32 numFastMovingShapes)
	{
		mNumFastMovingShapes = numFastMovingShapes;
	}

private:

	PxsAABBManager* mAABBMgr;
	PxsComputeAABBParams mParams;
	PxU32 mUpdateType;
	PxU32 mBPStart;
	PxU32 mBPCount;
	PxU32 mAggregateStart;
	PxU32 mAggregateCount;
	PxU32 mNumFastMovingShapes;
};

//Update a batch of actor local-space bounds using a single task.
//Multiple tasks running in parallel on different batches 
//together perform all necessary actor local-space bound updates.
class ActorAABBUpdateTask : public Cm::Task
{
	PX_NOCOPY(ActorAABBUpdateTask)

public:

	ActorAABBUpdateTask()
		: mAABBMgr(NULL),
		  mDirtyActorAggregateIds(NULL),
		  mDirtyActorBitmap(NULL),
		  mStart(0),
		  mCount(0)
	{
	}

	ActorAABBUpdateTask(PxsAABBManager* AABBMgr, const PxcBpHandle* dirtyActorAggregateIds, const Cm::BitMap* dirtyActorBitmap)
		: mAABBMgr(AABBMgr),
		  mDirtyActorAggregateIds(dirtyActorAggregateIds),
		  mDirtyActorBitmap(dirtyActorBitmap),
		  mStart(0),
		  mCount(0)
	{
	}

	virtual ~ActorAABBUpdateTask()
	{
	}

	virtual const char* getName() const { return "PxsAABBManager.actorAABBUpdate"; }

	void setCount(const PxU32 start, const PxU32 count)
	{
		mStart = start;
		mCount = count;
	}

	virtual void runInternal();

private:

	PxsAABBManager* mAABBMgr;
	const PxcBpHandle* mDirtyActorAggregateIds;
	const Cm::BitMap* mDirtyActorBitmap;

	PxU32 mStart;
	PxU32 mCount;
};

//Update the composite world-space bounds of a batch of aggregates with a single task.
//Multiple tasks running in parallel on different batches together perform all necessary 
//actor aggregate composite bounds.  
//Aggregates without self-collision compute the aggregate bounds from the 
//union of actor world-space bounds. This can be fast if the actor count is less than the shape count.
//Aggregates with self-collision compute the aggregate bounds from the union of 
//individual shape bounds because we will definitely need the shape bounds for self-collision.
class AggregateAABBUpdateTask: public Cm::Task
{
	PX_NOCOPY(AggregateAABBUpdateTask)

public:

	AggregateAABBUpdateTask()
		: mAABBMgr(NULL),
		  mActorToElemIds(NULL),
		  mStart(0),
		  mCount(0),
		  mNumFastMovingShapes(0)
	{
	}

	AggregateAABBUpdateTask(PxsAABBManager* AABBMgr, const PxsComputeAABBParams& params, PxcBpHandle* actorToElemIds)
		: mAABBMgr(AABBMgr),
		  mParams(params),
		  mActorToElemIds(actorToElemIds),
		  mStart(0),
		  mCount(0),
		  mNumFastMovingShapes(0)
	{
	}

	virtual ~AggregateAABBUpdateTask()
	{
	}

	void setCount(const PxU32 start, const PxU32 count)
	{
		mStart = start;
		mCount = count;
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsAABBManager.aggregateAABBUpdate"; }

	PX_FORCE_INLINE PxU32 getNumFastMovingShapes() const
	{
		return mNumFastMovingShapes;
	}

	PX_FORCE_INLINE void setNumFastMovingShapes(const PxU32 numFastMovingShapes)
	{
		mNumFastMovingShapes = numFastMovingShapes;
	}

private:

	PxsAABBManager* mAABBMgr;
	PxsComputeAABBParams mParams;
	PxcBpHandle* mActorToElemIds;
	PxU32 mStart;
	PxU32 mCount;
	PxU32 mNumFastMovingShapes;
};


#ifdef PX_PS3
//Update all shape world-space bounds using multiple spus as required.
//The shapes can be individual shapes with one broadphase entry
//per shape or shapes of aggregates with one broadphase entry per
//aggregate.
class SingleAABBTaskSPU : public physx::PxSpuTask
{
public:

	SingleAABBTaskSPU();

	SingleAABBTaskSPU(PxsAABBManager* aabbMgr, const PxU32 numSpus, const PxsComputeAABBParams& params, const PxU32 updateType);

	virtual const char* getName() const { return "PxsAABBManager.singleAABBSpu"; }

	virtual void release();

private:

	CellComputeAABBSPUInput			PX_ALIGN(128, mCellComputeAABBSPUInput);				
	CellComputeAABBSPUOutput		PX_ALIGN(128, mCellComputeAABBSPUOutput[6]);	
	PxU64							PX_ALIGN(16,  mProfileCounters[MAX_NUM_SPU_PROFILE_ZONES]);

	PxsAABBManager* mAABBMgr;
	PxU32 mUpdateType;
};
#endif // PX_PS3

//Update all shape world-space bounds using multiple tasks or spus as required.
//The shapes can be individual shapes with one broadphase entry
//per shape or shapes of aggregates with one broadphase entry per
//aggregate.
class SingleAABBTask: public Cm::Task
{
	PX_NOCOPY(SingleAABBTask)

public:

	SingleAABBTask();
	virtual ~SingleAABBTask();

	void setUpdateType(const PxU32 updateType)
	{
		mUpdateType = updateType;
	}

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{
		mParams = params;
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsAABBManager.singleAABB"; }

	void updateNumFastMovingShapes() const;

	void setNumFastMovingShapes(const PxU32 task, const PxU32 numFastMovingShapes);

private:

	enum
	{
		eMAX_NUM_TASKS=6
	};

	enum
	{
		eTASK_UNIT_WORK_SIZE=256
	};

	PxU32 mUpdateType;

	PxsAABBManager* mAABBMgr;

	PxsComputeAABBParams mParams;

#ifdef PX_PS3
	SingleAABBTaskSPU* mAABBUpdateTaskSPU;
#endif
	SingleAABBUpdateTask mAABBUpdateTask[eMAX_NUM_TASKS];
};


//Update all actor local-space bounds using multiple tasks as required.
class ActorAABBTask : public Cm::Task
{
	PX_NOCOPY(ActorAABBTask)

public:

	ActorAABBTask();
	virtual ~ActorAABBTask();

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{
		mParams = params;
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsAABBManager.actorAABB"; }

private:

	enum
	{
		eMAX_NUM_TASKS=6
	};

	enum
	{
		eTASK_UNIT_WORK_SIZE=32
	};

	PxsAABBManager* mAABBMgr;

	PxsComputeAABBParams mParams;

	ActorAABBUpdateTask mAABBUpdateTask[eMAX_NUM_TASKS];

};

//Update all aggregate bounds using multiple tasks as required.
class AggregateAABBTask: public Cm::Task
{
	PX_NOCOPY(AggregateAABBTask)

public:

	AggregateAABBTask();
	virtual ~AggregateAABBTask();

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{
		mParams = params;
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsAABBManager.aggregateAABB"; }

	void updateNumFastMovingShapes() const;

	void setNumFastMovingShapes(const PxU32 task, const PxU32 numFastMovingShapes);

private:

	enum
	{
		eMAX_NUM_TASKS=6
	};

	enum
	{
		eTASK_UNIT_WORK_SIZE=32
	};

	PxsAABBManager* mAABBMgr;

	PxsComputeAABBParams mParams;

	AggregateAABBUpdateTask mAABBUpdateTask[eMAX_NUM_TASKS];
};

//Update the broadphase
class BPWorkTask: public Cm::Task
{
	PX_NOCOPY(BPWorkTask)

public:

	BPWorkTask()
		: mAABBMgr(NULL)
	{
	}

	virtual ~BPWorkTask()
	{
	}

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{
		mParams = params;
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsAABBManager.broadphaseWork"; }

private:

	PxsAABBManager* mAABBMgr;

	PxsComputeAABBParams mParams;
};

//Post-process step after broadphase completes.
class ProcessBPResultsTask: public Cm::Task
{
	PX_NOCOPY(ProcessBPResultsTask)

public:

	ProcessBPResultsTask()
		: mAABBMgr(NULL)
	{
	}

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{ 
		mParams = params;
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsAABBManager.processBroadphaseResults"; }

private:

	PxsComputeAABBParams mParams;

	PxsAABBManager* mAABBMgr;
};

class AggregateOverlapWorkTask: public Cm::Task
{
	PX_NOCOPY(AggregateOverlapWorkTask)

public:

	friend class AggregateOverlapTask;

	AggregateOverlapWorkTask()
		: mAABBMgr(NULL),
		  mAggregateSortedData(NULL),
		  mAggAggStartId(0),
		  mAggAggNbWorkItems(0),
		  mAggSelfStartId(0),
		  mAggSelfNbWorkItems(0),
		  mCreatedPairs(NULL),
		  mCreatedPairsSize(0),
		  mCreatedPairsCapacity(0),
		  mDeletedPairs(NULL),
		  mDeletedPairsSize(0),
		  mDeletedPairsCapacity(0)
	{
	}

	virtual void runInternal();

	virtual const char* getName() const { return "BpAABBManager.aggregateOverlapWork"; }

	void setAABBMgr(PxsAABBManager* AABBMgr);

	void setAggregateSortedData(const AggregateSortedData* aggregateSortedData)
	{
		mAggregateSortedData = aggregateSortedData;
	}

	void setAggAggWorkStartAndCount(const PxU32 startId, const PxU32 nbWorkItems)
	{
		mAggAggStartId = startId;
		mAggAggNbWorkItems = nbWorkItems;
	}

	void setAggSelfWorkStartAndCount(const PxU32 startId, const PxU32 nbWorkItems)
	{
		mAggSelfStartId = startId;
		mAggSelfNbWorkItems = nbWorkItems;
	}

	//void complete();

private:

	PxsAABBManager* mAABBMgr;

	const AggregateSortedData* mAggregateSortedData;

	PxU32 mAggAggStartId;
	PxU32 mAggAggNbWorkItems;

	PxU32 mAggSelfStartId;
	PxU32 mAggSelfNbWorkItems;

	PxvBroadPhaseOverlap* mCreatedPairs;
	PxU32 mCreatedPairsSize;
	PxU32 mCreatedPairsCapacity;
	PxvBroadPhaseOverlap* mDeletedPairs;
	PxU32 mDeletedPairsSize;
	PxU32 mDeletedPairsCapacity;
};

//Perform all aggregate self-, aggregate-aggregate and aggregate-single overlaps.
class AggregateOverlapTask: public Cm::Task
{
	PX_NOCOPY(AggregateOverlapTask)

public:

	AggregateOverlapTask()
		:  mAABBMgr(NULL)
	{
	}

	virtual void runInternal();

	virtual const char* getName() const { return "BpAABBManager.aggregateOverlap"; }

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{ 
		mParams = params;
	}

	void complete();

private:

	enum
	{
		eMAX_NUM_TASKS=6
	};

public:

	enum
	{
		eTASK_UNIT_WORK_SIZE=16
	};

	PxsComputeAABBParams mParams;

	PxsAABBManager* mAABBMgr;

	AggregateOverlapWorkTask mAggregateOverlapWorkTasks[eMAX_NUM_TASKS];
	InlineBuffer<AggregateSortedData, 256> mAggregateSortedData;
	InlineBuffer<PxcBpHandle, 1024> mRankIds;
	InlineBuffer<PxcBpHandle, 1024> mElemIds;

	void processSelfCollideAndAggregatePairs();
};

class FinishTask: public Cm::Task
{
	PX_NOCOPY(FinishTask)

public:

	FinishTask()
		:  mAABBMgr(NULL)
	{
	}

	virtual void runInternal();

	virtual const char* getName() const { return "BpAABBManager.finish"; }

	void setAABBMgr(PxsAABBManager* AABBMgr) 
	{
		mAABBMgr = AABBMgr;
	}

	void set(const PxsComputeAABBParams& params)
	{ 
		mParams = params;
	}

private:

	PxsComputeAABBParams mParams;

	PxsAABBManager* mAABBMgr;
};

} //namespace physx

#endif //PXS_AABB_MANAGER_TASKS_H

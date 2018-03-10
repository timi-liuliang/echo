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

#include "PxsMBPTasks.h"
#include "PxsContext.h"
#include "PxsAABBManager.h"
#include "PxsBroadPhaseMBP.h"
#include "CmFlushPool.h"
#include "PxcScratchAllocator.h"
#include "PsTime.h"
#include "CmEventProfiler.h"
#include "PxProfileEventId.h"

#ifdef PX_PS3
#include "CellTimerMarker.h"
#endif

//	#define DUMP_TOTAL_MBP_TIME
// 256 convex stacks: ~880 most of the time, jumps to 1400 max
// pot pourri box: ~2700 to ~3300
// boxes: ~1400 to ~1900
//	#define DUMP_MBP_TIME

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

#ifdef DUMP_TOTAL_MBP_TIME
	static PxU64 gStartTime = shdfnd::Time::getCurrentCounterValue();
#endif

///////////////////////////////////////////////////////////////////////////////

MBPUpdateWorkTask::MBPUpdateWorkTask(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler)
	: mScratchAllocator(scratchAllocator),
	  mEventProfiler(eventProfiler)
{
#ifdef PX_PS3
	mSPUTask = NULL;
#endif
}

MBPUpdateWorkTask::~MBPUpdateWorkTask()
{
#ifdef PX_PS3
	if(mSPUTask)
	{
		physx::shdfnd::AlignedAllocator<128>().deallocate(mSPUTask);
		mSPUTask = NULL;
	}
#endif
}

MBPPostUpdateWorkTask::MBPPostUpdateWorkTask(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler)
	: mScratchAllocator(scratchAllocator),
	  mEventProfiler(eventProfiler)
{
}

void MBPUpdateWorkTask::runInternal()
{
#ifdef DUMP_MBP_TIME
	PxU64 startTime = shdfnd::Time::getCurrentCounterValue();
#endif

#ifdef PX_PS3
	if(mNumSpus)
	{
#ifdef PX_PROFILE
		Cm::EventProfiler* profiler = &mEventProfiler;
		CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::GetMBPUpdateSPU());
#ifdef PX_PS3
		startTimerMarker(eUPDATE_MBP_SPU);
#endif
#endif

		if(!mSPUTask)
		{
			mSPUTask = (MBPTaskSPU*)physx::shdfnd::AlignedAllocator<128>().allocate(sizeof(MBPTaskSPU), __FILE__, __LINE__);
	#ifdef MBP_SINGLE_SPU
			const PxU32 nbSPUs = 1;
	#else
			const PxU32 nbSPUs = mNumSpus;
	#endif
			PX_PLACEMENT_NEW(mSPUTask, MBPTaskSPU)(nbSPUs);
		}
		//printf("nbSPUs: %d\n", mNumSpus);
//		mSPUTask->setSpuCount(mNumSpus);
		mSPUTask->setNbSPUs(mNumSpus);

		mSPUTask->init(mScratchAllocator, *mMBP);
		mSPUTask->setContinuation(getContinuation());
		mSPUTask->removeReference();

//		mMBP->updatePPU(getContinuation());


#ifdef PX_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::GetMBPUpdateSPU());
#ifdef PX_PS3
		stopTimerMarker(eUPDATE_MBP_SPU);
#endif
#endif
	}
	else
#endif
	{
#if 0
		if(0)
		{
			const PxU32 numTasks = 4;
			MyTestTask* tasks = (MyTestTask*)mContext->getTaskPool().allocate(sizeof(MyTestTask) * numTasks);
			for(PxU32 i=0; i<numTasks; i++)
			{
				MyTestTask* pTask = PX_PLACEMENT_NEW((&tasks[i]), MyTestTask)();
				pTask->setContinuation(getContinuation());
				pTask->removeReference();
			}
		}
#endif

/*
	const PxU32 maxTasks = task.getTaskManager()->getCpuDispatcher()->getWorkerCount();
	const PxU32 numTasks = PxMin((bodyCount/IntegrationPerThread) + 1, maxTasks);

	if(numTasks > 1)
	{
		PxsAtomIntegrateTask* tasks = (PxsAtomIntegrateTask*)mContext->getTaskPool().allocate(sizeof(PxsAtomIntegrateTask) * numTasks);
		PxI32* atomicTesters = (PxI32*)mContext->getTaskPool().allocate(sizeof(PxI32) * 2);

		PxI32* pCounter = atomicTesters;
		volatile PxI32* pNumIntegrated = &atomicTesters[1];

		*pCounter = 0;

		*pNumIntegrated = 0;

		for(PxU32 a = 0; a < numTasks; ++a)
		{
			const PxU32 remainingBodies = PxMin(bodyCount - a, IntegrationPerThread);
			PX_UNUSED(remainingBodies);
			PxsAtomIntegrateTask* pTask = PX_PLACEMENT_NEW((&tasks[a]), PxsAtomIntegrateTask)(*this, bodyArray,
							originalBodyArray, solverBodyPool, solverBodyDataPool, motionVelocityArray, accelerationArray, dt,bodyCount, pCounter, pNumIntegrated,
							&maxSolverPositionIterations, &maxSolverVelocityIterations);

			pTask->setContinuation(&task);
			pTask->removeReference();
		}
	}
	else*/

#ifdef PX_PROFILE
		Cm::EventProfiler* profiler = &mEventProfiler;
		CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::GetMBPUpdate());
#ifdef PX_PS3
		startTimerMarker(eUPDATE_MBP_PPU);
#endif
#endif

		mMBP->updatePPU(getContinuation());


#ifdef PX_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::GetMBPUpdate());
#ifdef PX_PS3
		stopTimerMarker(eUPDATE_MBP_PPU);
#endif
#endif
	}
#ifdef DUMP_MBP_TIME
	PxU64 endTime = shdfnd::Time::getCurrentCounterValue();
	printf("MBPUpdateWorkTask Time: %llu\n", endTime - startTime);
#endif
}

///////////////////////////////////////////////////////////////////////////////

void MBPPostUpdateWorkTask::runInternal()
{
#ifdef PX_PROFILE
	Cm::EventProfiler* profiler = &mEventProfiler;
	CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::GetMBPPostUpdate());
#ifdef PX_PS3
	startTimerMarker(ePOSTUPDATE_MBP_PPU);
#endif
#endif

#ifdef DUMP_MBP_TIME
	PxU64 startTime = shdfnd::Time::getCurrentCounterValue();
#endif
	{
		mMBP->postUpdatePPU(getContinuation());
	}
#ifdef DUMP_MBP_TIME
	PxU64 endTime = shdfnd::Time::getCurrentCounterValue();
	printf("MBPPostUpdateWorkTask Time: %llu\n", endTime - startTime);
#endif

#ifdef DUMP_TOTAL_MBP_TIME
	PxU64 endTime = shdfnd::Time::getCurrentCounterValue();
	printf("MBP Time: %llu\n", endTime - gStartTime);
#endif

#ifdef PX_PROFILE
	CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::GetMBPPostUpdate());
#ifdef PX_PS3
	stopTimerMarker(ePOSTUPDATE_MBP_PPU);
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

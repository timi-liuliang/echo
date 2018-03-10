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


#include "ExtCpuWorkerThread.h"
#include "ExtDefaultCpuDispatcher.h"
#include "ExtTaskQueueHelper.h"
#include "PxTask.h"
#include "PsFPU.h"

using namespace physx;

Ext::CpuWorkerThread::CpuWorkerThread()
:	mQueueEntryPool(EXT_TASK_QUEUE_ENTRY_POOL_SIZE),
	mThreadId(0)
{
}


Ext::CpuWorkerThread::~CpuWorkerThread()
{
}


void Ext::CpuWorkerThread::initialize(DefaultCpuDispatcher* ownerDispatcher)
{
	mOwner = ownerDispatcher;
}


bool Ext::CpuWorkerThread::tryAcceptJobToLocalQueue(PxBaseTask& task, Ps::Thread::Id taskSubmitionThread)
{
	if(taskSubmitionThread == mThreadId)
	{
		SharedQueueEntry* entry = mQueueEntryPool.getEntry(&task);
		if (entry)
		{
			mLocalJobList.push(*entry);
			return true;
		}
		else
			return false;
	}

	return false;
}


PxBaseTask* Ext::CpuWorkerThread::giveUpJob()
{
	return TaskQueueHelper::fetchTask(mLocalJobList, mQueueEntryPool);
}


void Ext::CpuWorkerThread::execute()
{
	mThreadId = getId();

	while (!quitIsSignalled())
    {
        mOwner->resetWakeSignal();

		PxBaseTask* task = TaskQueueHelper::fetchTask(mLocalJobList, mQueueEntryPool);

		if(!task)
			task = mOwner->fetchNextTask();
		
		if (task)
		{
			mOwner->runTask(*task);
			task->release();
		}
		else
		{
			mOwner->waitForWork();
		}
	}

	quit();
}

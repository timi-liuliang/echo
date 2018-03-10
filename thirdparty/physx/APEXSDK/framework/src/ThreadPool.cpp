/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR == 2

#include "NxApexSDK.h"

#include "PsThread.h"
#include "PsSList.h"
#include "PsSync.h"
#include "PsString.h"
#include "PsUserAllocated.h"
#include "PsAllocator.h"

#include "ThreadPool.h"
#include "ProfilerCallback.h"

#if defined(PX_WINDOWS)
#define PROFILE_TASKS 1
#else
#define PROFILE_TASKS 1
#endif

namespace physx
{
namespace apex
{

PxU32 DefaultCpuDispatcher::getAffinityMask(PxU32 numThreads)
{
	PX_UNUSED(numThreads);
	PX_ASSERT(numThreads);
#ifdef PX_X360
	switch (numThreads)
	{
	case 1:
		return 0x01;
	case 2:
		return 0x14;
	case 3:
		return 0x15;
	case 4:
		return 0x3c;
	case 5:
		return 0x3e;
	case 6:
		return 0x3f;
	default:
		return 0x0;
	}
#else
	return 0;
#endif
}

PxCpuDispatcher* createDefaultThreadPool(unsigned int numThreads)
{
	if (numThreads == 0)
	{
#if defined(PX_WINDOWS)
		numThreads = 4;
#elif defined(PX_X360)
		numThreads = 3;
#elif defined(PX_PS3)
		numThreads = 2;
#elif defined(PX_APPLE)
		numThreads = 2;
#endif
	}
	return PX_NEW(DefaultCpuDispatcher)(numThreads, 0);
}

DefaultCpuDispatcher::DefaultCpuDispatcher(PxU32 numThreads, PxU32* affinityMasks)
	: mQueueEntryPool(TASK_QUEUE_ENTRY_POOL_SIZE), mNumThreads(numThreads), mShuttingDown(false)
{
	PxU32 defaultAffinityMask = 0;

	if (!affinityMasks)
	{
		defaultAffinityMask = getAffinityMask(numThreads);
	}

	// initialize threads first, then start

	mWorkerThreads = reinterpret_cast<CpuWorkerThread*>(PX_ALLOC(numThreads * sizeof(CpuWorkerThread), PX_DEBUG_EXP("CpuWorkerThread")));
	if (mWorkerThreads)
	{
		for (PxU32 i = 0; i < numThreads; ++i)
		{
			PX_PLACEMENT_NEW(mWorkerThreads + i, CpuWorkerThread)();
			mWorkerThreads[i].initialize(this);
		}

		for (PxU32 i = 0; i < numThreads; ++i)
		{
			mWorkerThreads[i].start(shdfnd::Thread::getDefaultStackSize());
			if (affinityMasks)
			{
				mWorkerThreads[i].setAffinityMask(affinityMasks[i]);
			}
			else
			{
				mWorkerThreads[i].setAffinityMask(defaultAffinityMask);
#ifdef PX_X360
				defaultAffinityMask &= defaultAffinityMask - 1; // clear lowest bit
#endif
			}

			char threadName[32];
			string::sprintf_s(threadName, 32, "PxWorker%02d", i);
			mWorkerThreads[i].setName(threadName);
		}
	}
	else
	{
		mNumThreads = 0;
	}
}


DefaultCpuDispatcher::~DefaultCpuDispatcher()
{
	for (PxU32 i = 0; i < mNumThreads; ++i)
	{
		mWorkerThreads[i].signalQuit();
	}

	mShuttingDown = true;
	mWorkReady.set();
	for (PxU32 i = 0; i < mNumThreads; ++i)
	{
		mWorkerThreads[i].waitForQuit();
	}

	for (PxU32 i = 0; i < mNumThreads; ++i)
	{
		mWorkerThreads[i].~CpuWorkerThread();
	}

	PX_FREE(mWorkerThreads);
}


void DefaultCpuDispatcher::submitTask(PxBaseTask& task)
{
	shdfnd::Thread::Id currentThread = shdfnd::Thread::getId();

	// TODO: Could use TLS to make this more efficient
	for (PxU32 i = 0; i < mNumThreads; ++i)
		if (mWorkerThreads[i].tryAcceptJobToLocalQueue(task, currentThread))
		{
			return mWorkReady.set();
		}

	SharedQueueEntry* entry = mQueueEntryPool.getEntry(&task);
	if (entry)
	{
		mJobList.push(*entry);
		mWorkReady.set();
	}
}

void DefaultCpuDispatcher::flush( PxBaseTask& task, PxI32 targetRef)
{
	// TODO: implement
	PX_ALWAYS_ASSERT();
	PX_UNUSED(task);
	PX_UNUSED(targetRef);
}

PxU32 DefaultCpuDispatcher::getWorkerCount() const
{
	return mNumThreads;
}

void DefaultCpuDispatcher::release()
{
	NxGetApexSDK()->releaseCpuDispatcher(*this);
}


PxBaseTask* DefaultCpuDispatcher::getJob(void)
{
	return TaskQueueHelper::fetchTask(mJobList, mQueueEntryPool);
}


PxBaseTask* DefaultCpuDispatcher::stealJob()
{
	PxBaseTask* ret = NULL;

	for (PxU32 i = 0; i < mNumThreads; ++i)
	{
		ret = mWorkerThreads[i].giveUpJob();

		if (ret != NULL)
		{
			break;
		}
	}

	return ret;
}


void DefaultCpuDispatcher::resetWakeSignal()
{
	mWorkReady.reset();

	// The code below is necessary to avoid deadlocks on shut down.
	// A thread usually loops as follows:
	// while quit is not signaled
	// 1)  reset wake signal
	// 2)  fetch work
	// 3)  if work -> process
	// 4)  else -> wait for wake signal
	//
	// If a thread reaches 1) after the thread pool signaled wake up,
	// the wake up sync gets reset and all other threads which have not
	// passed 4) already will wait forever.
	// The code below makes sure that on shutdown, the wake up signal gets
	// sent again after it was reset
	//
	if (mShuttingDown)
	{
		mWorkReady.set();
	}
}








CpuWorkerThread::CpuWorkerThread()
	:	mQueueEntryPool(TASK_QUEUE_ENTRY_POOL_SIZE)
	,	mThreadId(0)
{
}


CpuWorkerThread::~CpuWorkerThread()
{
}


void CpuWorkerThread::initialize(DefaultCpuDispatcher* ownerDispatcher)
{
	mOwner = ownerDispatcher;
}


bool CpuWorkerThread::tryAcceptJobToLocalQueue(PxBaseTask& task, shdfnd::Thread::Id taskSubmitionThread)
{
	if (taskSubmitionThread == mThreadId)
	{
		SharedQueueEntry* entry = mQueueEntryPool.getEntry(&task);
		if (entry)
		{
			mLocalJobList.push(*entry);
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}


PxBaseTask* CpuWorkerThread::giveUpJob()
{
	return TaskQueueHelper::fetchTask(mLocalJobList, mQueueEntryPool);
}


void CpuWorkerThread::execute()
{
	mThreadId = getId();

	while (!quitIsSignalled())
	{
		mOwner->resetWakeSignal();

		PxBaseTask* task = TaskQueueHelper::fetchTask(mLocalJobList, mQueueEntryPool);

		if (!task)
		{
			task = mOwner->getJob();
		}

		if (!task)
		{
			task = mOwner->stealJob();
		}

		if (task)
		{
#if PHYSX_PROFILE_SDK
			if (mPvdBinding!=NULL)
			{
				task->runProfiled();
			}
			else
			{
				task->run();
			}
#else
			task->run();
#endif
			task->release();
		}
		else
		{
			mOwner->waitForWork();
		}
	}

	quit();
};



} // end pxtask namespace
} // end physx namespace

#endif // NX_SDK_VERSION_MAJOR == 2

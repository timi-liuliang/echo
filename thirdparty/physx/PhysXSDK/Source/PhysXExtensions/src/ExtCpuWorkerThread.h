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


#ifndef PX_PHYSICS_EXTENSIONS_NP_CPU_WORKER_THREAD_H
#define PX_PHYSICS_EXTENSIONS_NP_CPU_WORKER_THREAD_H

#include "CmPhysXCommon.h"
#include "PsThread.h"
#include "ExtDefaultCpuDispatcher.h"
#include "ExtSharedQueueEntryPool.h"


namespace physx
{
	class PxBaseTask;
}

namespace physx
{
namespace Ext
{
	class DefaultCpuDispatcher;


#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
								// Because of the SList member I assume

	class CpuWorkerThread : public Ps::Thread
	{
	public:
        CpuWorkerThread();
        ~CpuWorkerThread();
		
		void					initialize(DefaultCpuDispatcher* ownerDispatcher);
		void					execute();
		bool					tryAcceptJobToLocalQueue(PxBaseTask& task, Ps::Thread::Id taskSubmitionThread);
		PxBaseTask*				giveUpJob();
		Ps::Thread::Id			getWorkerThreadId() const { return mThreadId; }

	protected:
		SharedQueueEntryPool<>			mQueueEntryPool;
		DefaultCpuDispatcher*			mOwner;
		Ps::SList      				    mLocalJobList;
		Ps::Thread::Id					mThreadId;
	};

#pragma warning(pop)


} // namespace Ext

}

#endif

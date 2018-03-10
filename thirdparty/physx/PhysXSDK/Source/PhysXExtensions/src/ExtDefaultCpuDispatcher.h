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


#ifndef PX_PHYSICS_EXTENSIONS_NP_DEFAULT_CPU_DISPATCHER_H
#define PX_PHYSICS_EXTENSIONS_NP_DEFAULT_CPU_DISPATCHER_H

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PsSync.h"
#include "PsSList.h"
#include "PxDefaultCpuDispatcher.h"
#include "ExtSharedQueueEntryPool.h"


namespace physx
{
	class PxBaseTask;
}

namespace physx
{
namespace Ext
{
	class CpuWorkerThread;

#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
								// Because of the SList member I assume

	class DefaultCpuDispatcher : public PxDefaultCpuDispatcher, public Ps::UserAllocated
	{
		friend class TaskQueueHelper;

	private:
		DefaultCpuDispatcher() : mQueueEntryPool(0) {}
		~DefaultCpuDispatcher();

	public:
		DefaultCpuDispatcher(PxU32 numThreads, PxU32* affinityMasks);

		//---------------------------------------------------------------------------------
		// physx::CpuDispatcher implementation
		//---------------------------------------------------------------------------------
		virtual void submitTask(PxBaseTask& task);
		virtual PxU32 getWorkerCount() const;

		//---------------------------------------------------------------------------------
		// PxDefaultCpuDispatcher implementation
		//---------------------------------------------------------------------------------
		virtual void release();

		virtual void setRunProfiled(bool runProfiled) { mRunProfiled = runProfiled; }

		virtual bool getRunProfiled() const { return mRunProfiled; }

		//---------------------------------------------------------------------------------
		// DefaultCpuDispatcher
		//---------------------------------------------------------------------------------
		PxBaseTask*		getJob();
		PxBaseTask*		stealJob();
		PxBaseTask*		fetchNextTask();
		void			runTask(PxBaseTask& task);

    	void					waitForWork() { mWorkReady.wait(); }
	    void					resetWakeSignal();

		static void				getAffinityMasks(PxU32* affinityMasks, PxU32 threadCount);


	protected:
				CpuWorkerThread*				mWorkerThreads;
				SharedQueueEntryPool<>			mQueueEntryPool;
				Ps::SList						mJobList;
				Ps::Sync						mWorkReady;
				PxU8*							mThreadNames;
				PxU32							mNumThreads;
				bool							mShuttingDown;
				bool							mRunProfiled;
	};

#pragma warning(pop)

} // namespace Ext
}

#endif

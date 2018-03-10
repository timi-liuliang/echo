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

#ifndef PXS_MBP_TASKS_H
#define PXS_MBP_TASKS_H

#include "PsUserAllocated.h"
#include "CmTask.h"

#ifdef PX_PS3
	#include "CellMBPTask.h"
	#include "PxsBroadPhaseMBPCommon.h"
	class MBP;
	class BoxPruner;
#endif


namespace physx
{
	class PxsBroadPhaseMBP;
	class PxcScratchAllocator;

	namespace Cm
	{
		class EventProfiler;
	}

//#define MBP_SINGLE_SPU
#define MBP_USE_SCRATCHPAD

#ifdef PX_PS3
	class PxsBroadPhaseMBP;
	class MBPTaskSPU : public physx::PxSpuTask
	{
	public:
										MBPTaskSPU(PxU32 numSpusToUse=1);

		virtual const char*				getName() const { return "MBPSpu"; }
		virtual void					release();

				void					init(PxcScratchAllocator& allocator, const PxsBroadPhaseMBP& mbp);
				void					setNbSPUs(PxU32 nb);
	private:
				MBP*					mMBP;
				PxU32					mNbBoxPruners;
				const BoxPruner*		mBoxPruners[256];

#ifdef MBP_USE_SCRATCHPAD
				PxcScratchAllocator*	mAllocator;
				void*					mScratchAddresses[6];
				void*					mScratchAddressesAligned[6];
#endif
#ifdef MBP_SINGLE_SPU
				CellMBPSPUInput			PX_ALIGN(128, mCellMBPSPUInput);
				CellMBPSPUOutput		PX_ALIGN(128, mCellMBPSPUOutput);
				MBP_Overlap				PX_ALIGN(16, mOverlaps[MBP_MAX_NB_OVERLAPS]);
				PxU32					mNbDone;
#else
				CellMBPSPUInput			PX_ALIGN(128, mCellMBPSPUInput[6]);
				CellMBPSPUOutput		PX_ALIGN(128, mCellMBPSPUOutput[6]);
	#ifndef MBP_USE_SCRATCHPAD
				MBP_Overlap				PX_ALIGN(16, mOverlaps[MBP_MAX_NB_OVERLAPS*6]);	// ####### PT: TODO: compress this, or use the scratchpad
	#endif
				PxU32					mNbDone[6];
#endif
				PxU32					mRestartCount;	// Number of times we restarted the SPU program

#ifdef MBP_SINGLE_SPU
				void					initPruners(PxU32 nb, PxU32 offset);
#else
				void					initPruners(PxU32 spuIndex, PxU32 nb, PxU32 offset);
#endif
	};
#endif

	class MBPTask : public Cm::Task, public shdfnd::UserAllocated
	{
		public:
										MBPTask() : mMBP(NULL), mNumCpuTasks(0), mNumSpus(0)	{}

		PX_FORCE_INLINE	void			setBroadphase(PxsBroadPhaseMBP* mbp)		{ mMBP = mbp;	}
		PX_FORCE_INLINE void			setNumCpuTasks(const PxU32 numCpuTasks)		{ mNumCpuTasks = numCpuTasks; }
		PX_FORCE_INLINE	void			setNumSpus(const PxU32 numSpus)				{ mNumSpus = numSpus;	}

		protected:
						PxsBroadPhaseMBP*	mMBP;
						PxU32				mNumCpuTasks;
						PxU32				mNumSpus;
	};

	class MBPUpdateWorkTask : public MBPTask
	{
	public:							
							MBPUpdateWorkTask(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler);

							~MBPUpdateWorkTask();
		// BaseTask
		virtual void		runInternal();
		virtual const char*	getName() const { return "PxsMBP.updateWork"; }
		//~BaseTask

	private:
		MBPUpdateWorkTask& operator=(const MBPUpdateWorkTask&);
		PxcScratchAllocator& mScratchAllocator;
		Cm::EventProfiler& mEventProfiler;

#ifdef PX_PS3
		MBPTaskSPU*	mSPUTask;
#endif
	};

	class MBPPostUpdateWorkTask : public MBPTask
	{
	public:

							MBPPostUpdateWorkTask(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler);					

		// BaseTask
		virtual void		runInternal();
		virtual const char*	getName() const { return "PxsMBP.postUpdateWork"; }

	protected:

		PxcScratchAllocator& mScratchAllocator;
		Cm::EventProfiler& mEventProfiler;

		//~BaseTask
	private:
		MBPPostUpdateWorkTask& operator=(const MBPPostUpdateWorkTask&);
	};

} //namespace physx

#endif // PXS_MBP_TASKS_H

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


#include "PxsSAPTasks.h"
#include "PxsContext.h"
#include "PxsBroadPhaseSap.h"
#include "PsTime.h"
#include "PxsAABBManagerAux.h"

#ifdef PX_PS3
	#include "PS3Support.h"
	#include "CellComputeAABBTask.h"
	#include "CellSPUProfiling.h"
	#include "CellTimerMarker.h"
#endif //PX_PS3

using namespace physx;

///////////////////////////////////////////////////////////////////////////////


//	#define DUMP_TOTAL_SAP_TIME
// 256 convex stacks: from ~13000 down to ~2000
// pot pourri box: from ~4000 to ~700
// boxes: ~3400 to ~4000

#ifdef DUMP_TOTAL_SAP_TIME
	static PxU64 gStartTime = shdfnd::Time::getCurrentCounterValue();
#endif

void SapUpdateWorkTask::runInternal()
{
#if SPU_BP_SAP
#if FORCE_SINGLE_SPU_SAP
	const PxU32 numSpus = 1;
#else
	const PxU32 numSpus = mNumSpus;
#endif
#endif

#if SPU_BP_SAP
	if(mSAP->canRunOnSpu() && numSpus>0)
	{
		mSAP->updateSPU(numSpus, getContinuation());
	}
	else
#endif
	{
		mSAP->updatePPU(getContinuation());
	}
}

void SapPostUpdateWorkTask::runInternal()
{
#if SPU_BP_SAP
#if FORCE_SINGLE_SPU_SAP
	const PxU32 numSpus = 1;
#else
	const PxU32 numSpus = mNumSpus;
#endif
#endif

#if SPU_BP_SAP
	//const PxU32 boxesCapacity=mAABBMgr->mBPElems.getCapacity();
	//const PxU32 boxesSize=mAABBMgr->mBPElems.getSize();
	if(mSAP->canRunOnSpu() && numSpus>0)
	{
		mSAP->postUpdateSPU(numSpus, getContinuation());
	}
	else
#endif
	{
		mSAP->postUpdatePPU(getContinuation());
	}
#ifdef DUMP_TOTAL_SAP_TIME
	PxU64 endTime = shdfnd::Time::getCurrentCounterValue();
	printf("SAP Time: %llu\n", endTime - gStartTime);
#endif
}

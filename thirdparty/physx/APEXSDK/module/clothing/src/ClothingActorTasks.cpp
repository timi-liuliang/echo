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
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "ClothingActorTasks.h"
#include "ClothingActor.h"
#include "ModulePerfScope.h"


namespace physx
{
namespace apex
{
namespace clothing
{

void ClothingActorBeforeTickTask::run()
{
#ifdef PROFILE
	PIXBeginNamedEvent(0, "ClothingActorBeforeTickTask");
#endif
	//PX_ASSERT(mDeltaTime > 0.0f); // need to allow simulate(0) calls
	mActor->tickSynchBeforeSimulate_LocksPhysX(mDeltaTime, mSubstepSize, 0, mNumSubSteps);
#ifdef PROFILE
	PIXEndNamedEvent();
#endif
}



const char* ClothingActorBeforeTickTask::getName() const
{
	return "ClothingActor::BeforeTickTask";
}


// --------------------------------------------------------------------


void ClothingActorDuringTickTask::run()
{
	mActor->tickAsynch_NoPhysX();
}



const char* ClothingActorDuringTickTask::getName() const
{
	return "ClothingActor::DuringTickTask";
}

// --------------------------------------------------------------------

void ClothingActorFetchResultsTask::run()
{
#ifdef PROFILE
	PIXBeginNamedEvent(0, "ClothingActorFetchResultsTask");
#endif
	mActor->fetchResults();
	ClothingActorData& actorData = mActor->getActorData();

	if (actorData.bAllGraphicalSubmeshesFitOnSpu)
	{
		actorData.tickSynchAfterFetchResults_LocksPhysXSimple();
	}
	else
	{
		actorData.tickSynchAfterFetchResults_LocksPhysX();
	}
	//mActor->tickSynchAfterFetchResults_LocksPhysX();
#ifdef PROFILE
	PIXEndNamedEvent();
#endif

}



const char* ClothingActorFetchResultsTask::getName() const
{
	return "ClothingActor::FetchResultsTask";
}


#ifdef PX_PS3

const char* ClothingActorFetchResultsTaskSpu::getName() const
{
	return "ClothingActor::FetchResultsTaskSpu";
}



const char* ClothingActorFetchResultsTaskSimpleSpu::getName() const
{
	return "ClothingActor::FetchResultsTaskSimpleSpu";
}



const char* ClothingActorSkinPhysicsTaskSimpleSpu::getName() const
{
	return "ClothingActor::SkinPhysicsTaskSimpleSpu";
}



void ClothingActorLockingTasks::run()
{
	mActor->applyLockingTasks();
}



const char* ClothingActorLockingTasks::getName() const
{
	return "ClothingActor::ClothingActorLockingTasks";
}


#endif // PX_PS3


}
}
}

#endif

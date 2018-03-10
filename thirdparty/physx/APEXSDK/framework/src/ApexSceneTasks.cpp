/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexSceneTasks.h"
#include "FrameworkPerfScope.h"

#include "PsTime.h"

namespace physx
{
namespace apex
{

// --------- LODComputeBenefitTask

const char* LODComputeBenefitTask::getName() const
{
	return AST_LOD_COMPUTE_BENEFIT;
}

void LODComputeBenefitTask::run()
{
	PxF32 totalSceneValue = 0.0f;

	mScene->mSumBenefit = 0.0f;

	NiModuleScene** sceneHndStop = mScene->mModuleScenes.end();
	for (NiModuleScene** sceneHnd = mScene->mModuleScenes.begin(); sceneHnd < sceneHndStop; ++sceneHnd)
	{
		NiModuleScene* scene = *sceneHnd;
		scene->computeBenefit();
		mScene->mSumBenefit += scene->getCachedBenefit();
		if (!scene->getNxModule()->getLODEnabled())
		{
			continue;	// Automatic LOD not enabled for this scene; skip
		}
		const PxF32 sceneValue = scene->getCachedBenefit() * scene->getNxModule()->getLODBenefitValue();
		totalSceneValue += sceneValue;
	}

	const PxF32 budgetScale = totalSceneValue > mScene->mTotalResource ? mScene->mTotalResource / totalSceneValue : 1.0f;

	mScene->mUsedResource = 0.0f;

	for (NiModuleScene** sceneHnd = mScene->mModuleScenes.begin(); sceneHnd < sceneHndStop; ++sceneHnd)
	{
		NiModuleScene* scene = *sceneHnd;
		const physx::PxF32 budget = scene->getNxModule()->getLODEnabled() ? budgetScale * scene->getCachedBenefit() * scene->getNxModule()->getLODBenefitValue() : 2147483647.0f;
		mScene->mUsedResource += scene->setResource(budget, budget, 1.0f);
	}

	PX_PROFILER_PLOT((PxU32)mScene->mUsedResource, "ApexSceneLODUsedResource");
	PX_PROFILER_PLOT((PxU32)mScene->mSumBenefit, "ApexSceneLODSumBenefit");
}



// --------- PhysXSimulateTask

PhysXSimulateTask::PhysXSimulateTask(ApexScene& scene, CheckResultsTask& checkResultsTask) 
: mScene(&scene)
, mElapsedTime(0.0f)
, mFollowingTask(NULL)
, mCheckResultsTask(checkResultsTask) 
#if NX_SDK_VERSION_MAJOR == 3
, mScratchBlock(NULL)
, mScratchBlockSize(0)
#endif
{}

PhysXSimulateTask::~PhysXSimulateTask() 
{
#if NX_SDK_VERSION_MAJOR == 3
	mScratchBlock = NULL;
	mScratchBlockSize = 0;
#endif
}

const char* PhysXSimulateTask::getName() const
{
	return AST_PHYSX_SIMULATE;
}


void PhysXSimulateTask::run()
{
#if NX_SDK_VERSION_MAJOR == 2
	mScene->acquirePhysXLock();
#endif

	// record the pretick APEX time
	ApexStatValue dataVal;
	physx::PxU64 qpc = Time::getCurrentCounterValue();
	dataVal.Float = ApexScene::ticksToMilliseconds(mScene->mApexSimulateTickCount, qpc);
	APEX_CHECK_STAT_TIMER("--------- ApexBeforeTickTime (mApexSimulateTickCount)");
	
	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mScene->mApexSimulateTickCount = qpc;

	mScene->setApexStatValue(ApexScene::ApexBeforeTickTime, dataVal);

	// start the PhysX simulation time timer
	APEX_CHECK_STAT_TIMER("--------- Set mPhysXSimulateTickCount");
	mScene->mPhysXSimulateTickCount = Time::getCurrentCounterValue();


	if (mScene->mPhysXScene)
	{
		PX_ASSERT(mElapsedTime >= 0.0f);
#if NX_SDK_VERSION_MAJOR == 2
		mScene->mPhysXScene->simulate(mElapsedTime);
#elif NX_SDK_VERSION_MAJOR == 3
		PX_ASSERT(mElapsedTime > 0);
		SCOPED_PHYSX_LOCK_WRITE(*mScene);
		mScene->mPhysXScene->simulate(mElapsedTime, &mCheckResultsTask, mScratchBlock, mScratchBlockSize, false);
#endif
	}


#if NX_SDK_VERSION_MAJOR == 2
	if (mFollowingTask != NULL)
	{
		mFollowingTask->removeReference();
	}

	mScene->releasePhysXLock();
#endif
}



void PhysXSimulateTask::setElapsedTime(PxF32 elapsedTime)
{
	PX_ASSERT(elapsedTime >= 0.0f);
	mElapsedTime = elapsedTime;
}



void PhysXSimulateTask::setFollowingTask(physx::PxBaseTask* following)
{
	mFollowingTask = following;
}



// --------- CheckResultsTask

const char* CheckResultsTask::getName() const
{
	return AST_PHYSX_CHECK_RESULTS;
}



void CheckResultsTask::run()
{
	SCOPED_PHYSX_LOCK_WRITE(*mScene);

#if !APEX_DURING_TICK_TIMING_FIX
	{
		// mark the end of the "during tick" simulation time
		ApexStatValue dataVal;
		{
			physx::PxU64 qpc = Time::getCurrentCounterValue();
			dataVal.Float = ApexScene::ticksToSeconds(mScene->mApexSimulateTickCount, qpc);
			APEX_CHECK_STAT_TIMER("--------- ApexDuringTickTime (mApexSimulateTickCount)");

			APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
			mScene->mApexSimulateTickCount = qpc;
		}
		mScene->setApexStatValue(ApexScene::ApexDuringTickTime, dataVal);
	}
#endif

	if (mScene->mPhysXScene)
	{
#if NX_SDK_VERSION_MAJOR == 2
		mScene->mPhysXScene->checkResults(NX_ALL_FINISHED, true);
#else
		mScene->mPhysXScene->checkResults(true);
#endif
	}

	// get the PhysX simulation time and add it to the ApexStats
	{
		ApexStatValue dataVal;
		{
			physx::PxU64 qpc = Time::getCurrentCounterValue();
			dataVal.Float = ApexScene::ticksToMilliseconds(mScene->mPhysXSimulateTickCount, qpc);
			APEX_CHECK_STAT_TIMER("--------- PhysXSimulationTime (mPhysXSimulateTickCount)");
		}

		mScene->setApexStatValue(ApexScene::PhysXSimulationTime, dataVal);
	}
}




// --------- FetchResultsTask

const char* FetchResultsTask::getName() const
{
	return AST_PHYSX_FETCH_RESULTS;
}



void FetchResultsTask::run()
{
}

void FetchResultsTask::setFollowingTask(physx::PxBaseTask* following)
{
	mFollowingTask = following;
	if (mFollowingTask)
	{
		mFollowingTask->addReference();
	}
}



/*
* \brief Called by dispatcher after Task has been run.
*
* If you re-implement this method, you must call this base class
* version before returning.
*/
void FetchResultsTask::release()
{
	physx::PxTask::release();

	// copy mFollowingTask into local variable, because it might be overwritten
	// as soon as mFetchResultsReady.set() is called (and before removeReference() is called on it)
	physx::PxBaseTask* followingTask = mFollowingTask;
	mFollowingTask = NULL;

	// Allow ApexScene::fetchResults() to run (potentially unblocking game thread)
	mScene->mFetchResultsReady.set();
	
	// remove reference to the scene completion task submitted in NxApexScene::simulate
	// this must be done after the scene's mFetchResultsReady event is set so that the
	// app's completion task can be assured that fetchResults is ready to run
	if (followingTask)
	{
		followingTask->removeReference();
	}	
}



#if APEX_DURING_TICK_TIMING_FIX
// --------- DuringTickCompleteTask

const char* DuringTickCompleteTask::getName() const
{
	return AST_DURING_TICK_COMPLETE;
}



void DuringTickCompleteTask::run()
{
	SCOPED_PHYSX_LOCK_WRITE(*mScene);

	// mark the end of the "during tick" simulation time
	ApexStatValue dataVal;
	physx::PxU64 qpc = Time::getCurrentCounterValue();
	dataVal.Float = ApexScene::ticksToMilliseconds(mScene->mApexSimulateTickCount, qpc);
	APEX_CHECK_STAT_TIMER("--------- ApexDuringTickTime (mApexSimulateTickCount)");

	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mScene->mApexSimulateTickCount = qpc;
	
	mScene->setApexStatValue(ApexScene::ApexDuringTickTime, dataVal);
}
#endif


// --------- PhysXBetweenStepsTask

const char* PhysXBetweenStepsTask::getName() const
{
	return AST_PHYSX_BETWEEN_STEPS;
}



void PhysXBetweenStepsTask::run()
{
	PX_ASSERT(mSubStepSize > 0.0f);
	PX_ASSERT(mNumSubSteps > 0);
#if NX_SDK_VERSION_MAJOR == 2
	NxScene* scene = mScene.getPhysXScene();
#else
	PxScene* scene = mScene.getPhysXScene();
#endif

	if (scene != NULL)
	{
		while (mSubStepNumber < mNumSubSteps)
		{
			PX_PROFILER_PERF_SCOPE("ApexSceneManualSubstep");
			// fetch the first substep
			PxU32 errorState = 0;
#if NX_SDK_VERSION_MAJOR == 2
			scene->fetchResults(NX_ALL_FINISHED, true, &errorState);
#else
			{
				SCOPED_PHYSX_LOCK_WRITE(mScene);
				scene->fetchResults(true, &errorState);
			}
#endif
			PX_ASSERT(errorState == 0);

			for (PxU32 i = 0; i < mScene.mModuleScenes.size(); i++)
			{
				PX_PROFILER_PERF_SCOPE("ModuleSceneManualSubstep");
				mScene.mModuleScenes[i]->interStep(mSubStepNumber, mNumSubSteps);
			}

			// run the next substep
			{
				SCOPED_PHYSX_LOCK_WRITE(mScene);
				scene->simulate(mSubStepSize);
			}

			mSubStepNumber++;
		}
	}

	mLast->removeReference(); // decrement artificially high ref count that prevented checkresults from being executed
}



void PhysXBetweenStepsTask::setSubstepSize(PxF32 substepSize, PxU32 numSubSteps)
{
	mSubStepSize = substepSize;
	mNumSubSteps = numSubSteps;
}



void PhysXBetweenStepsTask::setFollower(PxU32 substepNumber, physx::PxTask* last)
{
	mSubStepNumber = substepNumber;
	mLast = last;

	setContinuation(last);
}



} // namespace apex
} // namespace physx

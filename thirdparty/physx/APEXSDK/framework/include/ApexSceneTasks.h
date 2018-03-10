/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SCENE_TASKS_H
#define APEX_SCENE_TASKS_H

#include "ApexScene.h"

#include "PsAllocator.h"

namespace physx
{
namespace apex
{

class LODComputeBenefitTask : public physx::PxTask, public UserAllocated
{
public:
	LODComputeBenefitTask(ApexScene& scene) : mScene(&scene) {}

	const char* getName() const;
	void run();

protected:
	ApexScene* mScene;
};



class PhysXSimulateTask : public physx::PxTask, public UserAllocated
{
public:
	PhysXSimulateTask(ApexScene& scene, CheckResultsTask& checkResultsTask); 
	~PhysXSimulateTask();
	
	const char* getName() const;
	void run();
	void setElapsedTime(PxF32 elapsedTime);
	void setFollowingTask(physx::PxBaseTask* following);

#if NX_SDK_VERSION_MAJOR == 3
	void setScratchBlock(void* scratchBlock, PxU32 size)
	{
		mScratchBlock = scratchBlock;
		mScratchBlockSize = size;
	}
#endif

protected:
	ApexScene* mScene;
	PxF32 mElapsedTime;

	physx::PxBaseTask* mFollowingTask;
	CheckResultsTask& mCheckResultsTask;

#if NX_SDK_VERSION_MAJOR == 3
	void*			mScratchBlock;
	PxU32			mScratchBlockSize;
#endif

private:
	PhysXSimulateTask& operator=(const PhysXSimulateTask&);
};



class CheckResultsTask : public physx::PxTask, public UserAllocated
{
public:
	CheckResultsTask(ApexScene& scene) : mScene(&scene) {}

	const char* getName() const;
	void run();

protected:
	ApexScene* mScene;
};



class FetchResultsTask : public physx::PxTask, public UserAllocated
{
public:
	FetchResultsTask(ApexScene& scene) 
	:	mScene(&scene)
	,	mFollowingTask(NULL)
	{}

	const char* getName() const;
	void run();

	/**
	* \brief Called by dispatcher after Task has been run.
	*
	* If you re-implement this method, you must call this base class
	* version before returning.
	*/
	void release();

	void setFollowingTask(physx::PxBaseTask* following);

protected:
	ApexScene*					mScene;
	physx::PxBaseTask*	mFollowingTask;
};


/**  
*	This task is solely meant to record the duration of APEX's "during tick" tasks.
*	It could be removed and replaced with only the check results task if it is found
*	to be a performance issue.
*/
#if APEX_DURING_TICK_TIMING_FIX
class DuringTickCompleteTask : public physx::PxTask, public UserAllocated
{
public:
	DuringTickCompleteTask(ApexScene& scene) : mScene(&scene) {}

	const char* getName() const;
	void run();

protected:
	ApexScene* mScene;
};
#endif

/* This tasks loops all intermediate steps until the final fetchResults can be called */
class PhysXBetweenStepsTask : public physx::PxLightCpuTask, public UserAllocated
{
public:
	PhysXBetweenStepsTask(ApexScene& scene) : mScene(scene), mSubStepSize(0.0f),
		mNumSubSteps(0), mSubStepNumber(0), mLast(NULL) {}

	const char* getName() const;
	void run();
	void setSubstepSize(PxF32 substepSize, PxU32 numSubSteps);
	void setFollower(PxU32 substepNumber, physx::PxTask* last);

protected:
	ApexScene& mScene;
	PxF32 mSubStepSize;
	PxU32 mNumSubSteps;

	PxU32 mSubStepNumber;
	physx::PxTask* mLast;

private:
	PhysXBetweenStepsTask& operator=(const PhysXBetweenStepsTask&);
};

}
}

#endif

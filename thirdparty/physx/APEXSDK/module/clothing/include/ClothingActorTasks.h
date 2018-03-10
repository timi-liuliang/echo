/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ACTOR_TASKS_H
#define CLOTHING_ACTOR_TASKS_H

#include "PxTask.h"

#ifdef PX_PS3
#include "PxSpuTask.h"
#include "ClothingSpuPrograms.h"
#endif

namespace physx
{
namespace apex
{
namespace clothing
{

class ClothingActor;
class ClothingActorData;


class ClothingActorBeforeTickTask : public physx::PxLightCpuTask
{
public:
	ClothingActorBeforeTickTask(ClothingActor* actor) : mActor(actor), mDeltaTime(0.0f), mSubstepSize(0.0f), mNumSubSteps(0) {}

	PX_INLINE void setDeltaTime(PxF32 simulationDelta, PxF32 substepSize, PxU32 numSubSteps)
	{
		mDeltaTime = simulationDelta;
		mSubstepSize = substepSize;
		mNumSubSteps = numSubSteps;
	}

	virtual void        run();
	virtual const char* getName() const;

private:
	ClothingActor* mActor;
	PxF32 mDeltaTime;
	PxF32 mSubstepSize;
	PxU32 mNumSubSteps;
};



class ClothingActorDuringTickTask : public physx::PxTask
{
public:
	ClothingActorDuringTickTask(ClothingActor* actor) : mActor(actor) {}

	virtual void		run();
	virtual const char*	getName() const;

private:
	ClothingActor* mActor;
};



class ClothingActorFetchResultsTask : public physx::PxLightCpuTask
{
public:
	ClothingActorFetchResultsTask(ClothingActor* actor) : mActor(actor) {}

	virtual void		run();
	virtual const char*	getName() const;

private:
	ClothingActor* mActor;
};


#ifdef PX_PS3


class ClothingActorFetchResultsTaskSpu : public physx::PxSpuTask
{
public:
	ClothingActorFetchResultsTaskSpu() : PxSpuTask(gSpuClothingFetchResults.elfStart, gSpuClothingFetchResults.elfSize) {}

	virtual const char* getName() const;
};



class ClothingActorFetchResultsTaskSimpleSpu : public physx::PxSpuTask
{
public:
	ClothingActorFetchResultsTaskSimpleSpu() : PxSpuTask(gSpuClothingFetchResultsSimple.elfStart, gSpuClothingFetchResultsSimple.elfSize) {}

	virtual const char* getName() const;
};



class ClothingActorSkinPhysicsTaskSimpleSpu : public physx::PxSpuTask
{
public:
	ClothingActorSkinPhysicsTaskSimpleSpu() : PxSpuTask(gSpuClothingSkinPhysicsSimple.elfStart, gSpuClothingSkinPhysicsSimple.elfSize)
	{
	}

	virtual const char* getName() const;
};



class ClothingActorLockingTasks : public physx::PxLightCpuTask
{
public:
	ClothingActorLockingTasks(ClothingActor* actor) : mActor(actor) {}

	virtual void		run();
	virtual const char*	getName() const;

private:
	ClothingActor* mActor;
};

#endif // PX_PS3

}
} // namespace apex
} // namespace physx

#endif

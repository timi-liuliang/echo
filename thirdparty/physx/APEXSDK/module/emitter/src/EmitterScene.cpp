/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "PsShare.h"
#include "EmitterScene.h"
#include "NiApexScene.h"
#include "ModulePerfScope.h"

#include "NxLock.h"

namespace physx
{
namespace apex
{
namespace emitter
{

EmitterScene::EmitterScene(ModuleEmitter& module, NiApexScene& scene, NiApexRenderDebug* debugRender, NxResourceList& list) :
	mSumBenefit(0.0f),
	mDebugRender(debugRender)
{
	mModule = &module;
	mApexScene = &scene;
	list.add(*this);		// Add self to module's list of EmitterScenes

	/* Initialize reference to EmitterDebugRenderParams */
	{
		NX_READ_LOCK(*mApexScene);
		mDebugRenderParams = DYNAMIC_CAST(DebugRenderParams*)(mApexScene->getDebugRenderParams());
	}
	PX_ASSERT(mDebugRenderParams);
	NxParameterized::Handle handle(*mDebugRenderParams), memberHandle(*mDebugRenderParams);
	int size;

	if (mDebugRenderParams->getParameterHandle("moduleName", handle) == NxParameterized::ERROR_NONE)
	{
		handle.getArraySize(size, 0);
		handle.resizeArray(size + 1);
		if (handle.getChildHandle(size, memberHandle) == NxParameterized::ERROR_NONE)
		{
			memberHandle.initParamRef(EmitterDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to EmitterDebugRenderParams */
	NxParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mEmitterDebugRenderParams = DYNAMIC_CAST(EmitterDebugRenderParams*)(refPtr);
	PX_ASSERT(mEmitterDebugRenderParams);
}

EmitterScene::~EmitterScene()
{
}

void EmitterScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	for (physx::PxU32 i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->visualize(*mDebugRender);
	}
#endif
}

void EmitterScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

#if NX_SDK_VERSION_MAJOR == 2
void EmitterScene::setModulePhysXScene(NxScene* scene)
#elif NX_SDK_VERSION_MAJOR == 3
void EmitterScene::setModulePhysXScene(PxScene* scene)
#endif
{
	if (scene)
	{
		for (physx::PxU32 i = 0 ; i < mActorArray.size() ; i++)
		{
			EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
			actor->setPhysXScene(scene);
		}
	}
	else
	{
		for (physx::PxU32 i = 0 ; i < mActorArray.size() ; i++)
		{
			EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
			actor->setPhysXScene(NULL);
		}
	}

	mPhysXScene = scene;
}


physx::PxF32	EmitterScene::getBenefit()
{
	if (!mActorArray.size())
	{
		return 0.0f;
	}
	ApexActor** ss = mActorArray.begin();
	ApexActor** ee = mActorArray.end();

	// the address of a EmitterActorBase* and ApexActor* must be identical, otherwise the reinterpret cast will break
	PX_ASSERT(ss == NULL || ((void*)DYNAMIC_CAST(EmitterActorBase*)(*ss) == (void*)(*ss)));

	mSumBenefit = LODCollection<EmitterActorBase>::computeSumBenefit(reinterpret_cast<EmitterActorBase**>(ss), reinterpret_cast<EmitterActorBase**>(ee));
	return mSumBenefit;
}

physx::PxF32	EmitterScene::setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit)
{
	PX_UNUSED(maxRemaining);

	physx::PxF32 resourceUsed = LODCollection<EmitterActorBase>::distributeResource(reinterpret_cast<EmitterActorBase**>(mActorArray.begin()), reinterpret_cast<EmitterActorBase**>(mActorArray.end()), mSumBenefit, relativeBenefit, suggested);
	return resourceUsed;
}

void EmitterScene::submitTasks(PxF32 /*elapsedTime*/, PxF32 /*substepSize*/, PxU32 /*numSubSteps*/)
{
	for (physx::PxU32 i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->submitTasks();
	}
}

void EmitterScene::setTaskDependencies()
{
	for (physx::PxU32 i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->setTaskDependencies();
	}
}

// Called by ApexScene simulation thread after PhysX scene is stepped. All
// actors in the scene are render-locked.
void EmitterScene::fetchResults()
{
	PX_PROFILER_PERF_SCOPE("EmitterSceneFetchResults");

	for (physx::PxU32 i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->fetchResults();
	}
}

}
}
} // namespace physx::apex

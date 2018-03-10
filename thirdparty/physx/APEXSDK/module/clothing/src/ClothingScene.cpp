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

#include "NxApex.h"
#include "ClothingScene.h"
#include "ClothingActor.h"
#include "ClothingAsset.h"
#include "ClothingCooking.h"
#include "NiApexScene.h"
#include "ClothingRenderProxy.h"

#include "DebugRenderParams.h"
#include "ProfilerCallback.h"

#include "SimulationPhysX3.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxCompartmentDesc.h"
#include "NxScene.h"
#endif
#include "PsThread.h"
#include "PsShare.h"
#include "PsAtomic.h"

#ifdef PX_WINDOWS
#include "PxGpuDispatcher.h"
#endif

#include "PVDBinding.h"

namespace physx
{
namespace apex
{
namespace clothing
{

ClothingScene::ClothingScene(ModuleClothing& _module, NiApexScene& scene, NiApexRenderDebug* renderDebug, NxResourceList& list)
	: mModule(&_module)
	, mApexScene(&scene)
	, mPhysXScene(NULL)
#if NX_SDK_VERSION_MAJOR == 2
	, mNextClothHwCompartmentId(physx::PxU32(-1))
	, mNextClothSwCompartmentId(physx::PxU32(-1))
#endif
	, mSumBenefit(0)
	, mWaitForSolverTask(NULL)
	, mSimulationTask(NULL)
	, mSceneRunning(0)
	, mRenderDebug(renderDebug)
	, mDebugRenderParams(NULL)
	, mClothingDebugRenderParams(NULL)
	, mCurrentCookingTask(NULL)
	, mCurrentSimulationDelta(0)
	, mAverageSimulationFrequency(0.0f)
#ifndef _DEBUG
	, mFramesCount(0)
	, mSimulatedTime(0.f)
	, mTimestep(0.f)
#endif
	, mCpuFactory(NULL, NULL)
#ifdef PX_WINDOWS
	, mGpuFactory(NULL, NULL)
	, mPhysXGpuIndicator(NULL)
#endif
{
	mClothingBeforeTickStartTask.setScene(this);
	list.add(*this);

	/* Initialize reference to ClothingDebugRenderParams */
	mDebugRenderParams = DYNAMIC_CAST(DebugRenderParams*)(mApexScene->getDebugRenderParams());
	PX_ASSERT(mDebugRenderParams);
	NxParameterized::Handle handle(*mDebugRenderParams), memberHandle(*mDebugRenderParams);
	int size;

	if (mDebugRenderParams->getParameterHandle("moduleName", handle) == NxParameterized::ERROR_NONE)
	{
		handle.getArraySize(size, 0);
		handle.resizeArray(size + 1);
		if (handle.getChildHandle(size, memberHandle) == NxParameterized::ERROR_NONE)
		{
			memberHandle.initParamRef(ClothingDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to ClothingDebugRenderParams */
	NxParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mClothingDebugRenderParams = DYNAMIC_CAST(ClothingDebugRenderParams*)(refPtr);
	PX_ASSERT(mClothingDebugRenderParams);

	mLastSimulationDeltas.reserve(mModule->getAvgSimFrequencyWindowSize());



	mWaitForSolverTask = PX_NEW(WaitForSolverTask)(this);

	mSimulationTask = PX_NEW(ClothingPhysX3SceneSimulateTask)(mApexScene, this, mModule, NiGetApexSDK()->getPvdBinding());
	mSimulationTask->setWaitTask(mWaitForSolverTask);
}



ClothingScene::~ClothingScene()
{
}



void ClothingScene::simulate(PxF32 elapsedTime)
{
	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);
		clothingActor->waitForFetchResults();
	}

	if (mLastSimulationDeltas.size() < mLastSimulationDeltas.capacity())
	{
		mCurrentSimulationDelta = mLastSimulationDeltas.size();
		mLastSimulationDeltas.pushBack(elapsedTime);
	}
	else if (mLastSimulationDeltas.size() > 0)
	{
		mCurrentSimulationDelta = (mCurrentSimulationDelta + 1) % mLastSimulationDeltas.size();
		mLastSimulationDeltas[mCurrentSimulationDelta] = elapsedTime;
	}

	PxF32 temp = 0.0f;
	for (PxU32 i = 0; i < mLastSimulationDeltas.size(); i++)
	{
		temp += mLastSimulationDeltas[i];
	}

	if (temp > 0.0f)
	{
		mAverageSimulationFrequency = (PxF32)(mLastSimulationDeltas.size()) / temp;
	}
	else
	{
		mAverageSimulationFrequency = 0.0f;
	}

	tickRenderProxies();
}



bool ClothingScene::needsManualSubstepping() const
{
	// we could test if any of them is being simulated, but assuming some sane budget settings
	// there will always be >0 clothing actors simulated if there are any present

	if (!mModule->allowApexWorkBetweenSubsteps())
	{
		return false;
	}

	// PH: new rule. The actual simulation object needs to request this too!
	bool manualSubstepping = false;
	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);
		manualSubstepping |= clothingActor->needsManualSubstepping();
	}
	return manualSubstepping;
}



void ClothingScene::interStep(PxU32 substepNumber, PxU32 maxSubSteps)
{
	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);

		if (clothingActor->needsManualSubstepping())
		{
			clothingActor->tickSynchBeforeSimulate_LocksPhysX(0.0f, 0.0f, substepNumber, maxSubSteps);
			clothingActor->skinPhysicsMesh(maxSubSteps > 1, (PxF32)(substepNumber + 1) / (PxF32)maxSubSteps);
			clothingActor->updateConstrainPositions_LocksPhysX();
			clothingActor->applyCollision_LocksPhysX();
		}
	}
}



void ClothingScene::submitTasks(PxF32 elapsedTime, PxF32 substepSize, PxU32 numSubSteps)
{
	PxTaskManager* taskManager = mApexScene->getTaskManager();
	const bool isFinalStep = mApexScene->isFinalStep();

	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);
		clothingActor->initBeforeTickTasks(elapsedTime, substepSize, numSubSteps);

		if (isFinalStep)
		{
			clothingActor->submitTasksDuring(taskManager);
		}
	}

	taskManager->submitUnnamedTask(mClothingBeforeTickStartTask);

	mSimulationTask->setDeltaTime(elapsedTime);
	if (elapsedTime > 0.0f)
	{
		taskManager->submitUnnamedTask(*mSimulationTask);
		taskManager->submitUnnamedTask(*mWaitForSolverTask);
	}
}



void ClothingScene::setTaskDependencies()
{
	PxTaskManager* taskManager = mApexScene->getTaskManager();
	const PxTaskID lodTick = taskManager->getNamedTask(AST_LOD_COMPUTE_BENEFIT);
	const PxTaskID physxTick = taskManager->getNamedTask(AST_PHYSX_SIMULATE);
	PxTask* physxTickTask = taskManager->getTaskFromID(physxTick);

#if APEX_DURING_TICK_TIMING_FIX
	const PxTaskID duringFinishedId = taskManager->getNamedTask(AST_DURING_TICK_COMPLETE);
#else
	const PxTaskID duringFinishedId = taskManager->getNamedTask(AST_PHYSX_CHECK_RESULTS);
#endif

	bool startSimulateTask = mSimulationTask->getDeltaTime() > 0;
	PxTaskID duringStartId = startSimulateTask ? mSimulationTask->getTaskID() : physxTick;

	const bool isFinalStep = mApexScene->isFinalStep();

	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ApexActor* actor = mActorArray[i];
		ClothingActor* clothingActor = static_cast<ClothingActor*>(actor);

		PxTask* dependentTask = physxTickTask;
		if (startSimulateTask)
		{
			dependentTask = mSimulationTask;
			mSimulationTask->startAfter(mClothingBeforeTickStartTask.getTaskID());
		}

		clothingActor->setTaskDependenciesBefore(dependentTask);

		if (isFinalStep)
		{
			// PH:	daisy chain the during tasks to not trash other (=PhysX) tasks' cache etc.
			// HL:	found a case where duringTick becomes the bottleneck because of the daisy chaining

			/*duringStartId = */clothingActor->setTaskDependenciesDuring(duringStartId, duringFinishedId);
		}
	}

	mClothingBeforeTickStartTask.startAfter(lodTick);
	mClothingBeforeTickStartTask.finishBefore(physxTick);

	if(startSimulateTask)
	{
		mSimulationTask->finishBefore(physxTick);
		mWaitForSolverTask->startAfter(mSimulationTask->getTaskID());
		mWaitForSolverTask->startAfter(duringFinishedId);
		mWaitForSolverTask->finishBefore(taskManager->getNamedTask(AST_PHYSX_FETCH_RESULTS));
	}
}



void ClothingScene::fetchResults()
{
	if (!mApexScene->isFinalStep())
	{
		return;
	}

	PX_PROFILER_PERF_SCOPE("ClothingScene::fetchResults");

	for (PxI32 i = (physx::PxI32)mActorArray.size()-1; i >= 0 ; --i)
	{
		ClothingActor* actor = static_cast<ClothingActor*>(mActorArray[(physx::PxU32)i]);
		if (actor->getClothSolverMode() != ClothSolverMode::v3)
		{
			actor->startFetchTasks();
		}
	}

	// make sure to start cooking tasks if possible (and delete old ones)
	submitCookingTask(NULL);

	if (!mModule->allowAsyncFetchResults())
	{
		for (PxU32 i = 0; i < mActorArray.size(); i++)
		{
			ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);
			clothingActor->waitForFetchResults();
		}
	}

	// TBD - if we need to send callbacks to the user, add them here
}



physx::PxF32	ClothingScene::getBenefit()
{
	if (!getNxModule()->getLODEnabled())
	{
		return 0.0f;
	}

	ApexActor** ss = mActorArray.begin();
	ApexActor** ee = mActorArray.end();

	// the address of a ClothingActor* and ApexActor* must be identical, otherwise the reinterpret cast will break
	PX_ASSERT(ss == NULL || ((void*)DYNAMIC_CAST(ClothingActor*)(*ss) == (void*)(*ss)));

	mSumBenefit = LODCollection<ClothingActor>::computeSumBenefit(reinterpret_cast<ClothingActor**>(ss), reinterpret_cast<ClothingActor**>(ee));
	return mSumBenefit;
}



PxF32	ClothingScene::setResource(PxF32 suggested, PxF32 maxRemaining, PxF32 relativeBenefit)
{
	PX_UNUSED(maxRemaining);

	ClothingActor** begin = reinterpret_cast<ClothingActor**>(mActorArray.begin());
	ClothingActor** end   = reinterpret_cast<ClothingActor**>(mActorArray.end());
	PxF32 simulationCost = 0;
	if (getNxModule()->getLODEnabled())
	{
		simulationCost = LODCollection<ClothingActor>::distributeResource(begin, end, mSumBenefit, relativeBenefit, suggested);
	}

	// HL: avoid iterating over actors here for small function calls
	// if possible, as it's relatively expensive on xbox.
	// TODO: how to return the current cost effectivley

	return simulationCost;
}



#if NX_SDK_VERSION_MAJOR == 2
void ClothingScene::setModulePhysXScene(NxScene* newPhysXScene)
{
	if (mPhysXScene == newPhysXScene)
	{
		return;
	}

	// init has not been called yet on mModule, so canUseGpuPhysics is not yet initialized!!
	// if (mModule->canUseGpuPhysics())

	// PH: This is somewhat unsatisfying.
	//   1) We cannot release compartments because there's no API for that
	//   2) We cannot reuse compartments from an existing scene because we can't tell which ones were created by APEX and which ones by the user
	//   3) There is a maximum number of compartments a scene can hold (at least sw compartments)
	// if the user gives us NULL scene and then again the same scene as before it will contain more and more compartments, even though only
	// the ones created most recently are actually used. And it can run out of compartment space...

	mClothHwCompartments.clear();
	mNextClothHwCompartmentId = 0;

	mClothSwCompartments.clear();
	mNextClothSwCompartmentId = 0;

	NxScene* oldPhysXScene = mPhysXScene;

#elif NX_SDK_VERSION_MAJOR == 3

void ClothingScene::setModulePhysXScene(PxScene* newPhysXScene)
{
	if (mPhysXScene == newPhysXScene)
	{
		return;
	}

	PxScene* oldPhysXScene = mPhysXScene;

#endif

	mPhysXScene = newPhysXScene;
	for (PxU32 i = 0; i < mActorArray.size(); ++i)
	{
		// downcast
		ClothingActor* actor = static_cast<ClothingActor*>(mActorArray[i]);

		actor->setPhysXScene(newPhysXScene);
	}

	mClothingAssetsMutex.lock();
	for (PxU32 i = 0 ; i < mClothingAssets.size(); i++)
	{
		mClothingAssets[i]->hintSceneDeletion(oldPhysXScene);

		mClothingAssets[i]->releaseCookedInstances();
	}
	mClothingAssets.clear();
	mClothingAssetsMutex.unlock();

#ifdef PX_WINDOWS
	{
		if (mGpuFactory.factory != NULL && oldPhysXScene != NULL)
		{
			mSimulationTask->clearGpuSolver();
			if (newPhysXScene != NULL)
			{
#if NX_SDK_VERSION_MAJOR == 2
				mModule->releaseClothFactory(mApexScene->getTaskManager()->getGpuDispatcher()->getCudaContextManager());
#elif NX_SDK_VERSION_MAJOR == 3
				PX_ASSERT(mApexScene->getTaskManager() == oldPhysXScene->getTaskManager());
				mModule->releaseClothFactory(oldPhysXScene->getTaskManager()->getGpuDispatcher()->getCudaContextManager());
#endif
			}
			mGpuFactory.clear();
		}
	}
#endif
}



void ClothingScene::release()
{
	mModule->releaseNiModuleScene(*this);
}



void ClothingScene::visualize()
{
#ifdef WITHOUT_DEBUG_VISUALIZE
#else
	if (!mClothingDebugRenderParams->Actors)
	{
		return;
	}

	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		// downcast
		ClothingActor* actor = static_cast<ClothingActor*>(mActorArray[i]);
		actor->visualize();
	}
#endif
}



NxModule* ClothingScene::getNxModule()
{
	return mModule;
}



#if NX_SDK_VERSION_MAJOR == 2
NxCompartment* ClothingScene::getClothCompartment_LocksPhysX(bool hw)
{
#if !defined(PX_WINDOWS)
	PX_UNUSED(hw);
	return NULL;
#else
	if (mModule->getMaxNumCompartments() == 0 || mPhysXScene == NULL)
	{
		return NULL;
	}

	NxCompartment* compartment = NULL;

	mApexScene->acquirePhysXLock();

	if (hw)
	{
		if (mClothHwCompartments.size() <= mNextClothHwCompartmentId)
		{
			PX_ASSERT(mClothHwCompartments.size() == mNextClothHwCompartmentId);
			NxCompartmentDesc compartmentDesc;
			compartmentDesc.type = NX_SCT_CLOTH;
			compartmentDesc.deviceCode = (physx::PxU32)NX_DC_PPU_AUTO_ASSIGN;
			compartment = mPhysXScene->createCompartment(compartmentDesc);
			if (compartment != NULL)
			{
				mClothHwCompartments.pushBack(compartment);
			}
			else
			{
				APEX_INTERNAL_ERROR("GPU compartment could not be created.");
			}
		}
		else
		{
			compartment = mClothHwCompartments[mNextClothHwCompartmentId];
		}

		if (compartment != NULL)
		{
			mNextClothHwCompartmentId = (mNextClothHwCompartmentId + 1) % mModule->getMaxNumCompartments();
		}
	}
	else
	{
		if (mClothSwCompartments.size() <= mNextClothSwCompartmentId)
		{
			PX_ASSERT(mClothSwCompartments.size() == mNextClothSwCompartmentId);
			NxCompartmentDesc compartmentDesc;
			compartmentDesc.type = NX_SCT_CLOTH;
			compartmentDesc.deviceCode = (PxU32)NX_DC_CPU;
			compartment = mPhysXScene->createCompartment(compartmentDesc);
			if (compartment != NULL)
			{
				mClothSwCompartments.pushBack(compartment);
			}
			else
			{
				APEX_INTERNAL_ERROR("CPU compartment could not be created.");
			}
		}
		else
		{
			compartment = mClothSwCompartments[mNextClothSwCompartmentId];
		}

		if (compartment != NULL)
		{
			mNextClothSwCompartmentId = (mNextClothSwCompartmentId + 1) % mModule->getMaxNumCompartments();
		}
	}

	mApexScene->releasePhysXLock();

	return compartment;
#endif
}
#endif // NX_SDK_VERSION_MAJOR == 2



bool ClothingScene::isSimulating() const
{
	if (mApexScene != NULL)
	{
		return mApexScene->isSimulating();
	}

	return false;
}



void ClothingScene::registerAsset(ClothingAsset* asset)
{
	mClothingAssetsMutex.lock();
	for (PxU32 i = 0; i < mClothingAssets.size(); i++)
	{
		if (mClothingAssets[i] == asset)
		{
			mClothingAssetsMutex.unlock();
			return;
		}
	}
	mClothingAssets.pushBack(asset);
	mClothingAssetsMutex.unlock();
}



void ClothingScene::unregisterAsset(ClothingAsset* asset)
{
	// remove assets from assets list
	mClothingAssetsMutex.lock();
	for (PxI32 i = (physx::PxI32)mClothingAssets.size() - 1; i >= 0; i--)
	{
		if (mClothingAssets[(physx::PxU32)i] == asset)
		{
			mClothingAssets.replaceWithLast((physx::PxU32)i);
		}
	}
	mClothingAssetsMutex.unlock();

	removeRenderProxies(asset);
}



void ClothingScene::removeRenderProxies(ClothingAsset* asset)
{
	// delete all render proxies that have the RenderMeshAsset
	// of this ClothingAsset
	mRenderProxiesLock.lock();
	PxU32 numGraphicalMeshes = asset->getNumGraphicalMeshes();
	for (PxU32 i = 0; i < numGraphicalMeshes; ++i)
	{
		NiApexRenderMeshAsset* renderMeshAsset = asset->getGraphicalMesh(i);

		Array<ClothingRenderProxy*>& renderProxies = mRenderProxies[renderMeshAsset];
		for (PxI32 i = (physx::PxI32)renderProxies.size()-1; i >= 0 ; --i)
		{
			ClothingRenderProxy* renderProxy = renderProxies[(physx::PxU32)i];
			if (renderProxy->getTimeInPool() > 0)
			{
				PX_DELETE(renderProxies[(physx::PxU32)i]);
			}
			else
			{
				renderProxy->notifyAssetRelease();
			}
		}
		renderProxies.clear();
		mRenderProxies.erase(renderMeshAsset);
	}
	mRenderProxiesLock.unlock();
}



#if NX_SDK_VERSION_MAJOR == 2
void ClothingScene::registerCompartment(NxCompartment* compartment)
{
	for (PxU32 i = 0; i < mRegisteredCompartments.size(); i++)
	{
		if (mRegisteredCompartments[i].compartment == compartment)
		{
			mRegisteredCompartments[i].refCount++;
			return;
		}
	}

	RegisteredCompartment comp;
	comp.compartment = compartment;
	comp.refCount = 1;
	mRegisteredCompartments.pushBack(comp);
}



void ClothingScene::unregisterCompartment(NxCompartment* compartment)
{
	for (physx::PxI32 i = mRegisteredCompartments.size() - 1; i >= 0; i--)
	{
		if (mRegisteredCompartments[i].compartment == compartment)
		{
			if (mRegisteredCompartments[i].refCount == 1)
			{
				mRegisteredCompartments.replaceWithLast(i);
			}
			else
			{
				mRegisteredCompartments[i].refCount--;
			}

			return;
		}
	}
}
#endif



PxU32 ClothingScene::submitCookingTask(ClothingCookingTask* newTask)
{
	mCookingTaskMutex.lock();

	ClothingCookingTask** currPointer = &mCurrentCookingTask;
	ClothingCookingTask* lastTask = NULL;

	PxU32 numRunning = 0;
	PxU32 numReleased = 0;

	while (*currPointer != NULL)
	{
		PX_ASSERT(lastTask == NULL || currPointer == &lastTask->nextTask);
		if ((*currPointer)->readyForRelease())
		{
			ClothingCookingTask* releaseMe = *currPointer;
			*currPointer = releaseMe->nextTask;
			delete releaseMe;
			numReleased++;
		}
		else
		{
			lastTask = *currPointer;
			numRunning += lastTask->waitsForBeingScheduled() ? 0 : 1;
			currPointer = &(*currPointer)->nextTask;
		}
	}

	// set the linked list
	*currPointer = newTask;
	if (newTask != NULL)
	{
		PX_ASSERT(mApexScene->getTaskManager() != NULL);
		newTask->initCooking(*mApexScene->getTaskManager(), NULL);
	}

	if (numRunning == 0 && mCurrentCookingTask != NULL)
	{
		PX_ASSERT(mCurrentCookingTask->waitsForBeingScheduled());
		mCurrentCookingTask->removeReference();
	}

	mCookingTaskMutex.unlock();

	return numReleased;
}



void ClothingScene::destroy()
{
	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);
		clothingActor->waitForFetchResults();
	}

	removeAllActors();

	mClothingAssetsMutex.lock();
	for (physx::PxU32 i = 0 ; i < mClothingAssets.size(); i++)
	{
		if (mPhysXScene != NULL)
		{
			mClothingAssets[i]->hintSceneDeletion(mPhysXScene);
		}

		// for PhysX3: making sure that fabrics (in assets) are released before the factories (in mSimulationTask)
		mClothingAssets[i]->releaseCookedInstances();
	}
	mClothingAssets.clear();
	mClothingAssetsMutex.unlock();

	// clear render list
	for(HashMap<NiApexRenderMeshAsset*, Array<ClothingRenderProxy*> >::Iterator iter = mRenderProxies.getIterator(); !iter.done(); ++iter)
	{
		Array<ClothingRenderProxy*>& renderProxies = iter->second;

		for (PxI32 i = (physx::PxI32)renderProxies.size()-1; i >= 0 ; --i)
		{
			PxU32 timeInPool = renderProxies[(physx::PxU32)i]->getTimeInPool();
			if (timeInPool > 0)
			{
				PX_DELETE(renderProxies[(physx::PxU32)i]);
				renderProxies.replaceWithLast((physx::PxU32)i);
			}
			else
			{
				// actually the scene is released, but we just want to make sure
				// that the render proxy deletes itself when it's returned next time
				renderProxies[(physx::PxU32)i]->notifyAssetRelease();
			}
		}

		renderProxies.clear();
	}
	//mRenderProxies.clear();

	while (mCurrentCookingTask != NULL)
	{
		submitCookingTask(NULL);
		physx::Thread::sleep(0); // wait for remaining cooking tasks to finish
	}

	if (mSimulationTask != NULL)
	{
		setModulePhysXScene(NULL); // does some cleanup necessary here. Only needed when module gets deleted without the apex scene being deleted before!
		PX_DELETE(mSimulationTask);
		mSimulationTask = NULL;
	}

	if (mWaitForSolverTask != NULL)
	{
		PX_DELETE(mWaitForSolverTask);
		mWaitForSolverTask = NULL;
	}

	{
	if (mCpuFactory.factory != NULL)
	{
		mCpuFactory.clear();
	}

#if defined(PX_WINDOWS) && APEX_CUDA_SUPPORT
		PX_ASSERT(mGpuFactory.factory == NULL);

		NiApexSDK* apexSdk = NiGetApexSDK();
		apexSdk->unregisterPhysXIndicatorGpuClient(mPhysXGpuIndicator);
		mPhysXGpuIndicator = NULL;
#endif
	}

	mApexScene->moduleReleased(*this);
	delete this;
}



void ClothingScene::ClothingBeforeTickStartTask::run()
{
#ifdef PROFILE
	PIXBeginNamedEvent(0, "ClothingBeforeTickStartTask");
#endif
	for (PxU32 i = 0; i < m_pScene->mActorArray.size(); ++i)
	{
		ClothingActor* actor = static_cast<ClothingActor*>(m_pScene->mActorArray[i]);

		actor->startBeforeTickTask();
	}
#ifdef PROFILE
	PIXEndNamedEvent();
#endif
}



const char* ClothingScene::ClothingBeforeTickStartTask::getName() const
{
	return "ClothingScene::ClothingBeforeTickStartTask";
}



ClothFactory ClothingScene::getClothFactory(bool& useCuda)
{
#ifdef PX_WINDOWS
	if (useCuda)
	{
		if (mGpuFactory.factory == NULL)
		{
			PxCudaContextManager* contextManager = NULL;
			PxGpuDispatcher* gpuDispatcher = mApexScene->getTaskManager()->getGpuDispatcher();
			if (gpuDispatcher != NULL)
			{
				contextManager = gpuDispatcher->getCudaContextManager();
			}

#if APEX_CUDA_SUPPORT
			if (contextManager != NULL)
			{
				mGpuFactory = mModule->createClothFactory(contextManager);
				if (mGpuFactory.factory != NULL)
				{
					NiApexSDK* apexSdk = NiGetApexSDK();
					mPhysXGpuIndicator = apexSdk->registerPhysXIndicatorGpuClient();
				}
			}
#endif
		}

		//APEX_DEBUG_INFO("Gpu Factory %p", mGpuFactory);
		if (mGpuFactory.factory != NULL)
		{
			return mGpuFactory;
		}
		else
		{
			APEX_DEBUG_INFO("Gpu Factory could not be created");
			useCuda = false;
		}
	}
	
	if (!useCuda)
#else
	PX_UNUSED(useCuda);
#endif
	{
		if (mCpuFactory.factory == NULL)
		{
			mCpuFactory = mModule->createClothFactory(NULL);
		}

		//APEX_DEBUG_INFO("Cpu Factory %p", mCpuFactory.factory);
		return mCpuFactory;
	}

#ifdef PX_WINDOWS
	PX_ALWAYS_ASSERT_MESSAGE("this code path is unreachable, at least it used to be.");
	return ClothFactory(NULL, NULL);
#endif
}



cloth::Solver* ClothingScene::getClothSolver(bool useCuda)
{
	ClothFactory factory(NULL, NULL);
#ifdef PX_WINDOWS
	if (useCuda)
	{
		factory = mGpuFactory;
	}
	else
#else
	PX_UNUSED(useCuda);
#endif
	{
		factory = mCpuFactory;
	}

	PX_ASSERT(factory.factory != NULL);
	if (factory.factory != NULL)
	{
		return mSimulationTask->getSolver(factory);
	}

	return NULL;
}



void ClothingScene::lockScene()
{
	mSceneLock.lock();

	if (mSceneRunning == 1)
	{
		APEX_INVALID_OPERATION("The scene is running while the scene write lock is being acquired!");
		PX_ALWAYS_ASSERT();
	}
}



void ClothingScene::unlockScene()
{
	mSceneLock.unlock();
}



void ClothingScene::setSceneRunning(bool on)
{
#ifndef _DEBUG
	PxI32 newValue;
	if (on)
	{
		APEX_CHECK_STAT_TIMER("--------- Start ClothingSimulationTime");
		mClothingSimulationTime.getElapsedSeconds();

		newValue = shdfnd::atomicIncrement(&mSceneRunning);
	}
	else
	{
		ApexStatValue dataVal;
		dataVal.Float = (PxF32)(1000.0f * mClothingSimulationTime.getElapsedSeconds());
		APEX_CHECK_STAT_TIMER("--------- Stop ClothingSimulationTime");
		mApexScene->setApexStatValue(NiApexScene::ClothingSimulationTime, dataVal);

		// Warn if simulation time was bigger than timestep for 10 or more consecutive frames
		PxF32 simulatedTime = 1000.0f * mApexScene->getElapsedTime();
		if (simulatedTime < dataVal.Float)
		{
			mFramesCount++;
			mSimulatedTime	+= simulatedTime;
			mTimestep		+= dataVal.Float;
		}

		if (mFramesCount >= 10)
		{
			float averageSimulatedTime = mSimulatedTime / (PxF32)mFramesCount;
			float averageTimestep = mTimestep / (PxF32)mFramesCount;
			APEX_DEBUG_WARNING("Cloth complexity in scene is too high to be simulated in real time for 10 consecutive frames. (Average Delta Time: %f ms, Average Simulation Time: %f ms)", 
								averageSimulatedTime, averageTimestep);
			mFramesCount	= 0;
			mSimulatedTime	= 0.f;
			mTimestep		= 0.f;
		}

		newValue = shdfnd::atomicDecrement(&mSceneRunning);
	}

	if (newValue != (on ? 1 : 0))
	{
		APEX_INTERNAL_ERROR("scene running state was not tracked properly!: on = %s, prevValue = %d", on ? "true" : "false", newValue);
	}
#else
	PX_UNUSED(on);
#endif
}



void ClothingScene::embeddedPostSim()
{
	for (PxU32 i = 0; i < mActorArray.size(); i++)
	{
		ClothingActor* clothingActor = static_cast<ClothingActor*>(mActorArray[i]);
		if (clothingActor->getClothSolverMode() == ClothSolverMode::v3)
		{
			clothingActor->startFetchTasks();
		}
	}
}



ClothingRenderProxy* ClothingScene::getRenderProxy(NiApexRenderMeshAsset* rma, bool useFallbackSkinning, bool useCustomVertexBuffer, const HashMap<PxU32, ApexSimpleString>& overrideMaterials, const PxVec3* morphTargetNewPositions, const PxU32* morphTargetVertexOffsets)
{
	if (rma == NULL)
	{
		return NULL;
	}

	ClothingRenderProxy* renderProxy = NULL;


	mRenderProxiesLock.lock();
	Array<ClothingRenderProxy*>& renderProxies = mRenderProxies[rma];
	for (PxU32 i = 0; i < renderProxies.size(); ++i)
	{
		ClothingRenderProxy* proxyInPool = renderProxies[i];
		if (
			proxyInPool->getTimeInPool() > 0 && // proxy is available
			useFallbackSkinning == proxyInPool->usesFallbackSkinning() &&
			useCustomVertexBuffer == proxyInPool->usesCustomVertexBuffer() &&
			morphTargetNewPositions == proxyInPool->getMorphTargetBuffer() &&
			proxyInPool->overrideMaterialsEqual(overrideMaterials)
			)
		{
			renderProxy = proxyInPool;
			break;
		}
	}

	// no corresponding proxy in pool, so create one
	if (renderProxy == NULL)
	{
		renderProxy = PX_NEW(ClothingRenderProxy)(rma, useFallbackSkinning, useCustomVertexBuffer, overrideMaterials, morphTargetNewPositions, morphTargetVertexOffsets, this);
		renderProxies.pushBack(renderProxy);
	}

	renderProxy->setTimeInPool(0);
	mRenderProxiesLock.unlock();

	return renderProxy;
}



void ClothingScene::tickRenderProxies()
{
	PX_PROFILER_PERF_SCOPE("ClothingScene::tickRenderProxies");
	mRenderProxiesLock.lock();

	for(HashMap<NiApexRenderMeshAsset*, Array<ClothingRenderProxy*> >::Iterator iter = mRenderProxies.getIterator(); !iter.done(); ++iter)
	{
		Array<ClothingRenderProxy*>& renderProxies = iter->second;

		for (PxI32 i = (physx::PxI32)renderProxies.size()-1; i >= 0 ; --i)
		{
			PxU32 timeInPool = renderProxies[(physx::PxU32)i]->getTimeInPool();

			if (timeInPool > 0)
			{
				if (timeInPool > mModule->getMaxTimeRenderProxyInPool() + 1) // +1 because we add them with time 1
				{
					PX_DELETE(renderProxies[(physx::PxU32)i]);
					renderProxies.replaceWithLast((physx::PxU32)i);
				}
				else
				{
					renderProxies[(physx::PxU32)i]->setTimeInPool(timeInPool+1);
				}
			}
		}
	}

	mRenderProxiesLock.unlock();
}

}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

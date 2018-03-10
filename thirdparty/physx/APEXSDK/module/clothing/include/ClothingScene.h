/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_SCENE_H
#define CLOTHING_SCENE_H

#include "ApexContext.h"
#include "NiModule.h"
#include "ApexInterface.h"

#include "ClothStructs.h"

#include "PsSync.h"
#include "PxTask.h"
#include "PsTime.h"


namespace physx
{

#if defined(PX_WINDOWS)
class PhysXGpuIndicator;
#endif

namespace cloth
{
class Cloth;
class Factory;
class Solver;
}


namespace apex
{

class DebugRenderParams;
class NiApexRenderMeshAsset;
class ApexSimpleString;

namespace clothing
{
class ModuleClothing;
class ClothingAsset;
class ClothingCookingTask;
class ClothingDebugRenderParams;

class ClothingRenderProxy;


class ClothingScene : public NiModuleScene, public ApexContext, public NxApexResource, public ApexResource
{
public:
	ClothingScene(ModuleClothing& module, NiApexScene& scene, NiApexRenderDebug* renderDebug, NxResourceList& list);
	~ClothingScene();

	/* NiModuleScene */
	virtual void			simulate(PxF32 elapsedTime);
	virtual bool			needsManualSubstepping() const;
	virtual void			interStep(PxU32 substepNumber, PxU32 maxSubSteps);
	virtual void			submitTasks(PxF32 elapsedTime, PxF32 substepSize, PxU32 numSubSteps);
	virtual void			setTaskDependencies();
	virtual void			fetchResults();
	PxF32					getBenefit();
	PxF32					setResource(PxF32 suggested, PxF32 maxRemaining, PxF32 relativeBenefit);

#if NX_SDK_VERSION_MAJOR == 2
	void					setModulePhysXScene(NxScene*);
	NxScene*				getModulePhysXScene() const
	{
		return mPhysXScene;
	}
#elif NX_SDK_VERSION_MAJOR == 3
	void					setModulePhysXScene(PxScene*);
	PxScene*				getModulePhysXScene() const
	{
		return mPhysXScene;
	}
#endif

	void					release();
	void					visualize();

	virtual NxModule*		getNxModule();

	virtual NxApexSceneStats* getStats()
	{
		return 0;
	}

	bool							lockRenderResources()
	{
		renderLockAllActors();	// Lock options not implemented yet
		return true;
	}

	bool							unlockRenderResources()
	{
		renderUnLockAllActors();	// Lock options not implemented yet
		return true;
	}

	/* NxApexResource */
	PxU32					getListIndex() const
	{
		return m_listIndex;
	}
	void					setListIndex(NxResourceList& list, PxU32 index)
	{
		m_listIndex = index;
		m_list = &list;
	}

#if NX_SDK_VERSION_MAJOR == 2
	// returns the next compartment to use, cycles through compartments
	NxCompartment*			getClothCompartment_LocksPhysX(bool hw);
#endif

	bool					isSimulating() const;

	void					registerAsset(ClothingAsset* asset);
	void					unregisterAsset(ClothingAsset* asset);
	void					removeRenderProxies(ClothingAsset* asset);

#if NX_SDK_VERSION_MAJOR == 2
	void					registerCompartment(NxCompartment* compartment);
	void					unregisterCompartment(NxCompartment* compartment);
#endif

	PxU32					submitCookingTask(ClothingCookingTask* newTask);

	PxF32					getAverageSimulationFrequency()
	{
		return mAverageSimulationFrequency;
	}

	NiApexScene*			getApexScene()
	{
		return mApexScene;
	}

	const ClothingDebugRenderParams* getDebugRenderParams() const
	{
		return mClothingDebugRenderParams;
	}

	ClothFactory			getClothFactory(bool& useCuda);
	cloth::Solver*			getClothSolver(bool useCuda);

	void					lockScene();
	void					unlockScene();

	void					setSceneRunning(bool on);

	void					embeddedPostSim();

	// render proxy pool
	ClothingRenderProxy*	getRenderProxy(NiApexRenderMeshAsset* rma, bool useFallbackSkinning, bool useCustomVertexBuffer, const HashMap<PxU32, ApexSimpleString>& overrideMaterials, const PxVec3* morphTargetNewPositions, const PxU32* morphTargetVertexOffsets);
	void					tickRenderProxies();

protected:

	ModuleClothing* 		mModule;
	NiApexScene* 			mApexScene;
#if NX_SDK_VERSION_MAJOR == 2
	NxScene* 				mPhysXScene;
	Array<NxCompartment*>	mClothHwCompartments;
	Array<NxCompartment*>	mClothSwCompartments;
	PxU32					mNextClothHwCompartmentId;
	PxU32					mNextClothSwCompartmentId;

	struct RegisteredCompartment
	{
		NxCompartment*	compartment;
		PxU32			refCount;
	};
	Array<RegisteredCompartment>	mRegisteredCompartments;
#elif NX_SDK_VERSION_MAJOR == 3
	PxScene*				mPhysXScene;
#endif

	Array<ClothingAsset*>	mClothingAssets;
	shdfnd::Mutex			mClothingAssetsMutex;

	PxF32					mSumBenefit;

	void					destroy();

	class ClothingBeforeTickStartTask : public PxTask
	{
	public:
		ClothingBeforeTickStartTask() : m_pScene(NULL)
		{
		}

		void setScene(ClothingScene* pScene)
		{
			m_pScene = pScene;
		}

		virtual void run();
		virtual const char* getName() const;

		ClothingScene* m_pScene;
	} mClothingBeforeTickStartTask;

private:

	class WaitForSolverTask*				mWaitForSolverTask;
	class ClothingPhysX3SceneSimulateTask*	mSimulationTask;
	shdfnd::Mutex							mSceneLock;
	PxI32									mSceneRunning;

	NiApexRenderDebug*						mRenderDebug;

	DebugRenderParams*						mDebugRenderParams;
	ClothingDebugRenderParams*				mClothingDebugRenderParams;

	ClothingCookingTask*					mCurrentCookingTask;
	physx::Mutex							mCookingTaskMutex;

	Array<PxF32>							mLastSimulationDeltas;
	PxU32									mCurrentSimulationDelta;
	PxF32									mAverageSimulationFrequency;

#ifndef _DEBUG
	// For statistics
	PxU32									mFramesCount;
	PxF32									mSimulatedTime;
	PxF32									mTimestep;
#endif

	physx::shdfnd::Time						mClothingSimulationTime;

	ClothFactory							mCpuFactory;
#ifdef PX_WINDOWS
	ClothFactory							mGpuFactory;
	PhysXGpuIndicator*						mPhysXGpuIndicator;
#endif

	Mutex									mRenderProxiesLock;
	shdfnd::HashMap<NiApexRenderMeshAsset*, shdfnd::Array<ClothingRenderProxy*> > mRenderProxies;

	friend class ModuleClothing;
	friend class ClothingActor;
	friend class SimulationNxCommon;
};

}
} // namespace apex
} // namespace physx

#endif // CLOTHING_SCENE_H

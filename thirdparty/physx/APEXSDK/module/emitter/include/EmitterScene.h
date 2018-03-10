/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_SCENE_H__
#define __EMITTER_SCENE_H__

#include "NxApex.h"

#include "ModuleEmitter.h"
#include "NiApexSDK.h"
#include "NiModule.h"
#include "ApexInterface.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"
#include "ApexActor.h"

#include "DebugRenderParams.h"
#include "EmitterDebugRenderParams.h"

#include "PxTask.h"

namespace physx
{
namespace apex
{

class NiApexScene;

namespace emitter
{

class ModuleEmitter;


/* Each Emitter Actor should derive this class, so the scene can deal with it */
class EmitterActorBase : public ApexActor, public LODNode
{
public:
	virtual bool		isValid()
	{
		return mValid;
	}
	virtual void		tick() = 0;
	virtual void					visualize(NiApexRenderDebug& renderDebug) = 0;
	//virtual void					setPhysXScene( NxScene * ) = 0;
	//virtual NxScene*				getPhysXScene() const = 0;

	virtual void		submitTasks() = 0;
	virtual void		setTaskDependencies() = 0;
	virtual void		fetchResults() = 0;

protected:
	EmitterActorBase() : mValid(false) {}

	bool mValid;
};

class EmitterScene : public NiModuleScene, public ApexContext, public NxApexResource, public ApexResource
{
public:
	EmitterScene(ModuleEmitter& module, NiApexScene& scene, NiApexRenderDebug* debugRender, NxResourceList& list);
	~EmitterScene();

	/* NiModuleScene */
	void				visualize();
#if NX_SDK_VERSION_MAJOR == 2
	void				setModulePhysXScene(NxScene* s);
	NxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	NxScene*			mPhysXScene;
#elif NX_SDK_VERSION_MAJOR == 3
	void				setModulePhysXScene(PxScene* s);
	PxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	PxScene*			mPhysXScene;
#endif
	physx::PxF32		setResource(physx::PxF32, physx::PxF32, physx::PxF32);
	physx::PxF32		getBenefit();
	NxModule*			getNxModule()
	{
		return mModule;
	}

	void				fetchResults();

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
	physx::PxU32		getListIndex() const
	{
		return m_listIndex;
	}
	void				setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	void				release()
	{
		mModule->releaseNiModuleScene(*this);
	}

	void				submitTasks(PxF32 elapsedTime, PxF32 substepSize, PxU32 numSubSteps);
	void				setTaskDependencies();

protected:
	void                destroy();

	ModuleEmitter*		mModule;
	NiApexScene*		mApexScene;

	physx::PxF32		mSumBenefit;
private:
	NiApexRenderDebug* mDebugRender;

	DebugRenderParams*					mDebugRenderParams;
	EmitterDebugRenderParams*			mEmitterDebugRenderParams;

	friend class ModuleEmitter;
	friend class ApexEmitterActor;
	friend class GroundEmitterActor;
	friend class ImpactEmitterActor;
};

}
}
} // end namespace physx::apex

#endif

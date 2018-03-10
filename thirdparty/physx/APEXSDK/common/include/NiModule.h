/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NI_MODULE_H
#define NI_MODULE_H

#include "NxApexSDK.h"
#include "ApexLOD.h"

#if NX_SDK_VERSION_MAJOR == 2
class NxCompartment;
class NxScene;
#endif

typedef struct CUgraphicsResource_st* CUgraphicsResource;

#define PARAM_CLASS_DECLARE_FACTORY(clas) clas ## Factory m ## clas ## Factory;
#define PARAM_CLASS_REGISTER_FACTORY(t, clas) t->registerFactory(m ## clas ## Factory);
#define PARAM_CLASS_REMOVE_FACTORY(t, clas) t->removeFactory(clas::staticClassName()); clas::freeParameterDefinitionTable(t);

namespace physx
{

#if NX_SDK_VERSION_MAJOR == 3
class PxScene;
#endif

namespace debugger 
{
namespace comm 
{
class PvdDataStream;
}
}

namespace apex
{

class NiApexScene;
class NiModuleScene;
class NiApexRenderDebug;
class ApexActor;
class NxApexActor;
class NiApexModuleCachedData;
struct NxApexSceneStats;

/**
Framework interface to modules for use by ApexScenes and the ApexSDK
*/
class NiModule
{
public:
	NiModule(void)
	{
		mParent = NULL;
		mCreateOk = true;
	}
	virtual ~NiModule(void)
	{
		if ( mParent )
		{
			mParent->notifyChildGone(this);
		}
	}
	/**
	Cause a module to free all of its resources.  Only callable from ApexSDK::releaseModule()
	*/
	virtual void destroy() = 0;

	/**
	Notification from ApexSDK when it is being destructed and will, therefore, be releasing all modules
	*/
	virtual void notifyReleaseSDK(void) 
	{

	}

	/**
	Inits Classes sent to Pvd from this module
	*/
	virtual void initPvdClasses(physx::debugger::comm::PvdDataStream& /*pvdDataStream*/)
	{
	}

	/**
	Inits Instances when Pvd connects
	*/
	virtual void initPvdInstances(physx::debugger::comm::PvdDataStream& /*pvdDataStream*/)
	{
	}

	/**
	Called by a newly created NxApexScene to instantiate an NiModuleScene.  Can also be
	called when modules are created after scenes.  If your module does
	not create ApexActors, this function can return NULL.

	The debug render that the scene is to use is also passed.
	*/
	virtual NiModuleScene* createNiModuleScene(NiApexScene& apexScene, NiApexRenderDebug*) = 0;

	/**
	Release an NiModuleScene.  Only called when an NxApexScene has been released.
	All actors and other resources in the context should be released.
	*/
	virtual void           releaseNiModuleScene(NiModuleScene& moduleScene) = 0;

	/**
	Module can provide a data cache for its objects.  It is valid to return NULL.
	*/
	virtual NiApexModuleCachedData*	getModuleDataCache()
	{
		return NULL;
	}

	/**
	Returns the number of assets force loaded by all of the module's loaded assets
	Default impl returns 0, maybe this should be something really bad
	*/
	virtual physx::PxU32          forceLoadAssets()
	{
		return 0;
	}

	virtual ApexActor*     getApexActor(NxApexActor*, NxAuthObjTypeID) const
	{
		return NULL;
	}

	virtual void setParent(NiModule *parent)
	{
		mParent = parent;
	}

	virtual void notifyChildGone(NiModule *child)
	{
		PX_UNUSED(child);
	}

	void setCreateOk(bool state)
	{
		mCreateOk = state;
	}

	bool isCreateOk(void) const
	{
		return mCreateOk;
	}

	bool			mCreateOk;
	NiModule	*mParent;
};

class NiModuleScene : public LODNode
{
public:

	/**
	NiModuleScene::simulate() is called by ApexScene::simulate() from the context of the
	APEX API call (typically the main game thread).  Context sensitive code should run here.
	Note that the task manager will be executing tasks while simulate() is running, so it must
	be thread safe.
	\param elapsedTime The time passed to the NxApexScene::simulate call
	*/
	virtual void		simulate(physx::PxF32 elapsedTime)
	{
		PX_UNUSED(elapsedTime);
	}

	/**
	\brief If the PhysX scene runs with multiple substeps, modules can request manual substepping
	*/
	virtual bool		needsManualSubstepping() const
	{
		return false;
	}

	virtual void		interStep(PxU32 substepNumber, PxU32 maxSubSteps)
	{
		PX_UNUSED(substepNumber);
		PX_UNUSED(maxSubSteps);
	}

	/**
	NiModuleScene::submitTasks() is called by ApexScene::simulate() at the start of every
	simulation step.  Each module should submit tasks within this function call, though
	they are not restricted from submitting tasks later if they require.
	\param elapsedTime The time passed to the NxApexScene::simulate call
	\param numSubSteps Will be >1 if manual sub stepping is turned on, 1 otherwise
	*/
	virtual void		submitTasks(PxF32 elapsedTime, PxF32 substepSize, PxU32 numSubSteps) = 0;

	/**
	NiModuleScene::setTaskDependencies() is called by ApexScene::simulate() after every
	module has had the opportunity to submit their tasks to the task manager.  Therefore it
	is safe to set dependencies in this function based on cross-module TaskID APIs.
	*/
	virtual void		setTaskDependencies() {}

	/**
	NiModuleScene::fetchResults() is called by ApexScene::fetchResults() from the context of
	the APEX API call (typically the main game thread).  All renderable actors are locked by
	the scene for the length of this function call.
	*/
	virtual void		fetchResults() = 0;

	virtual void		fetchResultsPreRenderLock() {}
	virtual void		fetchResultsPostRenderUnlock() {}

#if NX_SDK_VERSION_MAJOR == 2
	/**
	Called by ApexScene when its NxScene reference has changed.  Provided pointer can be NULL.
	*/
	virtual void		setModulePhysXScene(NxScene* s) = 0;
	virtual NxScene*	getModulePhysXScene() const = 0;
#else
	virtual void		setModulePhysXScene(PxScene* s) = 0;
	virtual PxScene*	getModulePhysXScene() const = 0;
#endif // NX_SDK_VERSION_MAJOR == 2

	/**
	Called by ApexScene when it has been released.  The NiModuleScene must call its
	module's releaseNiModuleScene() method.
	*/
	virtual void		release() = 0;

	/**
	\brief Visualize the module's contents, using the new debug rendering facilities.

	This gets called from NxApexScene::updateRenderResources
	*/
	virtual void		visualize() = 0;

	/**
	\brief Returns the corresponding NxModule.

	This allows to get to information like the module name.
	*/
	virtual NxModule*	getNxModule() = 0;

	/**
	\brief handle any work needed to prepare render resources before rendering

	This gets called from NxApexScene::prepareRenderResourceContexts().
	*/
	virtual void		prepareRenderResources() {}

	/**
	\brief Lock render resources according to module scene-defined behavior.

	Returns true iff successful.
	*/
	virtual	bool		lockRenderResources() { return false; }

	/**
	\brief Unlock render resources according to module scene-defined behavior.

	Returns true iff successful.
	*/
	virtual	bool		unlockRenderResources() { return false; }

	virtual NxApexSceneStats* 	getStats() = 0;

	/**
	\brief return ApexCudaObj from CudaModuleScene or NULL for non CUDA scenes
	Should be implemented only for scenes that inherited from CudaModuleScene
	*/
	virtual void*				getHeadCudaObj()
	{		
		return NULL;
	}
};

}
} // end namespace physx::apex

#endif // NI_MODULE_H

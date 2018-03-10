/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CLOTHING_H
#define MODULE_CLOTHING_H

#include "NxModuleClothing.h"
#include "NxClothingAsset.h"
#include "NiApexSDK.h"
#include "NiModule.h"
#include "Module.h"
#include "ApexSDKHelpers.h"
#include "ClothingParamClasses.h"
#include "ClothStructs.h"
#include "ApexRWLockable.h"
// The clothing GPU source is in a separate DLL, we're going to load it and get a CuFactory create method
#if defined(PX_WINDOWS)
#include "ModuleUpdateLoader.h"
#endif

class ClothingPhysicalMeshParameters;

namespace physx
{
#ifdef PX_WINDOWS
	namespace cloth
	{
		class CuFactory;
	}
#endif

namespace apex
{
class ApexIsoMesh;
class NiApexRenderMeshAssetAuthoring;
class NxClothingAsset;
class NxClothingAssetAuthoring;

namespace clothing
{
class ClothingScene;
class ClothingAsset;
class ClothingAssetAuthoring;
class ClothingPhysicalMesh;
class ClothingIsoMesh;
class CookingAbstract;
class DummyActor;
class DummyAsset;
class SimulationAbstract;


class BackendFactory
{
public:
	virtual bool				isMatch(const char* simulationBackend) = 0;
	virtual const char*			getName() = 0;
	virtual PxU32				getCookingVersion() = 0;
	virtual PxU32				getCookedDataVersion(const NxParameterized::Interface* cookedData) = 0;
	virtual CookingAbstract*	createCookingJob() = 0;
	virtual void				releaseCookedInstances(NxParameterized::Interface* cookedData, bool tetraMesh) = 0;
	virtual SimulationAbstract* createSimulation(bool tetraMesh, ClothingScene* clothingScene, bool useHW) = 0;
};



class ModuleClothing : public NxModuleClothing, public NiModule, public Module, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleClothing(NiApexSDK* sdk);

	// from NxApexInterface
	PX_INLINE void				release()
	{
		Module::release();
	}

	// from NxModule
	void						init(NxParameterized::Interface& desc);
	NxParameterized::Interface* getDefaultModuleDesc();
	PX_INLINE const char*		getName() const
	{
		return Module::getName();
	}
	NxAuthObjTypeID				getModuleID() const;
	PX_INLINE physx::PxU32		getNbParameters() const
	{
		return Module::getNbParameters();
	}
	PX_INLINE NxApexParameter**	getParameters()
	{
		return Module::getParameters();
	}
	PX_INLINE void				setIntValue(physx::PxU32 parameterIndex, physx::PxU32 value)
	{
		return Module::setIntValue(parameterIndex, value);
	}
	void						setLODUnitCost(physx::PxF32 cost)
	{
		Module::setLODUnitCost(cost);
	}
	physx::PxF32				getLODUnitCost() const
	{
		return Module::getLODUnitCost();
	}
	void						setLODBenefitValue(physx::PxF32 value)
	{
		Module::setLODBenefitValue(value);
	}
	physx::PxF32				getLODBenefitValue() const
	{
		return Module::getLODBenefitValue();
	}
	void						setLODEnabled(bool enabled)
	{
		Module::setLODEnabled(enabled);
	}
	bool						getLODEnabled() const
	{
		return Module::getLODEnabled();
	}
	NxApexRenderableIterator*	createRenderableIterator(const NxApexScene&);

	NxClothingPhysicalMesh*		createEmptyPhysicalMesh();
	NxClothingPhysicalMesh*		createSingleLayeredMesh(NxRenderMeshAssetAuthoring* asset, PxU32 subdivisionSize, bool mergeVertices, bool closeHoles, IProgressListener* progress);
	NxClothingIsoMesh*			createMultiLayeredMesh(NxRenderMeshAssetAuthoring* asset, PxU32 subdivisionSize, PxU32 keepNBiggestMeshes, bool discardInnerMeshes, IProgressListener* progress);

	PX_INLINE NxParameterized::Interface* getApexClothingActorParams(void) const
	{
		return mApexClothingActorParams;
	}
	PX_INLINE NxParameterized::Interface* getApexClothingPreviewParams(void) const
	{
		return mApexClothingPreviewParams;
	}

	// from NiModule
	virtual void				destroy();
	virtual NiModuleScene* 		createNiModuleScene(NiApexScene&, NiApexRenderDebug*);
	virtual void				releaseNiModuleScene(NiModuleScene&);
	virtual physx::PxU32		forceLoadAssets();

#ifndef WITHOUT_PVD
	virtual void				initPvdClasses(physx::debugger::comm::PvdDataStream& pvdDataStream);
	virtual void				initPvdInstances(physx::debugger::comm::PvdDataStream& pvdDataStream);
	virtual void				destroyPvdInstances();
#endif

	// own methods

	ClothingScene* 				getClothingScene(const NxApexScene& scene);

	ClothingPhysicalMesh*		createPhysicalMeshInternal(ClothingPhysicalMeshParameters* mesh);

	void						releasePhysicalMesh(ClothingPhysicalMesh* physicalMesh);
	void						releaseIsoMesh(ClothingIsoMesh* isoMesh);
	void						unregisterAssetWithScenes(ClothingAsset* asset);
	void						notifyReleaseGraphicalData(ClothingAsset* asset);

	NxApexActor*				getDummyActor();

	// This pointer will not be released by this module!
	virtual void				registerBackendFactory(BackendFactory* factory);
	virtual void				unregisterBackendFactory(BackendFactory* factory);
	// this works both with the name of the simulation backend, or the cooking data type
	BackendFactory*				getBackendFactory(const char* simulationBackend);

	ClothFactory				createClothFactory(physx::PxCudaContextManager* contextManager);
	void						releaseClothFactory(physx::PxCudaContextManager* contextManager);

	PX_INLINE PxU32				getMaxUnusedPhysXResources() const
	{
		return mInternalModuleParams.maxUnusedPhysXResources;
	}
	PX_INLINE PxU32				getMaxNumCompartments() const
	{
		return mInternalModuleParams.maxNumCompartments;
	}
	PX_INLINE bool				allowAsyncFetchResults() const
	{
		return mInternalModuleParams.asyncFetchResults;
	}
	PX_INLINE bool				allowAsyncCooking() const
	{
		return mInternalModuleParams.allowAsyncCooking;
	}
	PX_INLINE PxU32				getAvgSimFrequencyWindowSize() const
	{
		return mInternalModuleParams.avgSimFrequencyWindow;
	}
	PX_INLINE bool				allowApexWorkBetweenSubsteps() const
	{
		return mInternalModuleParams.allowApexWorkBetweenSubsteps;
	}
	PX_INLINE PxF32				getInterCollisionDistance() const
	{
		return mInternalModuleParams.interCollisionDistance;
	}
	PX_INLINE PxF32				getInterCollisionStiffness() const
	{
		return mInternalModuleParams.interCollisionStiffness;
	}
	PX_INLINE PxU32				getInterCollisionIterations() const
	{
		return mInternalModuleParams.interCollisionIterations;
	}
	PX_INLINE void				setInterCollisionDistance(PxF32 distance)
	{
		mInternalModuleParams.interCollisionDistance = distance;
	}
	PX_INLINE void				setInterCollisionStiffness(PxF32 stiffness)
	{
		mInternalModuleParams.interCollisionStiffness = stiffness;
	}
	PX_INLINE void				setInterCollisionIterations(PxU32 iterations)
	{
		mInternalModuleParams.interCollisionIterations = iterations;
	}
	PX_INLINE bool				useSparseSelfCollision() const
	{
		return mInternalModuleParams.sparseSelfCollision;
	}
	PX_INLINE PxU32				getMaxTimeRenderProxyInPool() const
	{
		return mInternalModuleParams.maxTimeRenderProxyInPool;
	}

private:
	NxClothingPhysicalMesh*		createSingleLayeredMeshInternal(NiApexRenderMeshAssetAuthoring* asset, PxU32 subdivisionSize, bool mergeVertices, bool closeHoles, IProgressListener* progress);
	NxClothingIsoMesh*			createMultiLayeredMeshInternal(NiApexRenderMeshAssetAuthoring* asset, PxU32 subdivisionSize, PxU32 keepNBiggestMeshes, bool discardInnerMeshes, IProgressListener* progress);

	NxResourceList				mClothingSceneList;

	NxResourceList				mPhysicalMeshes;
	NxResourceList				mIsoMeshes;

	NxResourceList				mAssetAuthorableObjectFactories;

	DummyActor*					mDummyActor;
	DummyAsset*					mDummyAsset;
	shdfnd::Mutex				mDummyProtector;


	class ClothingBackendFactory : public BackendFactory
	{
	public:
		virtual bool				isMatch(const char* simulationBackend);
		virtual const char*			getName();
		virtual PxU32				getCookingVersion();
		virtual PxU32				getCookedDataVersion(const NxParameterized::Interface* cookedData);
		virtual CookingAbstract*	createCookingJob();
		virtual void				releaseCookedInstances(NxParameterized::Interface* cookedData, bool tetraMesh);
		virtual SimulationAbstract* createSimulation(bool tetraMesh, ClothingScene* clothingScene, bool useHW);
	};

	class ClothingPhysX3Backend : public BackendFactory, public shdfnd::UserAllocated
	{
	public:
		virtual bool				isMatch(const char* simulationBackend);
		virtual const char*			getName();
		virtual PxU32				getCookingVersion();
		virtual PxU32				getCookedDataVersion(const NxParameterized::Interface* cookedData);
		virtual CookingAbstract*	createCookingJob();
		virtual void				releaseCookedInstances(NxParameterized::Interface* cookedData, bool /*tetraMesh*/);
		virtual SimulationAbstract* createSimulation(bool tetraMesh, ClothingScene* clothingScene, bool useHW);
	};

	ClothingBackendFactory		mBackendFactory;
	ClothingPhysX3Backend		mBackendFactoryPhysX3;

	Array<BackendFactory*>		mBackendFactories;

#	define PARAM_CLASS(clas) PARAM_CLASS_DECLARE_FACTORY(clas)
#	include "ClothingParamClasses.inc"

	ClothingModuleParameters*	mModuleParams;
	ClothingModuleParametersNS::ParametersStruct mInternalModuleParams;

	NxParameterized::Interface*	mApexClothingActorParams;
	NxParameterized::Interface*	mApexClothingPreviewParams;

	shdfnd::Mutex				mFactoryMutex;
	cloth::Factory*				mCpuFactory;
	PxU32						mCpuFactoryReferenceCount;

#ifdef PX_WINDOWS
	void*						mGpuDllHandle;

	// this function is declared in CreateCuFactory.h and implemented in CreateCuFactory.cpp, which is in in APEX_ClothingGPU
	typedef physx::cloth::CuFactory* (PxCreateCuFactory_FUNC)(physx::PxCudaContextManager* contextManager);
	PxCreateCuFactory_FUNC*		mPxCreateCuFactoryFunc;
	
	struct GpuFactoryEntry
	{
		GpuFactoryEntry(cloth::Factory* f, physx::PxCudaContextManager* c) : factoryGpu(f), contextManager(c), referenceCount(0) {}

		cloth::Factory* factoryGpu;
		physx::PxCudaContextManager* contextManager;
		PxU32 referenceCount;
	};

	shdfnd::Array<GpuFactoryEntry> mGpuFactories;
#endif

	
	virtual ~ModuleClothing() {}
};

}
} // namespace apex
} // namespace physx

#endif // MODULE_CLOTHING_H

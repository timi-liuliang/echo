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
#include "ApexAuthorableObject.h"
#include "ApexIsoMesh.h"
#include "ApexSubdivider.h"
#include "ApexSharedUtils.h"

#include "NiApexRenderMeshAsset.h"
#include <limits.h>
#include <new>

#if NX_SDK_VERSION_MAJOR == 2
#include "NxPhysicsSDK.h"
#endif

#include "PxCudaContextManager.h"
#include "Factory.h"

#include "ModuleClothing.h"
#include "ClothingAsset.h"
#include "ClothingAssetAuthoring.h"
#include "ClothingPhysicalMesh.h"
#include "ClothingIsoMesh.h"
#include "NiApexScene.h"

#include "ClothingScene.h"
#include "NxFromPx.h"
#include "ModulePerfScope.h"

#include "CookingPhysX.h"
#include "CookingPhysX3.h"
#include "SimulationPhysX3.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "SimulationNxCloth.h"
#include "SimulationNxSoftBody.h"
#endif

#endif

#include "NiApexSDK.h"
#include "PsShare.h"

#ifdef PX_WINDOWS
#include "CuFactory.h"
#endif

#include "PVDBinding.h"
#include "PvdDataStream.h"

using namespace clothing;
using namespace physx::debugger;

#define INIT_PVD_CLASSES_PARAMETERIZED( parameterizedClassName ) { \
	pvdStream.createClass(NamespacedName(APEX_PVD_NAMESPACE, #parameterizedClassName)); \
	parameterizedClassName* params = DYNAMIC_CAST(parameterizedClassName*)(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(#parameterizedClassName)); \
	pvdBinding->initPvdClasses(*params->rootParameterDefinition(), #parameterizedClassName); \
	params->destroy(); }


namespace physx
{
namespace apex
{

#if defined(_USRDLL)

/* Modules don't have to link against the framework, they keep their own */
NiApexSDK* gApexSdk = 0;
NxApexSDK* NxGetApexSDK()
{
	return gApexSdk;
}
NiApexSDK* NiGetApexSDK()
{
	return gApexSdk;
}

NXAPEX_API NxModule*  NX_CALL_CONV createModule(
    NiApexSDK* inSdk,
    NiModule** niRef,
    physx::PxU32 APEXsdkVersion,
    physx::PxU32 PhysXsdkVersion,
    NxApexCreateError* errorCode)
{
	using namespace clothing;
	if (APEXsdkVersion != NX_APEX_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	if (PhysXsdkVersion != NX_PHYSICS_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
	gApexSdk = inSdk;
	APEX_INIT_FOUNDATION();
	initModuleProfiling(inSdk, "Clothing");
	clothing::ModuleClothing* impl = PX_NEW(clothing::ModuleClothing)(inSdk);
	*niRef  = (NiModule*) impl;
	return (NxModule*) impl;
#else // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
	if (errorCode != NULL)
	{
		*errorCode = APEX_CE_WRONG_VERSION;
	}

	PX_UNUSED(niRef);
	PX_UNUSED(inSdk);
	return NULL; // Clothing Module can only compile against MIN_PHYSX_SDK_VERSION_REQUIRED or above
#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
}

#else // !defined(_USRDLL)

/* Statically linking entry function */
void instantiateModuleClothing()
{
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
	using namespace clothing;
	NiApexSDK* sdk = NiGetApexSDK();
	initModuleProfiling(sdk, "Clothing");
	clothing::ModuleClothing* impl = PX_NEW(clothing::ModuleClothing)(sdk);
	sdk->registerExternalModule((NxModule*) impl, (NiModule*) impl);
#endif
}
#endif  // !defined(_USRDLL)

namespace clothing
{
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

// This is needed to have an actor assigned to every NxActor, even if they currently don't belong to an NxClothingActor
class DummyActor : public NxApexActor, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DummyActor(NxApexAsset* owner) : mOwner(owner) {}
	void release()
	{
		PX_DELETE(this);
	}
	NxApexAsset* getOwner() const
	{
		return mOwner;
	}

	void getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const
	{
		PX_UNUSED(min);
		PX_UNUSED(max);
		PX_UNUSED(intOnly);
	}

	physx::PxF32 getActivePhysicalLod() const
	{
		return -1.0f;
	}

	void forcePhysicalLod(physx::PxF32 lod)
	{
		PX_UNUSED(lod);
	}

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		PX_UNUSED(state);
	}

private:
	NxApexAsset* mOwner;
};



// This is needed to for every dummy actor to point to an asset that in turn has the right object type id.
class DummyAsset : public NxApexAsset, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DummyAsset(NxAuthObjTypeID assetTypeID) : mAssetTypeID(assetTypeID) {};

	void release()
	{
		PX_DELETE(this);
	}

	virtual const char* getName() const
	{
		return NULL;
	}
	virtual NxAuthObjTypeID getObjTypeID() const
	{
		return mAssetTypeID;
	}
	virtual const char* getObjTypeName() const
	{
		return NULL;
	}
	virtual PxU32 forceLoadAssets()
	{
		return 0;
	}
	virtual const NxParameterized::Interface* getAssetNxParameterized() const
	{
		return NULL;
	}

	NxParameterized::Interface* getDefaultActorDesc()
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	};
	NxParameterized::Interface* getDefaultAssetPreviewDesc()
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	};

	virtual NxApexActor* createApexActor(const NxParameterized::Interface& /*parms*/, NxApexScene& /*apexScene*/)
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	}

	virtual NxApexAssetPreview* createApexAssetPreview(const ::NxParameterized::Interface& /*params*/, NxApexAssetPreviewScene* /*previewScene*/)
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	}

	virtual bool isValidForActorCreation(const ::NxParameterized::Interface& /*parms*/, NxApexScene& /*apexScene*/) const
	{
		return true; // TODO implement this method
	}

	virtual bool isDirty() const
	{
		return false;
	}


	/**
	 * \brief Releases the ApexAsset but returns the NxParameterized::Interface and *ownership* to the caller.
	 */
	virtual NxParameterized::Interface* releaseAndReturnNxParameterizedInterface(void)
	{
		return NULL;
	}

private:
	NxAuthObjTypeID mAssetTypeID;
};

ModuleClothing::ModuleClothing(NiApexSDK* inSdk)
	: mDummyActor(NULL)
	, mDummyAsset(NULL)
	, mModuleParams(NULL)
	, mApexClothingActorParams(NULL)
	, mApexClothingPreviewParams(NULL)
	, mCpuFactory(NULL)
	, mCpuFactoryReferenceCount(0)
#ifdef PX_WINDOWS
	, mGpuDllHandle(NULL)
	, mPxCreateCuFactoryFunc(NULL)
#endif
{
	mInternalModuleParams.maxNumCompartments = 4;
	mInternalModuleParams.maxUnusedPhysXResources = 5;
	mInternalModuleParams.allowAsyncCooking = true;
	mInternalModuleParams.avgSimFrequencyWindow = 60;
	mInternalModuleParams.allowApexWorkBetweenSubsteps = true;
	mInternalModuleParams.interCollisionDistance = 0.0f;
	mInternalModuleParams.interCollisionStiffness = 1.0f;
	mInternalModuleParams.interCollisionIterations = 1;
	mInternalModuleParams.sparseSelfCollision = false;
	mInternalModuleParams.maxTimeRenderProxyInPool = 100;
	PX_COMPILE_TIME_ASSERT(sizeof(mInternalModuleParams) == 40); // don't forget to init the new param here (and then update this assert)

	name = "Clothing";
	mSdk = inSdk;
	mApiProxy = this;

	NxParameterized::Traits* traits = mSdk->getParameterizedTraits();
	if (traits)
	{
#		define PARAM_CLASS(clas) PARAM_CLASS_REGISTER_FACTORY(traits, clas)
#		include "ClothingParamClasses.inc"

		mApexClothingActorParams = traits->createNxParameterized(ClothingActorParam::staticClassName());
		mApexClothingPreviewParams = traits->createNxParameterized(ClothingPreviewParam::staticClassName());
	}

	// Set per-platform unit cost.  One unit is one cloth vertex times one solver iteration
#if defined( PX_WINDOWS ) || defined( PX_XBOXONE )
	mLodUnitCost = 0.0001f;
#elif defined( PX_PS4 )
	mLodUnitCost = 0.0001f;
#elif defined( PX_X360 )
	mLodUnitCost = 0.001f;
#elif defined( PX_PS3 )
	mLodUnitCost = 0.001f;
#elif defined( PX_ANDROID )
	mLodUnitCost = 0.001f;
#elif defined( PX_LINUX )
	mLodUnitCost = 0.001f;
#else
	// Using default value set in Module class
#endif


#ifdef PX_WINDOWS
// 	// Since we split out the GPU code, we load the module and create the CuFactory ourselves
// 	ApexSimpleString gpuClothingDllName;
// 	PX_COMPILE_TIME_ASSERT(sizeof(HMODULE) == sizeof(mGpuDllHandle));
// 	{
// 		ModuleUpdateLoader moduleLoader(UPDATE_LOADER_DLL_NAME);
// 
// #define APEX_CLOTHING_GPU_DLL_PREFIX "APEX_ClothingGPU"
// 
// #ifdef PX_PHYSX_DLL_NAME_POSTFIX
// # if defined(PX_X86)
// 		static const char*	gpuClothingDllPrefix = APEX_CLOTHING_GPU_DLL_PREFIX PX_STRINGIZE(PX_PHYSX_DLL_NAME_POSTFIX) "_x86";
// # elif defined(PX_X64)
// 		static const char*	gpuClothingDllPrefix = APEX_CLOTHING_GPU_DLL_PREFIX PX_STRINGIZE(PX_PHYSX_DLL_NAME_POSTFIX) "_x64";
// # endif
// #else
// # if defined(PX_X86)
// 		static const char*	gpuClothingDllPrefix = APEX_CLOTHING_GPU_DLL_PREFIX "_x86";
// # elif defined(PX_X64)
// 		static const char*	gpuClothingDllPrefix = APEX_CLOTHING_GPU_DLL_PREFIX "_x64";
// # endif
// #endif
// 
// #undef APEX_CLOTHING_GPU_DLL_PREFIX
// 
// 		gpuClothingDllName = ApexSimpleString(gpuClothingDllPrefix);
// 
// 		// applications can append strings to the APEX DLL filenames, support this with getCustomDllNamePostfix()
// 		gpuClothingDllName += ApexSimpleString(NiGetApexSDK()->getCustomDllNamePostfix());
// 		gpuClothingDllName += ApexSimpleString(".dll");
// 
// 		mGpuDllHandle = moduleLoader.loadModule(gpuClothingDllName.c_str(), NiGetApexSDK()->getAppGuid());
// 	}
// 
// 	if (mGpuDllHandle)
// 	{
// 		mPxCreateCuFactoryFunc = (PxCreateCuFactory_FUNC*)GetProcAddress((HMODULE)mGpuDllHandle, "PxCreateCuFactory");
// 		if (mPxCreateCuFactoryFunc == NULL)
// 		{
// 			APEX_DEBUG_WARNING("Failed to find method PxCreateCuFactory in dll \'%s\'", gpuClothingDllName.c_str());
// 			FreeLibrary((HMODULE)mGpuDllHandle);
// 			mGpuDllHandle = NULL;
// 		}
// 	}
// 	else if (!gpuClothingDllName.empty())
// 	{
// 		APEX_DEBUG_WARNING("Failed to load the GPU dll \'%s\'", gpuClothingDllName.c_str());
// 	}
#endif
}



NxAuthObjTypeID ModuleClothing::getModuleID() const
{
	return ClothingAsset::mAssetTypeID;
}



NxApexRenderableIterator* ModuleClothing::createRenderableIterator(const NxApexScene& apexScene)
{
	ClothingScene* cs = getClothingScene(apexScene);
	if (cs)
	{
		return cs->createRenderableIterator();
	}

	return NULL;
}

#ifdef WITHOUT_APEX_AUTHORING

class ClothingAssetDummyAuthoring : public NxApexAssetAuthoring, public UserAllocated
{
public:
	ClothingAssetDummyAuthoring(ModuleClothing* module, NxResourceList& list, NxParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	ClothingAssetDummyAuthoring(ModuleClothing* module, NxResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	ClothingAssetDummyAuthoring(ModuleClothing* module, NxResourceList& list)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
	}

	virtual ~ClothingAssetDummyAuthoring() {}

	virtual void setToolString(const char* /*toolName*/, const char* /*toolVersion*/, PxU32 /*toolChangelist*/)
	{

	}


	virtual void release()
	{
		destroy();
	}

	// internal
	void destroy()
	{
		PX_DELETE(this);
	}

	/**
	* \brief Returns the name of this APEX authorable object type
	*/
	virtual const char* getObjTypeName() const
	{
		return NX_CLOTHING_AUTHORING_TYPE_NAME;
	}

	/**
	 * \brief Prepares a fully authored Asset Authoring object for a specified platform
	*/
	virtual bool prepareForPlatform(physx::apex::NxPlatformTag)
	{
		PX_ASSERT(0);
		return false;
	}

	const char* getName(void) const
	{
		return NULL;
	}

	/**
	* \brief Save asset's NxParameterized interface, may return NULL
	*/
	virtual NxParameterized::Interface* getNxParameterized() const
	{
		PX_ASSERT(0);
		return NULL; //ClothingAsset::getAssetNxParameterized();
	}

	virtual NxParameterized::Interface* releaseAndReturnNxParameterizedInterface(void)
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	}

};

typedef ApexAuthorableObject<ModuleClothing, ClothingAsset, ClothingAssetDummyAuthoring> ClothingAO;
#else
typedef ApexAuthorableObject<ModuleClothing, ClothingAsset, ClothingAssetAuthoring> ClothingAO;
#endif

NxParameterized::Interface* ModuleClothing::getDefaultModuleDesc()
{
	NxParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(ClothingModuleParameters*)
		                (traits->createNxParameterized("ClothingModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	const NxParameterized::Hint* hint = NULL;
	NxParameterized::Handle h(mModuleParams);

	h.getParameter("maxNumCompartments");
	PX_ASSERT(h.isValid());
#if defined(PX_WINDOWS)
	hint = h.parameterDefinition()->hint("defaultValueWindows");
#else
	hint = h.parameterDefinition()->hint("defaultValueConsoles");
#endif
	PX_ASSERT(hint);
	if (hint)
	{
		mModuleParams->maxNumCompartments = (physx::PxU32)hint->asUInt();
	}

	return mModuleParams;
}



void ModuleClothing::init(NxParameterized::Interface& desc)
{
	if (strcmp(desc.className(), ClothingModuleParameters::staticClassName()) == 0)
	{
		ClothingModuleParameters* params = DYNAMIC_CAST(ClothingModuleParameters*)(&desc);
		mInternalModuleParams = *params;
	}
	else
	{
		APEX_INVALID_PARAMETER("The NxParameterized::Interface object is of the wrong type");
	}

#if !defined(PX_WINDOWS) && NX_SDK_VERSION_MAJOR == 2
	if (mInternalModuleParams.maxNumCompartments > 0)
	{
		APEX_DEBUG_WARNING("ModuleClothingDesc::maxNumCompartments > 0!  On consoles performance is worse when using compartments for cloth and softbodies");
	}
#endif


	ClothingAO* AOClothingAsset = PX_NEW(ClothingAO)(this, mAssetAuthorableObjectFactories, ClothingAssetParameters::staticClassName());
	ClothingAsset::mAssetTypeID = AOClothingAsset->getResID();
	registerBackendFactory(&mBackendFactory);
	registerBackendFactory(&mBackendFactoryPhysX3);


#ifndef WITHOUT_PVD
	AOClothingAsset->mAssets.setupForPvd(mApiProxy, "NxClothingAssets", "NxClothingAsset");

	// handle case if module is created after pvd connection
	PVD::PvdBinding* pvdBinding = mSdk->getPvdBinding();
	if (pvdBinding != NULL)
	{
		if (pvdBinding->getConnectionType() & PvdConnectionType::eDEBUG)
		{
			pvdBinding->lock();
			physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
			if (pvdStream != NULL)
			{
				NamespacedName pvdModuleName(APEX_PVD_NAMESPACE, getName());
				pvdStream->createClass(pvdModuleName);
				initPvdClasses(*pvdStream);

				NxModule* nxModule = static_cast<NxModule*>(this);
				pvdStream->createInstance(pvdModuleName, nxModule);
				pvdStream->pushBackObjectRef(mSdk, "NxModules", nxModule);
				initPvdInstances(*pvdStream);
			}
			pvdBinding->unlock();
		}
	}
#endif
}



NxClothingPhysicalMesh* ModuleClothing::createEmptyPhysicalMesh()
{
	NX_WRITE_ZONE();
	return createPhysicalMeshInternal(NULL);
}



NxClothingPhysicalMesh* ModuleClothing::createSingleLayeredMesh(NxRenderMeshAssetAuthoring* asset, PxU32 subdivisionSize, bool mergeVertices, bool closeHoles, IProgressListener* progress)
{
	NX_WRITE_ZONE();
	return createSingleLayeredMeshInternal(DYNAMIC_CAST(NiApexRenderMeshAssetAuthoring*)(asset), subdivisionSize, mergeVertices, closeHoles, progress);
}



NxClothingIsoMesh* ModuleClothing::createMultiLayeredMesh(NxRenderMeshAssetAuthoring* asset, PxU32 subdivisionSize, physx::PxU32 keepNBiggestMeshes, bool discardInnerMeshes, IProgressListener* progress)
{
	NX_WRITE_ZONE();
	return createMultiLayeredMeshInternal(DYNAMIC_CAST(NiApexRenderMeshAssetAuthoring*)(asset), subdivisionSize, keepNBiggestMeshes, discardInnerMeshes, progress);
}



void ModuleClothing::destroy()
{
	mClothingSceneList.clear();

	if (mApexClothingActorParams != NULL)
	{
		mApexClothingActorParams->destroy();
		mApexClothingActorParams = NULL;
	}

	if (mApexClothingPreviewParams != NULL)
	{
		mApexClothingPreviewParams->destroy();
		mApexClothingPreviewParams = NULL;
	}

#ifdef PX_WINDOWS
	for (PxU32 i = 0; i < mGpuFactories.size(); i++)
	{
		//APEX_DEBUG_INFO("Release Gpu factory %d", i);
		PX_DELETE(mGpuFactories[i].factoryGpu);
		mGpuFactories.replaceWithLast(i);
	}
#endif
	PX_DELETE(mCpuFactory);
	mCpuFactory = NULL;

#ifndef WITHOUT_PVD
	destroyPvdInstances();
#endif
	if (mModuleParams != NULL)
	{
		mModuleParams->destroy();
		mModuleParams = NULL;
	}

	if (mDummyActor != NULL)
	{
		mDummyActor->release();
		mDummyActor = NULL;
	}

	if (mDummyAsset != NULL)
	{
		mDummyAsset->release();
		mDummyAsset = NULL;
	}

	NxParameterized::Traits* traits = mSdk->getParameterizedTraits();

	Module::destroy();

	releaseModuleProfiling();

	mAssetAuthorableObjectFactories.clear(); // needs to be done before destructor!

#ifdef PX_WINDOWS
	if (mGpuDllHandle != NULL)
	{
		FreeLibrary((HMODULE)mGpuDllHandle);
	}
#endif

	PX_DELETE(this);

	if (traits)
	{
#		define PARAM_CLASS(clas) PARAM_CLASS_REMOVE_FACTORY(traits, clas)
#		include "ClothingParamClasses.inc"
	}
}



NiModuleScene* ModuleClothing::createNiModuleScene(NiApexScene& scene, NiApexRenderDebug* renderDebug)
{
	return PX_NEW(ClothingScene)(*this, scene, renderDebug, mClothingSceneList);
}



void ModuleClothing::releaseNiModuleScene(NiModuleScene& scene)
{
	ClothingScene* clothingScene = DYNAMIC_CAST(ClothingScene*)(&scene);
	clothingScene->destroy();
}



physx::PxU32 ModuleClothing::forceLoadAssets()
{
	physx::PxU32 loadedAssetCount = 0;
	for (physx::PxU32 i = 0; i < mAssetAuthorableObjectFactories.getSize(); i++)
	{
		NiApexAuthorableObject* ao = static_cast<NiApexAuthorableObject*>(mAssetAuthorableObjectFactories.getResource(i));
		loadedAssetCount += ao->forceLoadAssets();
	}
	return loadedAssetCount;
}



#ifndef WITHOUT_PVD
void ModuleClothing::initPvdClasses(physx::debugger::comm::PvdDataStream& pvdStream)
{
	NamespacedName objRef = getPvdNamespacedNameForType<ObjectRef>();

	// ---------------------------------------
	// Hierarchy

	// NxModule holds NxClothingAssets
	pvdStream.createClass(NamespacedName(APEX_PVD_NAMESPACE, "NxClothingAsset"));
	pvdStream.createProperty(NamespacedName(APEX_PVD_NAMESPACE, getName()), "NxClothingAssets", "children", objRef, PropertyType::Array);
	
	// NxClothingAsset holds NxClothingActors
	pvdStream.createClass(NamespacedName(APEX_PVD_NAMESPACE, "NxClothingActor"));
	pvdStream.createProperty(NamespacedName(APEX_PVD_NAMESPACE, "NxClothingAsset"), "NxClothingActors", "children", objRef, PropertyType::Array);


	// ---------------------------------------
	// NxParameterized
	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	PX_ASSERT(pvdBinding != NULL);

	// Module Params
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingModuleParameters);
	pvdStream.createProperty(NamespacedName(APEX_PVD_NAMESPACE, getName()), "ModuleParams", "", objRef, PropertyType::Scalar);

	// Asset Params

	INIT_PVD_CLASSES_PARAMETERIZED(ClothingPhysicalMeshParameters);
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingGraphicalLodParameters);
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingCookedParam);
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingCookedPhysX3Param);
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingMaterialLibraryParameters);
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingAssetParameters);
	pvdStream.createProperty(NamespacedName(APEX_PVD_NAMESPACE, "NxClothingAsset"), "AssetParams", "", objRef, PropertyType::Scalar);

	// Actor Params
	INIT_PVD_CLASSES_PARAMETERIZED(ClothingActorParam);
	pvdStream.createProperty(NamespacedName(APEX_PVD_NAMESPACE, "NxClothingActor"), "ActorParams", "", objRef, PropertyType::Scalar);


	// ---------------------------------------
	// Additional Properties

	// ---------------------------------------
}



void ModuleClothing::initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream)
{
	// if there's more than one AOFactory we don't know any more for sure that its a clothing asset factory, so we have to adapt the code below
	PX_ASSERT(mAssetAuthorableObjectFactories.getSize() == 1 && "Adapt the code below");

	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	PX_ASSERT(pvdBinding != NULL);

	// Module Params
	pvdStream.createInstance(NamespacedName(APEX_PVD_NAMESPACE, "ClothingModuleParameters"), mModuleParams);
	pvdStream.setPropertyValue(mApiProxy, "ModuleParams", DataRef<const PxU8>((const PxU8*)&mModuleParams, sizeof(ClothingModuleParameters*)), getPvdNamespacedNameForType<ObjectRef>());
	// update module properties (should we do this per frame? if so, how?)
	pvdBinding->updatePvd(mModuleParams, *mModuleParams);

	// prepare asset list and forward init calls
	NiApexAuthorableObject* ao = static_cast<NiApexAuthorableObject*>(mAssetAuthorableObjectFactories.getResource(0));
	ao->mAssets.initPvdInstances(pvdStream);
}



void ModuleClothing::destroyPvdInstances()
{
	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	if (pvdBinding != NULL)
	{
		if (pvdBinding->getConnectionType() & PvdConnectionType::eDEBUG)
		{
			pvdBinding->lock();
			physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
			{
				if (pvdStream != NULL)
				{
					pvdBinding->updatePvd(mModuleParams, *mModuleParams, PVD::PvdAction::DESTROY);
					pvdStream->destroyInstance(mModuleParams);
				}
			}
			pvdBinding->unlock();
		}
	}
}
#endif


ClothingScene* ModuleClothing::getClothingScene(const NxApexScene& apexScene)
{
	const NiApexScene* niScene = DYNAMIC_CAST(const NiApexScene*)(&apexScene);
	for (physx::PxU32 i = 0 ; i < mClothingSceneList.getSize() ; i++)
	{
		ClothingScene* clothingScene = DYNAMIC_CAST(ClothingScene*)(mClothingSceneList.getResource(i));
		if (clothingScene->mApexScene == niScene)
		{
			return clothingScene;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate ClothingScene");
	return NULL;
}



ClothingPhysicalMesh* ModuleClothing::createPhysicalMeshInternal(ClothingPhysicalMeshParameters* mesh)
{
	ClothingPhysicalMesh* result = PX_NEW(ClothingPhysicalMesh)(this, mesh, &mPhysicalMeshes);
	return result;
}



void ModuleClothing::releasePhysicalMesh(ClothingPhysicalMesh* physicalMesh)
{
	physicalMesh->destroy();
}



void ModuleClothing::releaseIsoMesh(ClothingIsoMesh* isoMesh)
{
	isoMesh->destroy();
}


void ModuleClothing::unregisterAssetWithScenes(ClothingAsset* asset)
{
	for (physx::PxU32 i = 0; i < mClothingSceneList.getSize(); i++)
	{
		ClothingScene* clothingScene = static_cast<ClothingScene*>(mClothingSceneList.getResource(i));
		clothingScene->unregisterAsset(asset);
	}
}


void ModuleClothing::notifyReleaseGraphicalData(ClothingAsset* asset)
{
	for (physx::PxU32 i = 0; i < mClothingSceneList.getSize(); i++)
	{
		ClothingScene* clothingScene = static_cast<ClothingScene*>(mClothingSceneList.getResource(i));
		clothingScene->removeRenderProxies(asset);
	}
}


NxApexActor* ModuleClothing::getDummyActor()
{
	mDummyProtector.lock();
	if (mDummyActor == NULL)
	{
		PX_ASSERT(mDummyAsset == NULL);
		mDummyAsset = PX_NEW(DummyAsset)(getModuleID());
		mDummyActor = PX_NEW(DummyActor)(mDummyAsset);
	}
	mDummyProtector.unlock();

	return mDummyActor;
}



void ModuleClothing::registerBackendFactory(BackendFactory* factory)
{
	for (PxU32 i = 0; i < mBackendFactories.size(); i++)
	{
		if (strcmp(mBackendFactories[i]->getName(), factory->getName()) == 0)
		{
			return;
		}
	}

	mBackendFactories.pushBack(factory);
}



void ModuleClothing::unregisterBackendFactory(BackendFactory* factory)
{
	PxU32 read = 0, write = 0;

	while (read < mBackendFactories.size())
	{
		mBackendFactories[write] = mBackendFactories[read];

		if (mBackendFactories[read] == factory)
		{
			read++;
		}
		else
		{
			read++, write++;
		}
	}

	while (read < write)
	{
		mBackendFactories.popBack();
	}
}



BackendFactory* ModuleClothing::getBackendFactory(const char* simulationBackend)
{
	PX_ASSERT(simulationBackend != NULL);

	for (PxU32 i = 0; i < mBackendFactories.size(); i++)
	{
		if (mBackendFactories[i]->isMatch(simulationBackend))
		{
			return mBackendFactories[i];
		}
	}

	//APEX_INVALID_OPERATION("Simulation back end \'%s\' not found, using \'PhysX\' instead\n", simulationBackend);

	PX_ASSERT(mBackendFactories.size() >= 1);
	PX_ASSERT(strcmp(mBackendFactories[0]->getName(), "Native") == 0);
	return mBackendFactories[0];
}



ClothFactory ModuleClothing::createClothFactory(physx::PxCudaContextManager* contextManager)
{
	shdfnd::Mutex::ScopedLock lock(mFactoryMutex);

#ifdef PX_WINDOWS

#if NX_SDK_VERSION_MAJOR == 2
	if (contextManager != NULL)
#elif NX_SDK_VERSION_MAJOR == 3
	if (contextManager != NULL && contextManager->supportsArchSM20())
#endif // NX_SDK_VERSION_MAJOR
	{
		for (PxU32 i = 0; i < mGpuFactories.size(); i++)
		{
			if (mGpuFactories[i].contextManager == contextManager)
			{
				mGpuFactories[i].referenceCount++;
				//APEX_DEBUG_INFO("Found Gpu factory %d (ref = %d)", i, mGpuFactories[i].referenceCount);
				return ClothFactory(mGpuFactories[i].factoryGpu, &mFactoryMutex);
			}
		}

		// nothing found
		if (mPxCreateCuFactoryFunc != NULL)
		{
			GpuFactoryEntry entry(mPxCreateCuFactoryFunc(contextManager), contextManager);
			if (entry.factoryGpu != NULL)
			{
				//APEX_DEBUG_INFO("Create Gpu factory %d", mGpuFactories.size());
				entry.referenceCount = 1;
				mGpuFactories.pushBack(entry);
				return ClothFactory(entry.factoryGpu, &mFactoryMutex);
			}
		}

		return ClothFactory(NULL, &mFactoryMutex);
	}
	else
#else
	PX_UNUSED(contextManager);
#endif
	{
		if (mCpuFactory == NULL)
		{
			mCpuFactory = cloth::Factory::createFactory(cloth::Factory::CPU);
			//APEX_DEBUG_INFO("Create Cpu factory");
			PX_ASSERT(mCpuFactoryReferenceCount == 0);
		}

		mCpuFactoryReferenceCount++;
		//APEX_DEBUG_INFO("Get Cpu factory (ref = %d)", mCpuFactoryReferenceCount);

		return ClothFactory(mCpuFactory, &mFactoryMutex);
	}
}



void ModuleClothing::releaseClothFactory(physx::PxCudaContextManager* contextManager)
{
	shdfnd::Mutex::ScopedLock lock(mFactoryMutex);

#ifdef PX_WINDOWS
	if (contextManager != NULL)
	{
		for (PxU32 i = 0; i < mGpuFactories.size(); i++)
		{
			if (mGpuFactories[i].contextManager == contextManager)
			{
				PX_ASSERT(mGpuFactories[i].referenceCount > 0);
				mGpuFactories[i].referenceCount--;
				//APEX_DEBUG_INFO("Found Gpu factory %d (ref = %d)", i, mGpuFactories[i].referenceCount);

				if (mGpuFactories[i].referenceCount == 0)
				{
					//APEX_DEBUG_INFO("Release Gpu factory %d", i);
					PX_DELETE(mGpuFactories[i].factoryGpu);
					mGpuFactories.replaceWithLast(i);
				}
			}
		}
	}
	else
#else
	PX_UNUSED(contextManager);
#endif
	{
		PX_ASSERT(mCpuFactoryReferenceCount > 0);

		mCpuFactoryReferenceCount--;
		//APEX_DEBUG_INFO("Release Cpu factory (ref = %d)", mCpuFactoryReferenceCount);

		if (mCpuFactoryReferenceCount == 0)
		{
			PX_DELETE(mCpuFactory);
			mCpuFactory = NULL;
		}
	}
}



NxClothingPhysicalMesh* ModuleClothing::createSingleLayeredMeshInternal(NiApexRenderMeshAssetAuthoring* renderMeshAsset, PxU32 subdivisionSize,
        bool mergeVertices, bool closeHoles, IProgressListener* progressListener)
{
	if (renderMeshAsset->getPartCount() > 1)
	{
		APEX_INVALID_PARAMETER("NxRenderMeshAssetAuthoring has more than one part (%d)", renderMeshAsset->getPartCount());
		return NULL;
	}

	if (subdivisionSize > 200)
	{
		APEX_INVALID_PARAMETER("subdivisionSize must be smaller or equal to 200 and has been clamped (was %d).", subdivisionSize);
		subdivisionSize = 200;
	}

	HierarchicalProgressListener progress(100, progressListener);


	PxU32 numGraphicalVertices = 0;

	for (PxU32 i = 0; i < renderMeshAsset->getSubmeshCount(); i++)
	{
		const NiApexRenderSubmesh& submesh = renderMeshAsset->getNiSubmesh(i);
		numGraphicalVertices += submesh.getVertexBuffer().getVertexCount();
	}

	ClothingPhysicalMesh* physicalMesh = DYNAMIC_CAST(ClothingPhysicalMesh*)(createEmptyPhysicalMesh());

	// set time for registration, merge, close and subdivision
	PxU32 times[4] = { 20, (PxU32)(mergeVertices ? 20 : 0), (PxU32)(closeHoles ? 30 : 0), (PxU32)(subdivisionSize > 0 ? 30 : 0) };
	PxU32 sum = times[0] + times[1] + times[2] + times[3];
	for (PxU32 i = 0; i < 4; i++)
	{
		times[i] = 100 * times[i] / sum;
	}

	progress.setSubtaskWork((physx::PxI32)times[0], "Creating single layered mesh");
	ApexSubdivider subdivider;

	Array<PxI32> old2New(numGraphicalVertices, -1);
	PxU32 nbVertices = 0;

	PxU32 vertexOffset = 0;

	for (PxU32 submeshNr = 0; submeshNr < renderMeshAsset->getSubmeshCount(); submeshNr++)
	{
		const NiApexRenderSubmesh& submesh = renderMeshAsset->getNiSubmesh(submeshNr);

		// used for physics?
		const NxVertexFormat& vf = submesh.getVertexBuffer().getFormat();
		PxU32 customIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getID("USED_FOR_PHYSICS"));
		NxRenderDataFormat::Enum outFormat = vf.getBufferFormat(customIndex);
		const PxU8* usedForPhysics = NULL;
		if (outFormat == NxRenderDataFormat::UBYTE1)
		{
			usedForPhysics = (const PxU8*)submesh.getVertexBuffer().getBuffer(customIndex);
		}

		customIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getID("LATCH_TO_NEAREST_SLAVE"));
		outFormat = vf.getBufferFormat(customIndex);
		const PxU32* latchToNearestSlave = outFormat != NxRenderDataFormat::UINT1 ? NULL : (PxU32*)submesh.getVertexBuffer().getBuffer(customIndex);

		customIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getID("LATCH_TO_NEAREST_MASTER"));
		outFormat = vf.getBufferFormat(customIndex);
		const PxU32* latchToNearestMaster = outFormat != NxRenderDataFormat::UINT1 ? NULL : (PxU32*)submesh.getVertexBuffer().getBuffer(customIndex);
		PX_ASSERT((latchToNearestSlave != NULL) == (latchToNearestMaster != NULL)); // both NULL or not NULL

		// triangles
		const PxU32* indices = submesh.getIndexBuffer(0); // only 1 part supported!

		// vertices
		NxRenderDataFormat::Enum format;
		PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::POSITION));
		const PxVec3* positions = (const PxVec3*)submesh.getVertexBuffer().getBufferAndFormat(format, bufferIndex);
		if (format != NxRenderDataFormat::FLOAT3)
		{
			PX_ALWAYS_ASSERT();
			positions = NULL;
		}

		const PxU32 submeshIndices = submesh.getIndexCount(0);
		for (PxU32 meshIndex = 0; meshIndex < submeshIndices; meshIndex += 3)
		{
			if (latchToNearestSlave != NULL)
			{
				PxU32 numVerticesOk = 0;
				for (PxU32 i = 0; i < 3; i++)
				{
					const PxU32 index = indices[meshIndex + i];
					numVerticesOk += latchToNearestSlave[index] == 0 ? 1u : 0u;
				}
				if (numVerticesOk < 3)
				{
					continue;    // skip this triangle
				}
			}
			else if (usedForPhysics != NULL)
			{
				PxU32 numVerticesOk = 0;
				for (PxU32 i = 0; i < 3; i++)
				{
					const PxU32 index = indices[meshIndex + i];
					numVerticesOk += usedForPhysics[index] == 0 ? 0u : 1u;
				}
				if (numVerticesOk < 3)
				{
					continue;    // skip this triangle
				}
			}

			// add triangle to subdivider
			for (PxU32 i = 0; i < 3; i++)
			{
				const PxU32 localIndex = indices[meshIndex + i];
				const PxU32 index = localIndex + vertexOffset;
				if (old2New[index] == -1)
				{
					old2New[index] = (physx::PxI32)nbVertices++;
					PxU32 master = latchToNearestMaster != NULL ? latchToNearestMaster[localIndex] : 0xffffffffu;
					subdivider.registerVertex(positions[localIndex], master);
				}
			}

			const PxU32 i0 = (physx::PxU32)old2New[indices[meshIndex + 0] + vertexOffset];
			const PxU32 i1 = (physx::PxU32)old2New[indices[meshIndex + 1] + vertexOffset];
			const PxU32 i2 = (physx::PxU32)old2New[indices[meshIndex + 2] + vertexOffset];
			subdivider.registerTriangle(i0, i1, i2);
		}
		vertexOffset += submesh.getVertexBuffer().getVertexCount();
	}

	subdivider.endRegistration();
	progress.completeSubtask();

	if (nbVertices == 0)
	{
		APEX_INVALID_PARAMETER("Mesh has no active vertices (see Physics on/off channel)");
		return NULL;
	}

	// use subdivider
	if (mergeVertices)
	{
		progress.setSubtaskWork((physx::PxI32)times[1], "Merging");
		subdivider.mergeVertices(&progress);
		progress.completeSubtask();
	}

	if (closeHoles)
	{
		progress.setSubtaskWork((physx::PxI32)times[2], "Closing holes");
		subdivider.closeMesh(&progress);
		progress.completeSubtask();
	}

	if (subdivisionSize > 0)
	{
		progress.setSubtaskWork((physx::PxI32)times[3], "Subdividing");
		subdivider.subdivide(subdivisionSize, &progress);
		progress.completeSubtask();
	}

	Array<PxVec3> newVertices(subdivider.getNumVertices());
	Array<PxU32> newMasterValues(subdivider.getNumVertices());
	for (PxU32 i = 0; i < newVertices.size(); i++)
	{
		subdivider.getVertex(i, newVertices[i], newMasterValues[i]);
	}

	Array<PxU32> newIndices(subdivider.getNumTriangles() * 3);
	for (PxU32 i = 0; i < newIndices.size(); i += 3)
	{
		subdivider.getTriangle(i / 3, newIndices[i], newIndices[i + 1], newIndices[i + 2]);
	}

	physicalMesh->setGeometry(false, newVertices.size(), sizeof(PxVec3), newVertices.begin(), newMasterValues.begin(), newIndices.size(), sizeof(PxU32), &newIndices[0]);

	return physicalMesh;
}



NxClothingIsoMesh* ModuleClothing::createMultiLayeredMeshInternal(NiApexRenderMeshAssetAuthoring* renderMeshAsset, PxU32 subdivisionSize,
        PxU32 keepNBiggestMeshes, bool discardInnerMeshes, IProgressListener* progressListener)
{
	if (renderMeshAsset->getPartCount() != 1)
	{
		APEX_INTERNAL_ERROR("Input mesh must have exactly one part (has %d)", renderMeshAsset->getPartCount());
		return NULL;
	}

	if (subdivisionSize > 200)
	{
		APEX_INVALID_PARAMETER("subdivisionSize must be smaller or equal to 200 and has been clamped (was %d).", subdivisionSize);
		subdivisionSize = 200;
	}

	ApexIsoMesh isoMesh(subdivisionSize, keepNBiggestMeshes, discardInnerMeshes);
	isoMesh.setBound(renderMeshAsset->getBounds(0));

	HierarchicalProgressListener progress(100, progressListener);
	progress.setSubtaskWork(90, "Initializing Iso Mesh");

	for (PxU32 submeshNr = 0; submeshNr < renderMeshAsset->getSubmeshCount(); submeshNr++)
	{
		const NiApexRenderSubmesh& submesh = renderMeshAsset->getNiSubmesh(submeshNr);

		NxRenderDataFormat::Enum positionFormat;
		const NxVertexBuffer& vb = submesh.getVertexBuffer();
		const NxVertexFormat& vf = vb.getFormat();
		PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::POSITION));
		const PxVec3* positions = (const PxVec3*)vb.getBufferAndFormat(positionFormat, bufferIndex);
		PX_ASSERT(positionFormat == physx::NxRenderDataFormat::FLOAT3);

		// triangles
		const PxU32* indices = submesh.getIndexBuffer(0);

		// used for physics
		PxU32 customIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getID("USED_FOR_PHYSICS"));
		NxRenderDataFormat::Enum outFormat = vf.getBufferFormat(customIndex);
		const PxU8* usedForPhysics = NULL;
		if (outFormat == NxRenderDataFormat::UBYTE1)
		{
			usedForPhysics = (const PxU8*)vb.getBuffer(customIndex);
		}

		customIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getID("LATCH_TO_NEAREST_SLAVE"));
		outFormat = vf.getBufferFormat(customIndex);
		const PxU32* latchToNearestSlave = outFormat != NxRenderDataFormat::UINT1 ? NULL : (PxU32*)submesh.getVertexBuffer().getBuffer(customIndex);

		customIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getID("LATCH_TO_NEAREST_MASTER"));
		outFormat = vf.getBufferFormat(customIndex);
		const PxU32* latchToNearestMaster = outFormat != NxRenderDataFormat::UINT1 ? NULL : (PxU32*)submesh.getVertexBuffer().getBuffer(customIndex);
		PX_ASSERT((latchToNearestSlave != NULL) == (latchToNearestMaster != NULL)); // both NULL or not NULL
		PX_UNUSED(latchToNearestMaster);

		const PxU32 submeshIndices = submesh.getIndexCount(0);
		for (PxU32 meshIndex = 0; meshIndex < submeshIndices; meshIndex += 3)
		{
			if (latchToNearestSlave != NULL)
			{
				PxU32 numVerticesOk = 0;
				for (PxU32 i = 0; i < 3; i++)
				{
					const PxU32 index = indices[meshIndex + i];
					numVerticesOk += latchToNearestSlave[index] == 0 ? 1u : 0u;
				}
				if (numVerticesOk < 3)
				{
					continue;    // skip this triangle
				}
			}
			else if (usedForPhysics != NULL)
			{
				PxU32 numVerticesOk = 0;
				for (PxU32 k = 0; k < 3; k++)
				{
					const PxU32 index = indices[meshIndex + k];
					numVerticesOk += usedForPhysics[index] == 0 ? 0u : 1u;
				}
				if (numVerticesOk < 3)
				{
					continue;    // skip this triangle
				}
			}

			isoMesh.addTriangle(positions[indices[meshIndex + 0]], positions[indices[meshIndex + 1]], positions[indices[meshIndex + 2]]);
		}
	}

	isoMesh.update(&progress);
	progress.completeSubtask();

	progress.setSubtaskWork(10, "Copying Iso Mesh");
	ClothingIsoMesh* clothingIsoMesh = PX_NEW(ClothingIsoMesh)(this, isoMesh, subdivisionSize, &mIsoMeshes);
	progress.completeSubtask();

	return clothingIsoMesh;
}


bool ModuleClothing::ClothingBackendFactory::isMatch(const char* simulationBackend)
{
	if (strcmp(getName(), simulationBackend) == 0)
	{
		return true;
	}

	if (strcmp(ClothingCookedParam::staticClassName(), simulationBackend) == 0)
	{
		return true;
	}

	return false;
}

const char* ModuleClothing::ClothingBackendFactory::getName()
{
	return "Native";
}

/*const char* ModuleClothing::ClothingBackendFactory::getCookingJobType()
{
	return ClothingCookedParam::staticClassName();
}*/

PxU32 ModuleClothing::ClothingBackendFactory::getCookingVersion()
{
	return CookingPhysX::getCookingVersion();
}

PxU32 ModuleClothing::ClothingBackendFactory::getCookedDataVersion(const NxParameterized::Interface* cookedData)
{
	if (cookedData != NULL && isMatch(cookedData->className()))
	{
		return ((ClothingCookedParam*)cookedData)->cookedDataVersion;
	}

	return 0;
}

CookingAbstract* ModuleClothing::ClothingBackendFactory::createCookingJob()
{
	return PX_NEW(CookingPhysX)();
}

void ModuleClothing::ClothingBackendFactory::releaseCookedInstances(NxParameterized::Interface* _cookedData, bool tetraMesh)
{
	if (_cookedData != NULL)
	{
		PX_ASSERT(strcmp(_cookedData->className(), ClothingCookedParam::staticClassName()) == 0);
		ClothingCookedParam* cookedData = static_cast<ClothingCookedParam*>(_cookedData);

#if NX_SDK_VERSION_MAJOR == 2
		NxPhysicsSDK* pSDK = NiGetApexSDK()->getPhysXSDK();
#endif
		for (PxI32 i = 0; i < cookedData->convexMeshPointers.arraySizes[0]; i++)
		{
#if NX_SDK_VERSION_MAJOR == 2
			NxConvexMesh* convexMesh = reinterpret_cast<NxConvexMesh*>(cookedData->convexMeshPointers.buf[i]);
			if (convexMesh != NULL)
			{
				pSDK->releaseConvexMesh(*convexMesh);
			}
#elif NX_SDK_VERSION_MAJOR == 3
			PX_ALWAYS_ASSERT();
#endif
			cookedData->convexMeshPointers.buf[i] = NULL;
		}

		for (PxI32 i = 0; i < cookedData->cookedPhysicalSubmeshes.arraySizes[0]; i++)
		{
			if (tetraMesh)
			{
#if NX_SDK_VERSION_MAJOR == 2
				NxSoftBodyMesh* mesh = reinterpret_cast<NxSoftBodyMesh*>(cookedData->cookedPhysicalSubmeshes.buf[i].deformableMeshPointer);
				if (mesh != NULL)
				{
					pSDK->releaseSoftBodyMesh(*mesh);
				}
#elif NX_SDK_VERSION_MAJOR == 3
				PX_ALWAYS_ASSERT();
#endif
			}
			else
			{
#if NX_SDK_VERSION_MAJOR == 2
				NxClothMesh* mesh = reinterpret_cast<NxClothMesh*>(cookedData->cookedPhysicalSubmeshes.buf[i].deformableMeshPointer);
				if (mesh != NULL)
				{
					pSDK->releaseClothMesh(*mesh);
				}
#elif NX_SDK_VERSION_MAJOR == 3
				PX_ALWAYS_ASSERT();
#endif
			}
			cookedData->cookedPhysicalSubmeshes.buf[i].deformableMeshPointer = NULL;

			PX_ASSERT(cookedData->cookedPhysicalSubmeshes.buf[i].deformableMeshPointer == NULL);
		}
	}
}



SimulationAbstract* ModuleClothing::ClothingBackendFactory::createSimulation(bool tetraMesh, ClothingScene* clothingScene, bool useHW)
{
	if (tetraMesh)
	{
#if NX_SDK_VERSION_MAJOR == 2
		return PX_NEW(SimulationNxSoftBody)(clothingScene, useHW);
#elif NX_SDK_VERSION_MAJOR == 3
		PX_ALWAYS_ASSERT();
		PX_UNUSED(clothingScene);
		PX_UNUSED(useHW);
		return NULL;
#endif
	}
	else
	{
#if NX_SDK_VERSION_MAJOR == 2
		return PX_NEW(SimulationNxCloth)(clothingScene, useHW);
#elif NX_SDK_VERSION_MAJOR == 3
		// PH: We only get here when the user specifies 'ForceNative' in the actor desc's 'simulationBackend'.
		//     Since there is no native solver (we ditched 3.2 PxCloth support) we should return NULL
		return NULL;
#endif
	}
}


bool ModuleClothing::ClothingPhysX3Backend::isMatch(const char* simulationBackend)
{
	if (strcmp(getName(), simulationBackend) == 0)
	{
		return true;
	}

	if (strcmp(ClothingCookedPhysX3Param::staticClassName(), simulationBackend) == 0)
	{
		return true;
	}

	return false;
}

const char*	ModuleClothing::ClothingPhysX3Backend::getName()
{
	return "Embedded";
}

PxU32 ModuleClothing::ClothingPhysX3Backend::getCookingVersion()
{
	return CookingPhysX3::getCookingVersion();
}

PxU32 ModuleClothing::ClothingPhysX3Backend::getCookedDataVersion(const NxParameterized::Interface* cookedData)
{
	if (cookedData != NULL && isMatch(cookedData->className()))
	{
		return static_cast<const ClothingCookedPhysX3Param*>(cookedData)->cookedDataVersion;
	}

	return 0;
}

CookingAbstract* ModuleClothing::ClothingPhysX3Backend::createCookingJob()
{
	bool withFibers = true;
	return PX_NEW(CookingPhysX3)(withFibers);
}

void ModuleClothing::ClothingPhysX3Backend::releaseCookedInstances(NxParameterized::Interface* cookedData, bool /*tetraMesh*/)
{
	SimulationPhysX3::releaseFabric(cookedData);
}

SimulationAbstract* ModuleClothing::ClothingPhysX3Backend::createSimulation(bool tetraMesh, ClothingScene* clothingScene, bool useHW)
{
	if (!tetraMesh)
	{
		return PX_NEW(SimulationPhysX3)(clothingScene, useHW);
	}
	APEX_INTERNAL_ERROR("PhysX3 does not (yet?) support tetrahedral meshes\n");
	return NULL;
}


#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

}
} // namespace apex
} // namespace physx

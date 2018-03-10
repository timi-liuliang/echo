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
#include "ApexScene.h"
#include "ApexSDK.h"
#include "FrameworkPerfScope.h"
#include "ApexRenderMeshAsset.h"
#include "ApexRenderMeshActor.h"
#include "ApexRenderMeshAssetAuthoring.h"
#include "ApexResourceProvider.h"
#include "PxMemoryBuffer.h"
#include "ApexString.h"
#include "ApexDefaultStream.h"
#include "ApexRenderDebug.h"
#include "NxParameterized.h"
#include "PvdNxParamSerializer.h"
#include "PxMemoryBuffer.h"
#include "PsFoundation.h"
#include "PvdConnection.h"
#include "PVDCommLayerNxDebuggerStream.h"
#include "PsIPC.h"
#include "PsFastIPC.h"
#include "PxFileBuffer.h"
#include "NxSerializerInternal.h"
#include "PxFileRenderDebug.h"
#include "NxUserOpaqueMesh.h"
#include "NxParamUtils.h"
#include "ApexShape.h"
#include "ApexStubPxProfileZone.h"
#include "ApexAssetPreviewScene.h"
#include "RenderResourceManagerWrapper.h"
#include "PsThread.h"


#define MAX_MSG_SIZE 65536
#define WITH_DEBUG_ASSET 0

#if defined(PX_WINDOWS)
#include <PsWindowsInclude.h>
#include <cstdio>
#include "ModuleUpdateLoader.h"

// We require at least Visual Studio 2010 w/ SP1 to compile
#if defined(_MSC_VER)
#	if _MSC_VER >= 1900
PX_COMPILE_TIME_ASSERT(_MSC_FULL_VER >= 190000000);
#	elif _MSC_VER >= 1800
PX_COMPILE_TIME_ASSERT(_MSC_FULL_VER >= 180000000);
#	elif _MSC_VER >= 1700
PX_COMPILE_TIME_ASSERT(_MSC_FULL_VER >= 170000000);
#	elif _MSC_VER >= 1600
PX_COMPILE_TIME_ASSERT(_MSC_FULL_VER >= 160040219);
#	endif

#	if _MSC_VER > 1900
	#pragma message("Detected compiler newer than Visual Studio 2013, please update min version checking in ApexSDK.cpp")
	PX_COMPILE_TIME_ASSERT(_MSC_VER <= 1900);
	#	endif
#endif

#endif //defined(PX_WINDOWS)

#if defined(PX_X86)
#define PTR_TO_UINT64(x) ((physx::PxU64)(physx::PxU32)(x))
#else
#define PTR_TO_UINT64(x) ((physx::PxU64)(x))
#endif

#if defined(APEX_CUDA_SUPPORT)
#include "windows/PhysXIndicator.h"
#endif

#if defined(PX_X360)
#include <stdarg.h>
#include <cstdio>
#endif

#include "foundation/PxErrorCallback.h"
#include "PxCudaContextManager.h"
#include "PxCpuDispatcher.h"

#ifdef PHYSX_PROFILE_SDK
#include "PVDBinding.h"
#if NX_SDK_VERSION_MAJOR == 2
#include "PVD28Binding.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxPhysics.h"
#endif

physx::PxProfileZone *gProfileZone=NULL;

#endif

namespace physx
{
namespace apex
{


extern ApexSDK* gApexSdk;

#if defined(_USRDLL)
typedef NxModule* (NxCreateModule_FUNC)(NiApexSDK*, NiModule**, physx::PxU32, physx::PxU32, NxApexCreateError*);
#else
/* When modules are statically linked, the user must instantiate modules manually before they can be
 * created via the NxApexSDK::createModule() method.  Each module must supply an instantiation function.
 */
#endif

ApexSDK::ApexSDK(NxApexCreateError* errorCode, physx::PxU32 /*inAPEXsdkVersion*/)
	: mAuthorableObjects(NULL)
	, mBatchSeedSize(128)
	, mErrorString(NULL)
	, mNumTempMemoriesActive(0)
#if NX_SDK_VERSION_MAJOR == 2
	, mApexThreadPool(0)
#endif
	, mOwnsFoundation(false)
	, renderResourceManager(NULL)
	, renderResourceManagerWrapper(NULL)
	, apexResourceProvider(NULL)
	, cookingVersion(0)
	, mURRdepthTLSslot(0xFFFFFFFF)
	, mEnableApexStats(true)
	, mEnableConcurrencyCheck(false)
{
	if (errorCode)
	{
		*errorCode = APEX_CE_NO_ERROR;
	}

#if defined(PX_DEBUG) || defined(PX_CHECKED)
	mURRdepthTLSslot = TlsAlloc();
#endif
}

NxAuthObjTypeID	ApexRenderMeshAsset::mObjTypeID;

void ModuleFramework::init(NxParameterized::Traits* t)
{
#	define PARAM_CLASS(clas) PARAM_CLASS_REGISTER_FACTORY(t, clas)
#	include "FrameworkParamClasses.inc"

#	define PARAM_CLASS(clas) PARAM_CLASS_REGISTER_FACTORY(t, clas)
#	include "CommonParamClasses.inc"
}

void ModuleFramework::release(NxParameterized::Traits* t)
{
#	define PARAM_CLASS(clas) PARAM_CLASS_REMOVE_FACTORY(t, clas)
#	include "FrameworkParamClasses.inc"

#	define PARAM_CLASS(clas) PARAM_CLASS_REMOVE_FACTORY(t, clas)
#	include "CommonParamClasses.inc"
}

// Many things can't be initialized in the constructor since they depend on gApexSdk
// being present.
void ApexSDK::init(const NxApexSDKDesc& desc, bool ownsFoundation)
{

	renderResourceManager = desc.renderResourceManager;
#if defined PX_DEBUG || defined PX_CHECKED
	if (renderResourceManagerWrapper != NULL)
	{
		PX_DELETE(renderResourceManagerWrapper);
		renderResourceManagerWrapper = NULL;
	}
	if (renderResourceManager != NULL)
	{
		renderResourceManagerWrapper = PX_NEW(RenderResourceManagerWrapper)(*renderResourceManager);
	}
#endif

	physXSDK = desc.physXSDK;
	cooking = desc.cooking;
	physXsdkVersion = desc.physXSDKVersion;
	mDllLoadPath = desc.dllLoadPath;
	mCustomDllNamePostfix = desc.dllNamePostfix;
	mWireframeMaterial = desc.wireframeMaterial;
	mSolidShadedMaterial = desc.solidShadedMaterial;
#ifdef PX_WINDOWS
	mAppGuid = desc.appGuid ? desc.appGuid : DEFAULT_APP_GUID;
#endif
	mRMALoadMaterialsLazily = desc.renderMeshActorLoadMaterialsLazily;

	mOwnsFoundation = ownsFoundation;
	mEnableConcurrencyCheck = desc.enableConcurrencyCheck;

	Framework::initFrameworkProfiling(this);

	apexResourceProvider = PX_NEW(ApexResourceProvider)();
	PX_ASSERT(apexResourceProvider);
	apexResourceProvider->setCaseSensitivity(desc.resourceProviderIsCaseSensitive);
	apexResourceProvider->registerCallback(desc.resourceCallback);

	// The param traits depend on the resource provider, so do this now
	mParameterizedTraits = PX_NEW(ApexParameterizedTraits)();
	PX_ASSERT(mParameterizedTraits);

	/* create global name space of authorable asset types */
	mObjTypeNS = apexResourceProvider->createNameSpace(APEX_AUTHORABLE_ASSETS_TYPES_NAME_SPACE, false);

	/* create global name space of NxParameterized authorable asset types */
	mNxParamObjTypeNS = apexResourceProvider->createNameSpace(APEX_NX_PARAM_AUTH_ASSETS_TYPES_NAME_SPACE, false);

	/* create namespace for user materials */
	mMaterialNS = apexResourceProvider->createNameSpace(APEX_MATERIALS_NAME_SPACE, true);

	/* create namespace for user opaque meshes */
	mOpaqueMeshNS = apexResourceProvider->createNameSpace(APEX_OPAQUE_MESH_NAME_SPACE, false);

	/* create namespace for custom vertex buffer semantics */
	mCustomVBNS = apexResourceProvider->createNameSpace(APEX_CUSTOM_VB_NAME_SPACE, true);

	/* create namespace for novodex collision groups */
	mCollGroupNS = apexResourceProvider->createNameSpace(APEX_COLLISION_GROUP_NAME_SPACE, false);

	/* create namespace for 128-bit NxGroupsMasks */
	mCollGroup128NS = apexResourceProvider->createNameSpace(APEX_COLLISION_GROUP_128_NAME_SPACE, false);

	/* create namespace for 64-bit NxGroupsMasks64 */
	mCollGroup64NS = apexResourceProvider->createNameSpace(APEX_COLLISION_GROUP_64_NAME_SPACE, false);

	/* create namespace for novodex collision groups masks */
	mCollGroupMaskNS = apexResourceProvider->createNameSpace(APEX_COLLISION_GROUP_MASK_NAME_SPACE, false);

	/* create namespace for novodex NxMaterial IDs (returned by raycasts) */
	mPhysMatNS = apexResourceProvider->createNameSpace(APEX_PHYSICS_MATERIAL_NAME_SPACE, false);

	/* create namespace for NxRenderMeshAssets */
	mAuthorableObjects = PX_NEW(NxResourceList);
	RenderMeshAuthorableObject* AO = PX_NEW(RenderMeshAuthorableObject)(&frameworkModule, *mAuthorableObjects, RenderMeshAssetParameters::staticClassName());
	ApexRenderMeshAsset::mObjTypeID = AO->getResID();

	frameworkModule.init(mParameterizedTraits);

	/* Create mDebugColorParams */
	void* newPtr = mParameterizedTraits->alloc(sizeof(DebugColorParamsEx));
	mDebugColorParams = NX_PARAM_PLACEMENT_NEW(newPtr, DebugColorParamsEx)(mParameterizedTraits, this);

	for (physx::PxU32 i = 0 ; i < DescHashSize ; i++)
	{
		mPhysXObjDescHash[i] = 0;
	}
	mDescFreeList = 0;

	mCachedData = PX_NEW(ApexSDKCachedData);

	mBatchSeedSize = desc.physXObjDescTableAllocationIncrement;

#if PHYSX_PROFILE_SDK
	mPvdBinding = NULL;
	mProfileZone = NULL;
#if NX_SDK_VERSION_MAJOR == 2
	NxRemoteDebugger* remoteDebugger = desc.physXSDK->getFoundationSDK().getRemoteDebugger();
	if (remoteDebugger != NULL)
	{
		mPvdBinding = &PVD::Pvd28Binding::attach(*remoteDebugger, false);
	}
#else
	if (physXSDK->getPvdConnectionManager() != NULL && physXSDK->getProfileZoneManager())
	{
		mPvdBinding = PVD::PvdBinding::create(*physXSDK->getPvdConnectionManager(), *physXSDK->getProfileZoneManager());
	}
#endif //sdk version
	if ( mPvdBinding )
	{
		mProfileZone = &mPvdBinding->getProfileManager().createProfileZone("ApexSDK", PxProfileNames(), 0x4000);
	}
	else
	{
		mProfileZone = PX_NEW(ApexStubPxProfileZone)();
	}
	gProfileZone = mProfileZone;
#endif //remote debugger
}


physx::Foundation*	ApexSDK::getFoundation() const
{
	return &physx::Foundation::getInstance();
}


ApexSDK::~ApexSDK()
{
#if defined(PX_DEBUG) || defined(PX_CHECKED)
	if (mURRdepthTLSslot != 0xFFFFFFFF)
	{
		TlsFree(mURRdepthTLSslot);
		mURRdepthTLSslot = 0xFFFFFFFF;
	}
#endif

	Framework::releaseFrameworkProfiling();
#if PHYSX_PROFILE_SDK
	if ( mProfileZone )
	{
		mProfileZone->release();
		mProfileZone = NULL;
	}
	if (mPvdBinding != NULL)
	{
		mPvdBinding->release();
	}
	mPvdBinding = NULL;
#endif
}

int ApexSDK::getSuggestedCudaDeviceOrdinal()
{
#if defined(PX_WINDOWS) && !defined(PX_WINMODERN) && PX_SUPPORT_GPU_PHYSX == 1
	return physx::PxGetSuggestedCudaDeviceOrdinal(*getErrorCallback());
#else
	return -1;
#endif
}

ApexActor* ApexSDK::getApexActor(NxApexActor* nxactor) const
{
	NxAuthObjTypeID type = nxactor->getOwner()->getObjTypeID();
	if (type == ApexRenderMeshAsset::mObjTypeID)
	{
		return (ApexRenderMeshActor*) nxactor;
	}

	ApexActor* a = NULL;
	for (physx::PxU32 i = 0; i < imodules.size(); i++)
	{
		a = imodules[i]->getApexActor(nxactor, type);
		if (a)
		{
			break;
		}
	}

	return a;
}

NxApexScene* ApexSDK::createScene(const NxApexSceneDesc& sceneDesc)
{
	if (!sceneDesc.isValid())
	{
		return 0;
	}

	ApexScene* s = PX_NEW(ApexScene)(sceneDesc, this);
	mScenes.pushBack(s);

	// Trigger NiModuleScene creation for all loaded modules
	for (physx::PxU32 i = 0; i < imodules.size(); i++)
	{
		s->moduleCreated(*imodules[i]);
	}

	return s;
}

NxApexAssetPreviewScene*   ApexSDK::createAssetPreviewScene()
{
	ApexAssetPreviewScene* s = PX_NEW(ApexAssetPreviewScene)(this);
	
	return s;
}

void ApexSDK::releaseScene(NxApexScene* nxScene)
{
	ApexScene* scene = DYNAMIC_CAST(ApexScene*)(nxScene);
	mScenes.findAndReplaceWithLast(scene);
	scene->destroy();
}

void ApexSDK::releaseAssetPreviewScene(NxApexAssetPreviewScene* nxScene)
{
	ApexAssetPreviewScene* scene = DYNAMIC_CAST(ApexAssetPreviewScene*)(nxScene);
	scene->destroy();
}

/** Map PhysX objects back to their APEX objects, hold flags and pointers **/

physx::PxU16 ApexPhysXObjectDesc::makeHash(size_t hashable)
{
	return static_cast<physx::PxU16>(PX_MAX_U16 & (hashable >> 8));
}


NiApexPhysXObjectDesc* ApexSDK::getGenericPhysXObjectInfo(const void* obj) const
{
	physx::Mutex::ScopedLock scopeLock(mPhysXObjDescsLock);

	physx::PxU16 h = (physx::PxU16)(ApexPhysXObjectDesc::makeHash(reinterpret_cast<size_t>(obj)) & (DescHashSize - 1));
	physx::PxU32 index = mPhysXObjDescHash[h];

	while (index)
	{
		ApexPhysXObjectDesc* desc = const_cast<ApexPhysXObjectDesc*>(&mPhysXObjDescs[index]);
		if ((void*) desc->mPhysXObject == obj)
		{
			return desc;
		}
		else
		{
			index = desc->mNext;
		}
	}
	return NULL;
}

#if NX_SDK_VERSION_MAJOR == 2
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const NxActor* actor)
{
	return createObjectDesc(apexActor, (const void*) actor);
}
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const NxShape* shape)
{
	return createObjectDesc(apexActor, (const void*) shape);
}
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const NxJoint* joint)
{
	return createObjectDesc(apexActor, (const void*) joint);
}
NiApexPhysXObjectDesc* ApexSDK::createObjectDesc(const NxApexActor* apexActor, const NxCloth* cloth)
{
	return createObjectDesc(apexActor, (const void*)cloth);
}
NiApexPhysXObjectDesc* ApexSDK::createObjectDesc(const NxApexActor* apexActor, const NxSoftBody* softbody)
{
	return createObjectDesc(apexActor, (const void*)softbody);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const NxActor* actor) const
{
	return getGenericPhysXObjectInfo((void*)actor);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const NxShape* shape) const
{
	return getGenericPhysXObjectInfo((void*)shape);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const NxJoint* joint) const
{
	return getGenericPhysXObjectInfo((void*)joint);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const NxCloth* cloth) const
{
	return getGenericPhysXObjectInfo(cloth);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const NxSoftBody* softbody) const
{
	return getGenericPhysXObjectInfo(softbody);
}

NxPhysicsSDK* ApexSDK::getPhysXSDK()
{
	return physXSDK;
}
NxCookingInterface* ApexSDK::getCookingInterface()
{
	return cooking;
}

#else
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const PxActor* actor)
{
	return createObjectDesc(apexActor, (const void*) actor);
}
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const PxShape* shape)
{
	return createObjectDesc(apexActor, (const void*) shape);
}
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const PxJoint* joint)
{
	return createObjectDesc(apexActor, (const void*) joint);
}
NiApexPhysXObjectDesc* ApexSDK::createObjectDesc(const NxApexActor* apexActor, const PxCloth* cloth)
{
	return createObjectDesc(apexActor, (const void*)cloth);
}
NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const PxParticleSystem* particleSystem)
{
	return createObjectDesc(apexActor, (const void*) particleSystem);
}
NiApexPhysXObjectDesc* ApexSDK::createObjectDesc(const NxApexActor* apexActor, const PxParticleFluid* particleFluid)
{
	return createObjectDesc(apexActor, (const void*)particleFluid);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const PxActor* actor) const
{
	return getGenericPhysXObjectInfo((void*)actor);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const PxShape* shape) const
{
	return getGenericPhysXObjectInfo((void*)shape);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const PxJoint* joint) const
{
	return getGenericPhysXObjectInfo((void*)joint);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const PxCloth* cloth) const
{
	return getGenericPhysXObjectInfo((void*)cloth);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const PxParticleSystem* particleSystem) const
{
	return getGenericPhysXObjectInfo((void*)particleSystem);
}
const NxApexPhysXObjectDesc* ApexSDK::getPhysXObjectInfo(const PxParticleFluid* particleFluid) const
{
	return getGenericPhysXObjectInfo((void*)particleFluid);
}
PxPhysics* ApexSDK::getPhysXSDK()
{
	return physXSDK;
}
PxCooking* ApexSDK::getCookingInterface()
{
	return cooking;
}

#endif

NxAuthObjTypeID ApexSDK::registerAuthObjType(const char* authTypeName, NxResID nsid)
{
	NxAuthObjTypeID aotid = apexResourceProvider->createResource(mObjTypeNS, authTypeName, false);
	apexResourceProvider->setResource(APEX_AUTHORABLE_ASSETS_TYPES_NAME_SPACE,
	                                  authTypeName,
	                                  (void*)(size_t) nsid, false);
	return aotid;
}

NxAuthObjTypeID ApexSDK::registerAuthObjType(const char* authTypeName, NiApexAuthorableObject* authObjPtr)
{
	NxAuthObjTypeID aotid = apexResourceProvider->createResource(mObjTypeNS, authTypeName, false);
	apexResourceProvider->setResource(APEX_AUTHORABLE_ASSETS_TYPES_NAME_SPACE,
	                                  authTypeName,
	                                  (void*) authObjPtr, false);
	return aotid;
}

NxAuthObjTypeID ApexSDK::registerNxParamAuthType(const char* authTypeName, NiApexAuthorableObject* authObjPtr)
{
	NxAuthObjTypeID aotid = apexResourceProvider->createResource(mNxParamObjTypeNS, authTypeName, false);
	apexResourceProvider->setResource(APEX_NX_PARAM_AUTH_ASSETS_TYPES_NAME_SPACE,
	                                  authTypeName,
	                                  (void*) authObjPtr, false);
	return aotid;
}

void ApexSDK::unregisterAuthObjType(const char* authTypeName)
{
	apexResourceProvider->setResource(APEX_AUTHORABLE_ASSETS_TYPES_NAME_SPACE,
	                                  authTypeName,
	                                  (void*) NULL, false);
}

void ApexSDK::unregisterNxParamAuthType(const char* authTypeName)
{
	apexResourceProvider->setResource(APEX_NX_PARAM_AUTH_ASSETS_TYPES_NAME_SPACE,
	                                  authTypeName,
	                                  (void*) NULL, false);
}

NiApexAuthorableObject*	ApexSDK::getAuthorableObject(const char* authTypeName)
{
	if (!apexResourceProvider->checkResource(mObjTypeNS, authTypeName))
	{
		return NULL;
	}

	void* ao = apexResourceProvider->getResource(APEX_AUTHORABLE_ASSETS_TYPES_NAME_SPACE, authTypeName);

	return static_cast<NiApexAuthorableObject*>(ao);
}

NiApexAuthorableObject*	ApexSDK::getNxParamAuthObject(const char* NxParamName)
{
	if (!apexResourceProvider->checkResource(mNxParamObjTypeNS, NxParamName))
	{
		return NULL;
	}

	void* ao = apexResourceProvider->getResource(APEX_NX_PARAM_AUTH_ASSETS_TYPES_NAME_SPACE, NxParamName);

	return static_cast<NiApexAuthorableObject*>(ao);
}

bool ApexSDK::getAuthorableObjectNames(const char** authTypeNames, physx::PxU32& outCount, physx::PxU32 inCount)
{
	NxResID ids[128];

	if (!apexResourceProvider->getResourceIDs(APEX_AUTHORABLE_ASSETS_TYPES_NAME_SPACE, ids, outCount, 128))
	{
		return false;
	}

	if (outCount > inCount)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < outCount; i++)
	{
		authTypeNames[i] = apexResourceProvider->getResourceName(ids[i]);

#define JUST_A_TEST 0
#if JUST_A_TEST
#include <stdio.h>
		NiApexAuthorableObject* ao = (NiApexAuthorableObject*)getAuthorableObject(authTypeNames[i]);

		NxApexAsset* assetList[32];
		physx::PxU32 retCount = 0;
		ao->getAssetList(assetList, retCount, 32);

		if (retCount)
		{
			printf("%s count: %d\n", authTypeNames[i], retCount);
			const NxParameterized::Interface* p = assetList[0]->getAssetNxParameterized();
			if (p)
			{
				printf(" NxParam class name: %s\n", p->className());
			}
		}
#endif
	}

	return true;
}

NxResID ApexSDK::getApexMeshNameSpace()
{
	NiApexAuthorableObject* AO = getAuthorableObject(NX_RENDER_MESH_AUTHORING_TYPE_NAME);
	if (AO)
	{
		return AO->getResID();
	}
	else
	{
		return INVALID_RESOURCE_ID;
	}
}



NiApexPhysXObjectDesc* 	ApexSDK::createObjectDesc(const NxApexActor* apexActor, const void* nxPtr)
{
	physx::Mutex::ScopedLock scopeLock(mPhysXObjDescsLock);

	physx::PxU16 h = (physx::PxU16)(ApexPhysXObjectDesc::makeHash(reinterpret_cast<size_t>(nxPtr)) & (DescHashSize - 1));
	physx::PxU32 index = mPhysXObjDescHash[ h ];

	while (index)
	{
		ApexPhysXObjectDesc* desc = &mPhysXObjDescs[ index ];
		if (desc->mPhysXObject == nxPtr)
		{
			APEX_DEBUG_WARNING("createObjectDesc: Nx Object already registered");
			bool hasActor = false;
			for (physx::PxU32 i = desc->mApexActors.size(); i--;)
			{
				if (desc->mApexActors[i] == apexActor)
				{
					hasActor = true;
					break;
				}
			}
			if (hasActor)
			{
				APEX_DEBUG_WARNING("createObjectDesc: Nx Object already registered with the given NxApexActor");
			}
			else
			{
				desc->mApexActors.pushBack(apexActor);
			}
			return desc;
		}
		else
		{
			index = desc->mNext;
		}
	}

	// Match not found, allocate new object descriptor

	if (!mDescFreeList)
	{
		// Free list is empty, seed it with new batch
		physx::PxU32 size = mPhysXObjDescs.size();
		if (size == 0)  // special initial case, reserve entry 0
		{
			size = 1;
			mPhysXObjDescs.resize(size + mBatchSeedSize);
		}
		else
		{
			PX_PROFILER_PERF_DSCOPE("objDescsResize",size + mBatchSeedSize);

			// Instead of doing a straight resize of mPhysXObjDescs the array is resized by swapping. Doing so removes the potential 
			// copying/reallocating of the arrays held in ApexPhysXObjectDesc elements which is costly performance wise.
			physx::Array<ApexPhysXObjectDesc> swapArray;
			swapArray.swap(mPhysXObjDescs);

			mPhysXObjDescs.resize(size + mBatchSeedSize);
			ApexPhysXObjectDesc* src = swapArray.begin();
			ApexPhysXObjectDesc* dst = mPhysXObjDescs.begin();
			for (physx::PxU32 i = 0; i < size; i++)
			{
				src[i].swap(dst[i]);
			}
		}

		for (physx::PxU32 i = size ; i < size + mBatchSeedSize ; i++)
		{
			mPhysXObjDescs[i].mNext = mDescFreeList;
			mDescFreeList = i;
		}
	}

	index = mDescFreeList;
	ApexPhysXObjectDesc* desc = &mPhysXObjDescs[ index ];
	mDescFreeList = desc->mNext;

	desc->mFlags = 0;
	desc->userData = NULL;
	desc->mApexActors.reset();
	desc->mApexActors.pushBack(apexActor);
	desc->mNext = mPhysXObjDescHash[ h ];
	if (desc->mNext)
	{
		PX_ASSERT(mPhysXObjDescs[ desc->mNext ].mPrev == 0);
		mPhysXObjDescs[ desc->mNext ].mPrev = index;
	}
	desc->mPrev = 0;
	desc->mPhysXObject = nxPtr;
	mPhysXObjDescHash[ h ] = index;

	/* Calling function can set mFlags and userData */

	return desc;
}

void ApexSDK::releaseObjectDesc(void* physXObject)
{
	physx::Mutex::ScopedLock scopeLock(mPhysXObjDescsLock);

	physx::PxU16 h = (physx::PxU16)(ApexPhysXObjectDesc::makeHash(reinterpret_cast<size_t>(physXObject)) & (DescHashSize - 1));
	physx::PxU32 index = mPhysXObjDescHash[ h ];

	while (index)
	{
		ApexPhysXObjectDesc* desc = &mPhysXObjDescs[ index ];

		if (desc->mPhysXObject == physXObject)
		{
			if (desc->mPrev)
			{
				mPhysXObjDescs[ desc->mPrev ].mNext = desc->mNext;
			}
			else
			{
				mPhysXObjDescHash[ h ] = desc->mNext;
			}

			if (desc->mNext)
			{
				mPhysXObjDescs[ desc->mNext ].mPrev = desc->mPrev;
			}

			desc->mNext = mDescFreeList;
			mDescFreeList = index;

			desc->mApexActors.reset();
			return;
		}
		else
		{
			index = desc->mNext;
		}
	}

	APEX_DEBUG_WARNING("releaseObjectDesc: Unable to release object descriptor");
}


void ApexSDK::releaseModule(NxModule* module)
{
	for (physx::PxU32 i = 0; i < modules.size(); i++)
	{
		if (modules[i] != module)
		{
			continue;
		}

		// The module will remove its NiModuleScenes from each NxApexScene
		mCachedData->unregisterModuleDataCache(imodules[ i ]->getModuleDataCache());

		NiModule *im = imodules[i];
		imodules[i] = NULL;
		modules[i] = NULL;
		im->destroy();

//		modules.replaceWithLast(i);
//		imodules.replaceWithLast(i);

		break;
	}
}

void ApexSDK::registerModule(NxModule* newModule, NiModule* newIModule)
{
	
	physx::PxU32 newIndex = modules.size();
	for (physx::PxU32 i=0; i<newIndex; i++)
	{
		if ( imodules[i] == NULL )
		{
			newIndex = i;
			break;
		}
	}
	if ( newIndex == modules.size() )
	{
		modules.pushBack(newModule);
		imodules.pushBack(newIModule);
	}

	// Trigger NiModuleScene creation for all existing scenes
	for (physx::PxU32 i = 0 ; i < mScenes.size(); i++)
	{
		(DYNAMIC_CAST(ApexScene*)(mScenes[i]))->moduleCreated(*newIModule);
	}

	mCachedData->registerModuleDataCache(newIModule->getModuleDataCache());
}

NxModule* ApexSDK::createModule(const char* name, NxApexCreateError* err)
{
	if (err)
	{
		*err = APEX_CE_NO_ERROR;
	}

	// Return existing module if it's already loaded
	for (physx::PxU32 i = 0; i < modules.size(); i++)
	{
		if ( modules[i] && !strcmp(modules[ i ]->getName(), name))
		{
			NiModule *imodule = imodules[i];
			if( imodule->isCreateOk() )
			{
				return modules[ i ];
			}
			else
			{
				APEX_DEBUG_WARNING("ApexSDK::createModule(%s) Not allowed.", name );
				if (err)
				{
					*err = APEX_CE_CREATE_NO_ALLOWED;
				}
				return NULL;
			}
		}
	}

	NxModule* newModule = NULL;
	NiModule* newIModule = NULL;

#if defined(_USRDLL)
	/* Dynamically linked module libraries */

#if defined(WIN32)
	ApexSimpleString dllName = mDllLoadPath + ApexSimpleString("APEX_") + ApexSimpleString(name);
#if _DEBUG
	// Request DEBUG DLL unless the user has explicitly asked for it
	const size_t nameLen = strlen(name);
	if (nameLen <= 5 || strcmp(name + nameLen - 5, "DEBUG"))
	{
		dllName += ApexSimpleString("DEBUG");
	}
#elif defined(PX_CHECKED)
	dllName += ApexSimpleString("CHECKED");
#elif defined(PHYSX_PROFILE_SDK)
	dllName += ApexSimpleString("PROFILE");
#endif

#if defined(_DEBUG) && defined(APEX_TEST)
	dllName += ApexSimpleString("_TEST");
#elif defined(APEX_TEST)
	dllName += ApexSimpleString("TEST");
#endif

#if defined(PX_X86)
	dllName += ApexSimpleString("_x86");
#elif defined(PX_X64)
	dllName += ApexSimpleString("_x64");
#endif

	dllName += mCustomDllNamePostfix;

	dllName += ApexSimpleString(".dll");

	HMODULE library = NULL;
	{
		ModuleUpdateLoader moduleLoader(UPDATE_LOADER_DLL_NAME);
		library = moduleLoader.loadModule(dllName.c_str(), getAppGuid());

		if (NULL == library)
		{
			dllName = ApexSimpleString("APEX/") + dllName;
			library = moduleLoader.loadModule(dllName.c_str(), getAppGuid());
		}
	}

	if (library)
	{
		NxCreateModule_FUNC* createModuleFunc = (NxCreateModule_FUNC*) GetProcAddress(library, "createModule");
		if (createModuleFunc)
		{
			newModule = createModuleFunc((NiApexSDK*) this,
			                             &newIModule,
			                             NX_APEX_SDK_VERSION,
			                             NX_PHYSICS_SDK_VERSION,
			                             err);
		}
	}
#else
	/* TODO: other platform dynamic linking? */
#endif

#else
	/* Statically linked module libraries */

	/* Modules must supply an instantiation function which calls NiApexSDK::registerModule()
	 * The user must call this function after creating ApexSDK and before createModule().
	 */
#endif

	// register new module and its parameters
	if (newModule)
	{
		registerModule(newModule, newIModule);
	}
	else if (err)
	{
		*err = APEX_CE_NOT_FOUND;
	}

	return newModule;
}

NiModule* ApexSDK::getNiModuleByName(const char* name)
{
	// Return existing module if it's already loaded
	for (physx::PxU32 i = 0; i < modules.size(); i++)
	{
		if (!strcmp(modules[ i ]->getName(), name))
		{
			return imodules[ i ];
		}
	}
	return NULL;
}

PxFileBuf* ApexSDK::createStream(const char* filename, physx::PxFileBuf::OpenMode mode)
{
	return PX_NEW(PxFileBuffer)(filename, mode);
}

// deprecated, use getErrorCallback instead
physx::PxErrorCallback* ApexSDK::getOutputStream()
{
	return getErrorCallback();
}

physx::PxErrorCallback* ApexSDK::getErrorCallback()
{
	return &getFoundation()->getErrorCallback();
}

physx::ErrorHandler& ApexSDK::getErrorHandler() const
{
	return getFoundation()->getErrorHandler();
}

NxResourceProvider* ApexSDK::getNamedResourceProvider()
{
	return apexResourceProvider;
}

NiResourceProvider* ApexSDK::getInternalResourceProvider()
{
	return apexResourceProvider;
}

physx::PxU32 ApexSDK::getNbModules()
{
	PxU32 moduleCount = 0;
	for (PxU32 i=0; i<modules.size(); i++)
	{
		if (modules[i] != NULL)
		{
			moduleCount++;
		}
	}

	return moduleCount;
}

NxModule** ApexSDK::getModules()
{
	if (modules.size() > 0)
	{
		moduleListForAPI.resize(0);
		for (PxU32 i=0; i<modules.size(); i++)
		{
			if (modules[i] != NULL)
			{
				moduleListForAPI.pushBack(modules[i]);
			}
		}
		
		return &moduleListForAPI.front();
	}
	else
	{
		return NULL;
	}
}


NiModule** ApexSDK::getNiModules()
{
	if (imodules.size() > 0)
	{
		return &imodules.front();
	}
	else
	{
		return NULL;
	}
}

physx::PxU32 ApexSDK::forceLoadAssets()
{
	physx::PxU32 loadedAssetCount = 0;

	// handle render meshes, since they don't live in a module
	if (mAuthorableObjects != NULL)
	{
		for (physx::PxU32 i = 0; i < mAuthorableObjects->getSize(); i++)
		{
			NiApexAuthorableObject* ao = static_cast<NiApexAuthorableObject*>(mAuthorableObjects->getResource(i));
			loadedAssetCount += ao->forceLoadAssets();
		}
	}

	for (physx::PxU32 i = 0; i < imodules.size(); i++)
	{
		loadedAssetCount += imodules[i]->forceLoadAssets();
	}

	return loadedAssetCount;
}


void ApexSDK::debugAsset(NxApexAsset* asset, const char* name)
{
	PX_UNUSED(asset);
	PX_UNUSED(name);
#if WITH_DEBUG_ASSET
	if (asset)
	{
		const NxParameterized::Interface* pm = asset->getAssetNxParameterized();
		if (pm)
		{
			NxParameterized::Serializer* s1 = internalCreateSerializer(NxParameterized::Serializer::NST_XML, mParameterizedTraits);
			NxParameterized::Serializer* s2 = internalCreateSerializer(NxParameterized::Serializer::NST_BINARY, mParameterizedTraits);
			if (s1 && s2)
			{
				PxMemoryBuffer mb1;
				PxMemoryBuffer mb2;
				s1->serialize(mb1, &pm, 1);
				s2->serialize(mb2, &pm, 1);
				{
					char scratch[512];
					physx::string::strcpy_s(scratch, 512, name);
					char* dot = NULL;
					char* scan = scratch;
					while (*scan)
					{
						if (*scan == '/')
						{
							*scan = '_';
						}
						if (*scan == '\\')
						{
							*scan = '_';
						}
						if (*scan == '.')
						{
							dot = scan;
						}
						scan++;
					}

					if (dot)
					{
						*dot = 0;
					}

					physx::string::strcat_s(scratch, 512, ".apx");
					FILE* fph = fopen(scratch, "wb");
					if (fph)
					{
						fwrite(mb1.getWriteBuffer(), mb1.getWriteBufferSize(), 1, fph);
						fclose(fph);
					}
					if (dot)
					{
						*dot = 0;
					}

					physx::string::strcat_s(scratch, 512, ".apb");
					fph = fopen(scratch, "wb");
					if (fph)
					{
						fwrite(mb2.getWriteBuffer(), mb2.getWriteBufferSize(), 1, fph);
						fclose(fph);
					}

				}
				s1->release();
				s2->release();
			}
		}
	}
#endif
}

/**
 *	checkAssetName
 *	-If name is NULL, we'll autogenerate one that won't collide with other names and issue a warning
 *	-If name collides with another name, we'll issue a warning and return NULL, so as not to confuse the
 *	 user by creating an asset authoring with a name that's different from the name specified.
 */
const char* ApexSDK::checkAssetName(NiApexAuthorableObject& ao, const char* inName, ApexSimpleString& autoNameStorage)
{
	NiResourceProvider* iNRP = getInternalResourceProvider();

	if (!inName)
	{
		autoNameStorage = ao.getName();
		iNRP->generateUniqueName(ao.getResID(), autoNameStorage);

		APEX_DEBUG_INFO("No name provided for asset, auto-naming <%s>.", autoNameStorage.c_str());
		return autoNameStorage.c_str();
	}

	if (iNRP->checkResource(ao.getResID(), inName))
	{
		// name collides with another asset [author]
		APEX_DEBUG_WARNING("Name provided collides with another asset in the %s namespace: <%s>, no asset created.", ao.getName().c_str(), inName);

		return NULL;
	}

	return inName;
}

/**
 *	createAsset
 *	This method will load *any* APEX asset.
 *  1. Read the APEX serialization header
 *	2. Determine the correct module
 *	3. Pass the remainder of the stream to the module along with the asset type name and asset version
 */

NxApexAsset* ApexSDK::createAsset(NxApexAssetAuthoring& nxAssetAuthoring, const char* name)
{
	NxApexAsset* ret = NULL;
	NiApexAuthorableObject* ao = getAuthorableObject(nxAssetAuthoring.getObjTypeName());
	if (ao)
	{
		ApexSimpleString autoName;
		name = checkAssetName(*ao, name, autoName);
		if (!name)
		{
			return NULL;
		}

		ret = ao->createAsset(nxAssetAuthoring, name);
	}
	else
	{
		APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", nxAssetAuthoring.getObjTypeName());
	}
	debugAsset(ret, name);
	return ret;
}

NxApexAsset* ApexSDK::createAsset(NxParameterized::Interface* params, const char* name)
{
	NxApexAsset* ret = NULL;
	// params->className() will tell us the name of the parameterized struct
	// there is a mapping of parameterized structs to
	PX_ASSERT(params);
	if (params)
	{
		NiApexAuthorableObject* ao = getNxParamAuthObject(params->className());
		if (ao)
		{
			ApexSimpleString autoName;
			name = checkAssetName(*ao, name, autoName);
			if (!name)
			{
				return NULL;
			}

			ret = ao->createAsset(params, name);
		}
		else
		{
			APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", params->className());
		}
	}
	debugAsset(ret, name);
	return ret;
}

NxApexAssetAuthoring* ApexSDK::createAssetAuthoring(const char* aoTypeName)
{
	NiApexAuthorableObject* ao = getAuthorableObject(aoTypeName);
	if (ao)
	{
		ApexSimpleString autoName;
		const char* name = 0;
		name = checkAssetName(*ao, name, autoName);
		if (!name)
		{
			return NULL;
		}


		return ao->createAssetAuthoring(name);
	}
	else
	{
		APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", aoTypeName);
	}

	return NULL;
}

NxApexAssetAuthoring* ApexSDK::createAssetAuthoring(const char* aoTypeName, const char* name)
{
	NiApexAuthorableObject* ao = getAuthorableObject(aoTypeName);
	if (ao)
	{
		ApexSimpleString autoName;
		name = checkAssetName(*ao, name, autoName);
		if (!name)
		{
			return NULL;
		}

		return ao->createAssetAuthoring(name);
	}
	else
	{
		APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", aoTypeName);
	}

	return NULL;
}

NxApexAssetAuthoring* ApexSDK::createAssetAuthoring(NxParameterized::Interface* params, const char* name)
{
	PX_ASSERT(params);
	if (!params)
	{
		APEX_DEBUG_WARNING("NULL NxParameterized Interface, no asset author created.");
		return NULL;
	}

	NiApexAuthorableObject* ao = getNxParamAuthObject(params->className());
	if (ao)
	{
		ApexSimpleString autoName;
		name = checkAssetName(*ao, name, autoName);
		if (!name)
		{
			return NULL;
		}

		return ao->createAssetAuthoring(params, name);
	}
	else
	{
		APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", params->className());
	}

	return NULL;
}

/**
 *	releaseAsset
 *
 */
void ApexSDK::releaseAsset(NxApexAsset& nxasset)
{
	NiApexAuthorableObject* ao = getAuthorableObject(nxasset.getObjTypeName());
	if (ao)
	{
		return ao->releaseAsset(nxasset);
	}
	else
	{
		APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", nxasset.getObjTypeName());
	}
}

void ApexSDK::releaseAssetAuthoring(NxApexAssetAuthoring& nxAssetAuthoring)
{
	NiApexAuthorableObject* ao = getAuthorableObject(nxAssetAuthoring.getObjTypeName());
	if (ao)
	{
		return ao->releaseAssetAuthoring(nxAssetAuthoring);
	}
	else
	{
		APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", nxAssetAuthoring.getObjTypeName());
	}
}

void ApexSDK::reportError(physx::PxErrorCode::Enum code, const char* file, int line, const char* functionName, const char* msgFormat, ...)
{
	mReportErrorLock.lock();

	if (gApexSdk)
	{
		if (getErrorCallback())
		{
			if (mErrorString == NULL && code != physx::PxErrorCode::eOUT_OF_MEMORY)
			{
				mErrorString = (char*) PX_ALLOC(sizeof(char) * MAX_MSG_SIZE, PX_DEBUG_EXP("char"));
			}
			if (mErrorString != NULL)
			{
				va_list va;
				va_start(va, msgFormat);

				size_t tempLength = 0;
				if (functionName != NULL)
				{
					physx::string::sprintf_s(mErrorString,MAX_MSG_SIZE, "%s: ", functionName);
					tempLength = strlen(mErrorString);
				}

				vsprintf(mErrorString + tempLength, msgFormat, va);
				va_end(va);
				getErrorHandler().reportError(code, mErrorString, file, line);
			}
			else
			{
				// we can't allocate any memory anymore, let's hope the stack has still a bit of space
				char buf[ 100 ];
				va_list va;
				va_start(va, msgFormat);
				vsprintf(buf, msgFormat, va);
				va_end(va);
				getErrorCallback()->reportError(code, buf, file, line);
			}
		}
	}
	mReportErrorLock.unlock();
}



void* ApexSDK::getTempMemory(PxU32 size)
{
	mTempMemoryLock.lock();

	if (size == 0 || mTempMemories.size() > 100)	//later growing a size 0 allocation is not handled gracefully!
	{
		// this is most likely a leak in temp memory consumption
		mTempMemoryLock.unlock();
		return NULL;
	}

	// now find the smallest one that is bigger than 'size'
	physx::PxI32 found = -1;
	physx::PxU32 bestSize = 0;
	for (physx::PxU32 i = mNumTempMemoriesActive; i < mTempMemories.size(); i++)
	{
		if (mTempMemories[i].size >= size)
		{
			if (found == -1 || bestSize > mTempMemories[i].size)
			{
				found = (physx::PxI32)i;
				bestSize = mTempMemories[i].size;
			}
		}
	}

	TempMemory result;

	if (found != -1)
	{
		// found
		if ((physx::PxU32)found > mNumTempMemoriesActive)
		{
			// swap them
			TempMemory temp = mTempMemories[mNumTempMemoriesActive];
			mTempMemories[mNumTempMemoriesActive] = mTempMemories[(physx::PxU32)found];
			mTempMemories[(physx::PxU32)found] = temp;
		}
		PX_ASSERT(mTempMemories[mNumTempMemoriesActive].used == 0);
		mTempMemories[mNumTempMemoriesActive].used = size;
		result = mTempMemories[mNumTempMemoriesActive];
		mNumTempMemoriesActive++;
	}
	else if (mNumTempMemoriesActive < mTempMemories.size())
	{
		// not found, use last one

		// swap
		TempMemory temp = mTempMemories.back();
		mTempMemories.back() = mTempMemories[mNumTempMemoriesActive];

		void* nb = PX_ALLOC(size, PX_DEBUG_EXP("ApexSDK::getTempMemory"));
		if (nb)
		{
			memcpy(nb, temp.memory, PxMin(temp.size, size));
		}
		PX_FREE(temp.memory);
		temp.memory = nb;
		temp.size = size;
		PX_ASSERT(temp.used == 0);
		temp.used = size;

		mTempMemories[mNumTempMemoriesActive] = temp;
		result = temp;
		mNumTempMemoriesActive++;
	}
	else
	{
		mNumTempMemoriesActive++;
		TempMemory& newTemp = mTempMemories.insert();
		newTemp.memory = PX_ALLOC(size, PX_DEBUG_EXP("ApexSDK::getTempMemory"));
		newTemp.size = size;
		newTemp.used = size;
		result = newTemp;
	}
	mTempMemoryLock.unlock();

#ifdef _DEBUG
	if (result.used < result.size)
	{
		memset((char*)result.memory + result.used, 0xfd, result.size - result.used);
	}
#endif

	return result.memory;
}


void ApexSDK::releaseTempMemory(void* data)
{
	if (data == NULL)		//this is a valid consequence of permittion 0 sized allocations.
	{
		return;
	}

	mTempMemoryLock.lock();
	physx::PxU32 numReleased = 0;

	for (physx::PxU32 i = 0; i < mNumTempMemoriesActive; i++)
	{
		if (mTempMemories[i].memory == data)
		{
			PX_ASSERT(mTempMemories[i].used > 0);
#ifdef _DEBUG
			if (mTempMemories[i].used < mTempMemories[i].size)
			{
				for (PxU32 j = mTempMemories[i].used; j < mTempMemories[i].size; j++)
				{
					unsigned char cur = ((unsigned char*)mTempMemories[i].memory)[j];
					if (cur != 0xfd)
					{
						PX_ASSERT(cur == 0xfd);
						break; // only hit this assert once per error
					}
				}
			}
			// you should not operate on data that has been released!
			memset(mTempMemories[i].memory, 0xcd, mTempMemories[i].size);
#endif
			mTempMemories[i].used = 0;

			// swap with last valid one
			if (i < mNumTempMemoriesActive - 1)
			{
				TempMemory temp = mTempMemories[mNumTempMemoriesActive - 1];
				mTempMemories[mNumTempMemoriesActive - 1] = mTempMemories[i];
				mTempMemories[i] = temp;
			}
			mNumTempMemoriesActive--;

			numReleased++;
			break;
		}
	}

	PX_ASSERT(numReleased == 1);

	mTempMemoryLock.unlock();
}


void ApexSDK::release()
{
	if (renderResourceManagerWrapper != NULL)
	{
		PX_DELETE(renderResourceManagerWrapper);
		renderResourceManagerWrapper = NULL;
	}

	for (physx::PxU32 i = 0; i < mScenes.size(); i++)
	{
		(DYNAMIC_CAST(ApexScene*)(mScenes[ i ]))->destroy();
	}
	mScenes.clear();

	// Notify all modules that the ApexSDK is getting destructed
	for (physx::PxU32 i = 0; i < modules.size(); i++)
	{
		if ( imodules[i] )
		{
			imodules[ i ]->notifyReleaseSDK();
		}
	}

	// Now we destroy each module; but we make sure to null out each array element before we call the
	// actual destruction routine so that the array of avlie/registered modules contains no pointers to deleted objects
	for (physx::PxU32 i = 0; i < modules.size(); i++)
	{
		NiModule *d = imodules[i];
		imodules[i] = NULL;
		modules[i] = NULL;
		if ( d )
		{
			d->destroy();
		}
	}
	modules.clear();
	imodules.clear();

	/* Free all render meshes created from the SDK, release named resources */
	if (mAuthorableObjects != NULL)
	{
		PX_DELETE(mAuthorableObjects);
		mAuthorableObjects = NULL;
	}

	if (mDebugColorParams)
	{
		mDebugColorParams->destroy();
		mDebugColorParams = NULL;
	}

	frameworkModule.release(mParameterizedTraits);

	delete mParameterizedTraits;
	mParameterizedTraits = 0;

	apexResourceProvider->destroy();
	apexResourceProvider = 0;


	mPhysXObjDescs.clear();

	for (physx::PxU32 i = 0; i < mTempMemories.size(); i++)
	{
		if (mTempMemories[i].memory != NULL)
		{
			PX_FREE(mTempMemories[i].memory);
			mTempMemories[i].memory = NULL;
			mTempMemories[i].size = 0;
		}
	}
	mTempMemories.clear();

	PX_DELETE(mCachedData);
	mCachedData = NULL;

	if (mErrorString)
	{
		PX_FREE_AND_RESET(mErrorString);
	}

#if NX_SDK_VERSION_MAJOR == 2
	if (mApexThreadPool)
	{
		PX_DELETE(mApexThreadPool);
		mApexThreadPool = NULL;
	}
	while (mUserAllocThreadPools.size())
	{
		releaseCpuDispatcher(*mUserAllocThreadPools[0]);
	}
#endif

	// copy this variable!!
	const bool ownsFoundation = mOwnsFoundation;

	delete this;
	// be very careful what goes below this line!

	if (ownsFoundation)
	{
		physx::Foundation::destroyInstance();
	}

	gApexSdk = NULL;
}

NxApexRenderDebug* ApexSDK::createApexRenderDebug(bool useRemoteDebugVisualization, bool debugVisualizeLocally)
{
	PX_UNUSED(useRemoteDebugVisualization);
	PX_UNUSED(debugVisualizeLocally);

#ifdef WITHOUT_DEBUG_VISUALIZE
	return NULL;

#else

	physx::PxProcessRenderDebug* prd = NULL;
#ifdef PX_WINDOWS
	if (useRemoteDebugVisualization)
	{
		prd = createFileRenderDebug("ApexRenderDebug.bin", false, debugVisualizeLocally);
		if (prd == NULL && !debugVisualizeLocally)
		{
			return NULL;
		}
	}
#endif
	NiApexRenderDebug* ret = createNiApexRenderDebug(this, prd);

	return static_cast< NxApexRenderDebug*>(ret);

#endif
}

void ApexSDK::releaseApexRenderDebug(NxApexRenderDebug& debug)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(debug);
#else
	NiApexRenderDebug* d = static_cast< NiApexRenderDebug*>(&debug);
	if (d)
	{
		physx::PxProcessRenderDebug* prd = d->getProcessRenderDebug();
		if (prd)
		{
			prd->release();
		}
	}
	releaseNiApexRenderDebug(d);
#endif
}

NxApexSphereShape* ApexSDK::createApexSphereShape()
{
	ApexSphereShape* m = PX_NEW(ApexSphereShape);
	return static_cast< NxApexSphereShape*>(m);
}

NxApexCapsuleShape* ApexSDK::createApexCapsuleShape()
{
	ApexCapsuleShape* m = PX_NEW(ApexCapsuleShape);
	return static_cast< NxApexCapsuleShape*>(m);
}

NxApexBoxShape* ApexSDK::createApexBoxShape()
{
	ApexBoxShape* m = PX_NEW(ApexBoxShape);
	return static_cast< NxApexBoxShape*>(m);
}

NxApexHalfSpaceShape* ApexSDK::createApexHalfSpaceShape()
{
	ApexHalfSpaceShape* m = PX_NEW(ApexHalfSpaceShape);
	return static_cast< NxApexHalfSpaceShape*>(m);
}

void ApexSDK::releaseApexShape(NxApexShape& shape)
{
	shape.releaseApexShape();
}

const char* ApexSDK::getWireframeMaterial()
{
	return mWireframeMaterial.c_str();
}

const char* ApexSDK::getSolidShadedMaterial()
{
	return mSolidShadedMaterial.c_str();
}

PVD::PvdBinding* ApexSDK::getPvdBinding()
{
#if defined(PHYSX_PROFILE_SDK)
	return mPvdBinding;
#else
	return NULL;
#endif
}

physx::PxProfileZone * ApexSDK::getProfileZone()
{
#if defined(PHYSX_PROFILE_SDK)
	return mProfileZone;
#else
	return NULL;
#endif
}



void ApexSDK::acquirePhysXSdkLock()
{
#if NX_SDK_VERSION_MAJOR == 2
	mPhysXSceneLock.lock();
#endif
}



void ApexSDK::releasePhysXSdkLock()
{
#if NX_SDK_VERSION_MAJOR == 2
	mPhysXSceneLock.unlock();
#endif
}



#ifdef PX_WINDOWS
const char* ApexSDK::getAppGuid()
{
	return mAppGuid.c_str();
}
#endif

#if defined(APEX_CUDA_SUPPORT)
PhysXGpuIndicator* ApexSDK::registerPhysXIndicatorGpuClient()
{
	//allocate memory for the PhysXGpuIndicator
	PhysXGpuIndicator* gpuIndicator = static_cast<PhysXGpuIndicator*>(PX_ALLOC(sizeof(PhysXGpuIndicator), PX_DEBUG_EXP("PhysXGpuIndicator")));
	PX_PLACEMENT_NEW(gpuIndicator, PhysXGpuIndicator);
	
	gpuIndicator->gpuOn();
	return gpuIndicator;
}

void ApexSDK::unregisterPhysXIndicatorGpuClient(PhysXGpuIndicator* gpuIndicator)
{
	if (gpuIndicator != NULL)
	{
		gpuIndicator->~PhysXGpuIndicator();
		PX_FREE(gpuIndicator);
	}
}
#endif

void ApexSDK::updateDebugColorParams(const char* color, physx::PxU32 val)
{
	for (physx::PxU32 i = 0; i < mScenes.size(); i++)
	{
		DYNAMIC_CAST(ApexScene*)(mScenes[ i ])->updateDebugColorParams(color, val);
	}
}


bool ApexSDK::getRMALoadMaterialsLazily()
{
	return mRMALoadMaterialsLazily;
}

///////////////////////////////////////////////////////////////////////////////
// ApexRenderMeshAssetAuthoring
///////////////////////////////////////////////////////////////////////////////


NxApexAsset* RenderMeshAuthorableObject::createAsset(NxApexAssetAuthoring& author, const char* name)
{
	NxParameterized::Interface* newObj = NULL;
	author.getNxParameterized()->clone(newObj);
	return createAsset(newObj, name);
}

NxApexAsset* RenderMeshAuthorableObject::createAsset(NxParameterized::Interface* params, const char* name)
{
	ApexRenderMeshAsset* asset = PX_NEW(ApexRenderMeshAsset)(mAssets, name, 0);
	if (asset)
	{
		asset->createFromParameters((RenderMeshAssetParameters*)params);
		NiGetApexSDK()->getNamedResourceProvider()->setResource(mAOTypeName.c_str(), name, asset);
	}
	return asset;
}

void RenderMeshAuthorableObject::releaseAsset(NxApexAsset& nxasset)
{
	ApexRenderMeshAsset* aa = DYNAMIC_CAST(ApexRenderMeshAsset*)(&nxasset);
	NiGetApexSDK()->getInternalResourceProvider()->setResource(ApexRenderMeshAsset::getClassName(), nxasset.getName(), NULL, false, false);
	aa->destroy();
}

// this should no longer be called now that we're auto-assigning names in createAssetAuthoring()
NxApexAssetAuthoring* RenderMeshAuthorableObject::createAssetAuthoring()
{
	return createAssetAuthoring("");
}

NxApexAssetAuthoring* 	RenderMeshAuthorableObject::createAssetAuthoring(NxParameterized::Interface* params, const char* name)
{
#ifdef WITHOUT_APEX_AUTHORING
	PX_UNUSED(params);
	PX_UNUSED(name);
	return NULL;
#else
	ApexRenderMeshAssetAuthoring* assetAuthor = PX_NEW(ApexRenderMeshAssetAuthoring)(mAssetAuthors, (RenderMeshAssetParameters*)params, name);
	if (assetAuthor)
	{
		NiGetApexSDK()->getNamedResourceProvider()->setResource(mAOTypeName.c_str(), name, assetAuthor);
	}
	return assetAuthor;

#endif
}

NxApexAssetAuthoring* RenderMeshAuthorableObject::createAssetAuthoring(const char* name)
{
#ifdef WITHOUT_APEX_AUTHORING
	PX_UNUSED(name);
	return NULL;
#else
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	RenderMeshAssetParameters* params = (RenderMeshAssetParameters*)traits->createNxParameterized(RenderMeshAssetParameters::staticClassName());
	ApexRenderMeshAssetAuthoring* assetAuthor = PX_NEW(ApexRenderMeshAssetAuthoring)(mAssetAuthors, params, name);
	if (assetAuthor)
	{
		NiGetApexSDK()->getNamedResourceProvider()->setResource(mAOTypeName.c_str(), name, assetAuthor);
	}
	return assetAuthor;

#endif
}

void RenderMeshAuthorableObject::releaseAssetAuthoring(NxApexAssetAuthoring& nxauthor)
{
#ifdef WITHOUT_APEX_AUTHORING
	PX_UNUSED(nxauthor);
#else
	ApexRenderMeshAssetAuthoring* aa = DYNAMIC_CAST(ApexRenderMeshAssetAuthoring*)(&nxauthor);
	NiGetApexSDK()->getInternalResourceProvider()->setResource(mAOTypeName.c_str(), aa->getName(), NULL, false, false);

	aa->destroy();
#endif
}


physx::PxU32 RenderMeshAuthorableObject::forceLoadAssets()
{
	physx::PxU32 loadedAssetCount = 0;

	for (physx::PxU32 i = 0; i < mAssets.getSize(); i++)
	{
		ApexRenderMeshAsset* asset = DYNAMIC_CAST(ApexRenderMeshAsset*)(mAssets.getResource(i));
		loadedAssetCount += asset->forceLoadAssets();
	}
	return loadedAssetCount;
}


// NxApexResource methods
void RenderMeshAuthorableObject::release()
{
	// test this by releasing the module before the individual assets

	// remove all assets that we loaded (must do now else we cannot unregister)
	mAssets.clear();
	mAssetAuthors.clear();

	// remove this AO's name from the authorable namespace
	NiGetApexSDK()->unregisterAuthObjType(mAOTypeName.c_str());
	destroy();
}

void RenderMeshAuthorableObject::destroy()
{
	delete this;
}


/**
Returns a physx::PxFileBuf which reads from a buffer in memory.
*/
physx::PxFileBuf* ApexSDK::createMemoryReadStream(const void* mem, physx::PxU32 len)
{
	physx::PxFileBuf* ret = 0;

	PxMemoryBuffer* rb = PX_NEW(PxMemoryBuffer)((const physx::PxU8*)mem, len);
	ret = static_cast< physx::PxFileBuf*>(rb);

	return ret;
}

/**
Returns a physx::PxFileBuf which writes to memory.
*/
physx::PxFileBuf* ApexSDK::createMemoryWriteStream(physx::PxU32 alignment)
{
	physx::PxFileBuf* ret = 0;

	PxMemoryBuffer* mb = PX_NEW(PxMemoryBuffer)(physx::BUFFER_SIZE_DEFAULT, alignment);
	ret = static_cast< physx::PxFileBuf*>(mb);

	return ret;
}

/**
Returns the address and length of the contents of a memory write buffer stream.
*/
const void* ApexSDK::getMemoryWriteBuffer(physx::PxFileBuf& stream, physx::PxU32& len)
{
	const void* ret = 0;

	PxMemoryBuffer* wb = static_cast< PxMemoryBuffer*>(&stream);
	len = wb->getWriteBufferSize();
	ret = wb->getWriteBuffer();

	return ret;
}

/**
Releases a previously created physx::PxFileBuf used as a write or read buffer.
*/
void ApexSDK::releaseMemoryReadStream(physx::PxFileBuf& stream)
{
	PxMemoryBuffer* rb = static_cast< PxMemoryBuffer*>(&stream);
	delete rb;
}

void ApexSDK::releaseMemoryWriteStream(physx::PxFileBuf& stream)
{
	PxMemoryBuffer* wb = static_cast< PxMemoryBuffer*>(&stream);
	delete wb;
}


NxParameterized::Serializer* ApexSDK::createSerializer(NxParameterized::Serializer::SerializeType type)
{
	return NxParameterized::internalCreateSerializer(type, mParameterizedTraits);
}

NxParameterized::Serializer* ApexSDK::createSerializer(NxParameterized::Serializer::SerializeType type, NxParameterized::Traits* traits)
{
	return NxParameterized::internalCreateSerializer(type, traits);
}

NxParameterized::Serializer::SerializeType ApexSDK::getSerializeType(const void* data, PxU32 dlen)
{
	physx::PxMemoryBuffer stream(data, dlen);
	return NxParameterized::Serializer::peekSerializeType(stream);
}

NxParameterized::Serializer::SerializeType ApexSDK::getSerializeType(physx::PxFileBuf& stream)
{
	return NxParameterized::Serializer::peekSerializeType(stream);
}

NxParameterized::Serializer::ErrorType ApexSDK::getSerializePlatform(const void* data, PxU32 dlen, NxParameterized::SerializePlatform& platform)
{
	if (dlen < 56)
	{
		APEX_INVALID_PARAMETER("At least 56 Bytes are needed to read the platform of a binary file");
	}

	physx::PxMemoryBuffer stream(data, dlen);
	return NxParameterized::Serializer::peekPlatform(stream, platform);
}

NxParameterized::Serializer::ErrorType ApexSDK::getSerializePlatform(physx::PxFileBuf& stream, NxParameterized::SerializePlatform& platform)
{
	return NxParameterized::Serializer::peekPlatform(stream, platform);
}

void ApexSDK::getCurrentPlatform(NxParameterized::SerializePlatform& platform) const
{
	platform = NxParameterized::GetCurrentPlatform();
}

bool ApexSDK::getPlatformFromString(const char* name, NxParameterized::SerializePlatform& platform) const
{
	return NxParameterized::GetPlatform(name, platform);
}

const char* ApexSDK::getPlatformName(const NxParameterized::SerializePlatform& platform) const
{
	return NxParameterized::GetPlatformName(platform);
}

NxApexAsset*	ApexSDK::createAsset(const char* opaqueMeshName, NxUserOpaqueMesh* om)
{
	NxApexAsset* ret = NULL;
	NxParameterized::Interface* params = getParameterizedTraits()->createNxParameterized(RenderMeshAssetParameters::staticClassName());
	if (params)
	{
		ret = this->createAsset(params, opaqueMeshName);
		if (ret)
		{
			NxParameterized::setParamBool(*params, "isReferenced", false);
			ApexRenderMeshAsset* nrma = static_cast<ApexRenderMeshAsset*>(ret);
			nrma->setOpaqueMesh(om);
		}
		else
		{
			params->destroy();
		}
	}
	return ret;
}

NiModule *ApexSDK::getNiModule(NxModule *module)
{
	NiModule *ret = NULL;

	for (physx::PxU32 i = 0; i < modules.size(); i++)
	{
		if ( modules[ i ] == module )
		{
			ret = imodules[i];
			break;
		}
	}

	return ret;
}

NxModule *ApexSDK::getNxModule(NiModule *module)
{
	NxModule *ret = NULL;

	for (physx::PxU32 i = 0; i < imodules.size(); i++)
	{
		if ( imodules[ i ] == module )
		{
			ret = modules[i];
			break;
		}
	}

	return ret;
}


void ApexSDK::enterURR()
{
	if (mURRdepthTLSslot != 0xFFFFFFFF)
	{
		PxU64 currentDepth = PTR_TO_UINT64(TlsGet(mURRdepthTLSslot));
		++currentDepth;
		TlsSet(mURRdepthTLSslot, (void*)currentDepth);
	}
}


void ApexSDK::leaveURR()
{
	if (mURRdepthTLSslot != 0xFFFFFFFF)
	{
		PxU64 currentDepth = PTR_TO_UINT64(TlsGet(mURRdepthTLSslot));
		if (currentDepth > 0)
		{
			--currentDepth;
			TlsSet(mURRdepthTLSslot, (void*)currentDepth);
		}
		else
		{
			// if this is hit, something is wrong with the
			// URR_SCOPE implementation
			PX_ALWAYS_ASSERT();
		}
	}
}


void ApexSDK::checkURR()
{
	if (mURRdepthTLSslot != 0xFFFFFFFF)
	{
		PxU64 currentDepth = PTR_TO_UINT64(TlsGet(mURRdepthTLSslot));
		if (currentDepth == 0)
		{
			// if this assert is hit it means that
			// - either some render resources are created where it's not allowed
			//   (outside of updateRenderResources or prepareRenderResources)
			//   => change the place in code where the resource is created
			//
			// - or the updateRenderResources call is not marked with a URR_SCOPE.
			//   => add the macro
			PX_ALWAYS_ASSERT();
		}
	}
}

}
} // end namespace physx::apex

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
#include "ModuleDestructible.h"
#include "DestructibleAssetProxy.h"
#include "DestructibleActorProxy.h"
#include "DestructibleActorJointProxy.h"
#include "DestructibleScene.h"
#include "ApexSharedUtils.h"
#include "PxMemoryBuffer.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxScene.h"
#include "NxCooking.h"
#include "NxConvexMeshDesc.h"
#include "NxConvexShapeDesc.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxPhysics.h"
#include "PxScene.h"
#endif

#include "ApexStream.h"
#include "ModulePerfScope.h"
using namespace destructible;
#endif

#include "NiApexSDK.h"
#include "PsShare.h"
#include "WriteCheck.h"

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
	initModuleProfiling(inSdk, "Destructible");
	destructible::ModuleDestructible* impl = PX_NEW(destructible::ModuleDestructible)(inSdk);
	*niRef  = (NiModule*) impl;
	return (NxModule*) impl;
#else
	if (errorCode != NULL)
	{
		*errorCode = APEX_CE_WRONG_VERSION;
	}

	PX_UNUSED(niRef);
	PX_UNUSED(inSdk);
	return NULL; // Destructible Module cannot use this PhysX version
#endif
}
#else
/* Statically linking entry function */
void instantiateModuleDestructible()
{
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
	NiApexSDK* sdk = NiGetApexSDK();
	initModuleProfiling(sdk, "Destructible");
	destructible::ModuleDestructible* impl = PX_NEW(destructible::ModuleDestructible)(sdk);
	sdk->registerExternalModule((NxModule*) impl, (NiModule*) impl);
#endif
}
#endif

namespace destructible
{

#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

enum NxModuleDestructibleLODParameters
{
	/**
		Controls the maximum number of dynamic chunk islands allowed
	*/
	NX_DESTRUCTIBLE_LOD_CHUNK_ISLAND_NUM = 0,

	/**
		Controls the maximum number of levels in the chunk hierarchy
	*/
	NX_DESTRUCTIBLE_LOD_CHUNK_DEPTH = 1,

	/**
		Controls the maximum separation (both lifetime and distance) for which a dynamic chunk can exist
	*/
	NX_DESTRUCTIBLE_LOD_CHUNK_SEPARATION = 2
};

NxAuthObjTypeID DestructibleAsset::mAssetTypeID;  // Static class member
#ifdef WITHOUT_APEX_AUTHORING

class DestructibleAssetDummyAuthoring : public NxApexAssetAuthoring, public UserAllocated
{
public:
	DestructibleAssetDummyAuthoring(ModuleDestructible* module, NxResourceList& list, NxParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	DestructibleAssetDummyAuthoring(ModuleDestructible* module, NxResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	DestructibleAssetDummyAuthoring(ModuleDestructible* module, NxResourceList& list)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
	}

	virtual ~DestructibleAssetDummyAuthoring() {}

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
		delete this;
	}


#if 0
	/**
	* \brief Save asset configuration to a stream
	*/
	virtual physx::PxFileBuf& serialize(physx::PxFileBuf& stream) const
	{
		PX_ASSERT(0);
		return stream;
	}

	/**
	* \brief Load asset configuration from a stream
	*/
	virtual physx::PxFileBuf& deserialize(physx::PxFileBuf& stream)
	{
		PX_ASSERT(0);
		return stream;
	}
#endif

	const char* getName(void) const
	{
		return NULL;
	}

	/**
	* \brief Returns the name of this APEX authorable object type
	*/
	virtual const char* getObjTypeName() const
	{
		return NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME;
	}

	/**
	 * \brief Prepares a fully authored Asset Authoring object for a specified platform
	 */
	virtual bool prepareForPlatform(physx::apex::NxPlatformTag)
	{
		PX_ASSERT(0);
		return false;
	}

	/**
	* \brief Save asset's NxParameterized interface, may return NULL
	*/
	virtual NxParameterized::Interface* getNxParameterized() const
	{
		PX_ASSERT(0);
		return NULL;
	}

	virtual NxParameterized::Interface* releaseAndReturnNxParameterizedInterface(void)
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	}
};

typedef ApexAuthorableObject<ModuleDestructible, DestructibleAssetProxy, DestructibleAssetDummyAuthoring> DestructionAO;


#else
typedef ApexAuthorableObject<ModuleDestructible, DestructibleAssetProxy, DestructibleAssetAuthoringProxy> DestructionAO;
#endif


ModuleDestructible::ModuleDestructible(NiApexSDK* inSdk) :
	m_isInitialized(false),
	m_maxChunkDepthOffset(0),
	m_maxChunkSeparationLOD(0.5f),
	m_maxFracturesProcessedPerFrame(PX_MAX_U32),
	m_maxActorsCreateablePerFrame(PX_MAX_U32),
	m_dynamicActorFIFOMax(0),
	m_chunkFIFOMax(0),
	m_sortByBenefit(false),
	m_chunkReport(NULL),
	m_impactDamageReport(NULL),
	m_chunkReportBitMask(0xffffffff),
	m_destructiblePhysXActorReport(NULL),
	m_chunkReportMaxFractureEventDepth(0xffffffff),
	m_chunkStateEventCallbackSchedule(NxDestructibleCallbackSchedule::Disabled),
	m_chunkCrumbleReport(NULL),
	m_chunkDustReport(NULL),
	m_massScale(1.0f),
	m_scaledMassExponent(0.5f),
	mApexDestructiblePreviewParams(NULL),
	mUseLegacyChunkBoundsTesting(false),
	mUseLegacyDamageRadiusSpread(false),
	mChunkCollisionHullCookingScale(1.0f),
	mFractureTools(NULL)
{
	name = "Destructible";
	mSdk = inSdk;
	mApiProxy = this;
	mModuleParams = NULL;

	NxParameterized::Traits* traits = mSdk->getParameterizedTraits();
	if (traits)
	{
#		define PARAM_CLASS(clas) PARAM_CLASS_REGISTER_FACTORY(traits, clas)
#		include "DestructibleParamClasses.inc"

		mApexDestructiblePreviewParams = traits->createNxParameterized(DestructiblePreviewParam::staticClassName());
	}

	/* Register this module's authorable object types and create their namespaces */
	const char* pName = DestructibleAssetParameters::staticClassName();
	DestructionAO* eAO = PX_NEW(DestructionAO)(this, mAuthorableObjects, pName);
	DestructibleAsset::mAssetTypeID = eAO->getResID();
	//	NX_DESTRUCTIBLE_LOD_CHUNK_ISLAND_NUM = 0
	registerLODParameter("ChunkIslandNum", NxRange<physx::PxU32>(1, 10));
	//	NX_DESTRUCTIBLE_LOD_CHUNK_DEPTH = 1
	registerLODParameter("ChunkDepth", NxRange<physx::PxU32>(1, 10));
	//	NX_DESTRUCTIBLE_LOD_CHUNK_SEPARATION = 2
	registerLODParameter("ChunkSeparation", NxRange<physx::PxU32>(1, 10));

	mCachedData = PX_NEW(DestructibleModuleCachedData)(getModuleID());
	// Set per-platform unit cost.  One unit is one chunk.
#if defined(PX_WINDOWS) || defined( PX_XBOXONE )
	mLodUnitCost = 0.01f;
#elif defined(PX_PS4)
	mLodUnitCost = 0.01f;
#elif defined(PX_X360)
	mLodUnitCost = 0.1f;
#elif defined(PX_PS3)
	mLodUnitCost = 0.1f;
#elif defined(PX_ANDROID)
	mLodUnitCost = 0.1f;
#elif defined(PX_LINUX)
	mLodUnitCost = 0.01f;
#else
	// Using default value set in Module class
#endif

#ifndef WITHOUT_APEX_AUTHORING
	mFractureTools = PX_NEW(FractureTools)();
#endif
}

NxAuthObjTypeID ModuleDestructible::getModuleID() const
{
	NX_READ_ZONE();
	return DestructibleAsset::mAssetTypeID;
}

ModuleDestructible::~ModuleDestructible()
{
	m_destructibleSceneList.clear();

	// This needs to happen after the scene list is cleared (actors do stuff)
	releaseModuleProfiling();

	PX_DELETE(mCachedData);
	mCachedData = NULL;
}

NxParameterized::Interface* ModuleDestructible::getDefaultModuleDesc()
{
	NX_READ_ZONE();
	NxParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(DestructibleModuleParameters*)
		                (traits->createNxParameterized("DestructibleModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

void ModuleDestructible::init(NxParameterized::Interface& desc)
{
	NX_WRITE_ZONE();
	if (strcmp(desc.className(), DestructibleModuleParameters::staticClassName()) == 0)
	{
		DestructibleModuleParameters* params = DYNAMIC_CAST(DestructibleModuleParameters*)(&desc);
		setValidBoundsPadding(params->validBoundsPadding);
		setMaxDynamicChunkIslandCount(params->maxDynamicChunkIslandCount);
		setSortByBenefit(params->sortFIFOByBenefit);
		setMaxChunkSeparationLOD(params->maxChunkSeparationLOD);
		setMaxActorCreatesPerFrame(params->maxActorCreatesPerFrame);
		setMaxChunkDepthOffset(params->maxChunkDepthOffset);

		if (params->massScale > 0.0f)
		{
			m_massScale = params->massScale;
		}

		if (params->scaledMassExponent > 0.0f && params->scaledMassExponent <= 1.0f)
		{
			m_scaledMassExponent = params->scaledMassExponent;
		}

		m_isInitialized = true;
		for (PxU32 i = 0; i < m_destructibleSceneList.getSize(); ++i)
		{
			// when module is created after the scene
			(DYNAMIC_CAST(DestructibleScene*)(m_destructibleSceneList.getResource(i)))->initModuleSettings();
		}
	}
	else
	{
		APEX_INVALID_PARAMETER("The NxParameterized::Interface object is the wrong type");
	}
}

NiModuleScene* ModuleDestructible::createNiModuleScene(NiApexScene& scene, NiApexRenderDebug* debugRender)
{
	return PX_NEW(DestructibleScene)(*this, scene, debugRender, m_destructibleSceneList);
}

void ModuleDestructible::releaseNiModuleScene(NiModuleScene& scene)
{
	DestructibleScene* ds = DYNAMIC_CAST(DestructibleScene*)(&scene);
	ds->destroy();
}

physx::PxU32 ModuleDestructible::forceLoadAssets()
{
	physx::PxU32 loadedAssetCount = 0;

	for (physx::PxU32 i = 0; i < mAuthorableObjects.getSize(); i++)
	{
		NiApexAuthorableObject* ao = static_cast<NiApexAuthorableObject*>(mAuthorableObjects.getResource(i));
		loadedAssetCount += ao->forceLoadAssets();
	}
	return loadedAssetCount;
}

DestructibleScene* ModuleDestructible::getDestructibleScene(const NxApexScene& apexScene) const
{
	for (physx::PxU32 i = 0 ; i < m_destructibleSceneList.getSize() ; i++)
	{
		DestructibleScene* ds = DYNAMIC_CAST(DestructibleScene*)(m_destructibleSceneList.getResource(i));
		if (ds->mApexScene == &apexScene)
		{
			return ds;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate DestructibleScene");
	return NULL;
}

NxApexRenderableIterator* ModuleDestructible::createRenderableIterator(const NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	DestructibleScene* ds = getDestructibleScene(apexScene);
	if (ds)
	{
		return ds->createRenderableIterator();
	}

	return NULL;
}

NxDestructibleActorJoint* ModuleDestructible::createDestructibleActorJoint(const NxDestructibleActorJointDesc& destructibleActorJointDesc, NxApexScene& scene)
{
	NX_WRITE_ZONE();
	if (!destructibleActorJointDesc.isValid())
	{
		return NULL;
	}
	DestructibleScene* 	ds = getDestructibleScene(scene);
	if (ds)
	{
		return ds->createDestructibleActorJoint(destructibleActorJointDesc);
	}
	else
	{
		return NULL;
	}
}

bool ModuleDestructible::isDestructibleActorJointActive(const NxDestructibleActorJoint* candidateJoint, NxApexScene& apexScene) const
{
	NX_READ_ZONE();
	PX_ASSERT(candidateJoint != NULL);
	PX_ASSERT(&apexScene != NULL);
	DestructibleScene* destructibleScene = NULL;
	destructibleScene = getDestructibleScene(apexScene);
	PX_ASSERT(destructibleScene != NULL);
	bool found = false;
	if (destructibleScene != NULL)
	{
		for (physx::PxU32 index = 0; index < destructibleScene->mDestructibleActorJointList.getSize(); ++index)
		{
			NxDestructibleActorJoint* activeJoint = NULL;
			activeJoint = static_cast<NxDestructibleActorJoint*>(static_cast<DestructibleActorJointProxy*>((destructibleScene->mDestructibleActorJointList.getResource(index))));
			PX_ASSERT(activeJoint != NULL);
			if (activeJoint == candidateJoint)
			{
				found = true;
				break;
			}
		}
	}
	return found;
}

void ModuleDestructible::setMaxDynamicChunkIslandCount(physx::PxU32 maxCount)
{
	NX_WRITE_ZONE();
	m_dynamicActorFIFOMax = maxCount;
}

void ModuleDestructible::setMaxChunkCount(physx::PxU32 maxCount)
{
	NX_WRITE_ZONE();
	m_chunkFIFOMax = maxCount;
}

void ModuleDestructible::setSortByBenefit(bool sortByBenefit)
{
	NX_WRITE_ZONE();
	m_sortByBenefit = sortByBenefit;
}

void ModuleDestructible::setMaxChunkDepthOffset(physx::PxU32 offset)
{
	NX_WRITE_ZONE();
	m_maxChunkDepthOffset = offset;
}

void ModuleDestructible::setMaxChunkSeparationLOD(physx::PxF32 separationLOD)
{
	NX_WRITE_ZONE();
	m_maxChunkSeparationLOD = physx::PxClamp(separationLOD, 0.0f, 1.0f);
}

void ModuleDestructible::setValidBoundsPadding(physx::PxF32 pad)
{
	NX_WRITE_ZONE();
	m_validBoundsPadding = pad;
}

void ModuleDestructible::setChunkReport(NxUserChunkReport* chunkReport)
{
	NX_WRITE_ZONE();
	m_chunkReport = chunkReport;
}

void ModuleDestructible::setImpactDamageReportCallback(NxUserImpactDamageReport* impactDamageReport)
{
	NX_WRITE_ZONE();
	m_impactDamageReport = impactDamageReport;
}

void ModuleDestructible::setChunkReportBitMask(physx::PxU32 chunkReportBitMask)
{
	NX_WRITE_ZONE();
	m_chunkReportBitMask = chunkReportBitMask;
}

void ModuleDestructible::setDestructiblePhysXActorReport(NxUserDestructiblePhysXActorReport* destructiblePhysXActorReport)
{
	NX_WRITE_ZONE();
	m_destructiblePhysXActorReport = destructiblePhysXActorReport;
}

void ModuleDestructible::setChunkReportMaxFractureEventDepth(physx::PxU32 chunkReportMaxFractureEventDepth)
{
	NX_WRITE_ZONE();
	m_chunkReportMaxFractureEventDepth = chunkReportMaxFractureEventDepth;
}

void ModuleDestructible::scheduleChunkStateEventCallback(NxDestructibleCallbackSchedule::Enum chunkStateEventCallbackSchedule)
{
	NX_WRITE_ZONE();
	if (chunkStateEventCallbackSchedule >= (NxDestructibleCallbackSchedule::Enum)0 && chunkStateEventCallbackSchedule < NxDestructibleCallbackSchedule::Count)
	{
		m_chunkStateEventCallbackSchedule = chunkStateEventCallbackSchedule;
	}
}

void ModuleDestructible::setChunkCrumbleReport(NxUserChunkParticleReport* chunkCrumbleReport)
{
	NX_WRITE_ZONE();
	m_chunkCrumbleReport = chunkCrumbleReport;
}

void ModuleDestructible::setChunkDustReport(NxUserChunkParticleReport* chunkDustReport)
{
	NX_WRITE_ZONE();
	m_chunkDustReport = chunkDustReport;
}

#if NX_SDK_VERSION_MAJOR == 2
void ModuleDestructible::setWorldSupportPhysXScene(NxApexScene& apexScene, NxScene* physxScene)
#elif NX_SDK_VERSION_MAJOR == 3
void ModuleDestructible::setWorldSupportPhysXScene(NxApexScene& apexScene, PxScene* physxScene)
#endif
{
	NX_WRITE_ZONE();
	DestructibleScene* ds = getDestructibleScene(apexScene);
	if (ds)
	{
		ds->setWorldSupportPhysXScene(physxScene);
	}
}

#if NX_SDK_VERSION_MAJOR == 2
bool ModuleDestructible::owns(const NxActor* actor) const
#elif NX_SDK_VERSION_MAJOR == 3
bool ModuleDestructible::owns(const PxRigidActor* actor) const
#endif
{
	NX_READ_ZONE();
	const NiApexPhysXObjectDesc* desc = static_cast<const NiApexPhysXObjectDesc*>(mSdk->getPhysXObjectInfo(actor));
	if (desc != NULL)
	{
		const physx::PxU32 actorCount = desc->mApexActors.size();
		for (physx::PxU32 i = 0; i < actorCount; ++i)
		{
			const NxApexActor* actor = desc->mApexActors[i];
			if (actor != NULL && actor->getOwner()->getObjTypeID() == DestructibleAsset::mAssetTypeID)
			{
				return true;
			}
		}
	}

	return false;
}

#if NX_SDK_VERSION_MAJOR == 3
#if APEX_RUNTIME_FRACTURE
bool ModuleDestructible::isRuntimeFractureShape(const PxShape& shape) const
{
	NX_READ_ZONE();
	for (physx::PxU32 i = 0 ; i < m_destructibleSceneList.getSize() ; i++)
	{
		DestructibleScene* ds = DYNAMIC_CAST(DestructibleScene*)(m_destructibleSceneList.getResource(i));
		physx::fracture::SimScene* simScene = ds->getDestructibleRTScene(false);
		if (simScene && simScene->owns(shape))
		{
			return true;
		}
	}
	return false;
}
#endif
#endif


NxDestructibleActor* ModuleDestructible::getDestructibleAndChunk(const NxShape* shape, physx::PxI32* chunkIndex) const
{
	NX_READ_ZONE();
	const NiApexPhysXObjectDesc* desc = static_cast<const NiApexPhysXObjectDesc*>(mSdk->getPhysXObjectInfo(shape));
	if (desc != NULL)
	{
		const physx::PxU32 actorCount = desc->mApexActors.size();
		PX_ASSERT(actorCount == 1);	// Shapes should only be associated with one chunk
		if (actorCount > 0)
		{
			const DestructibleActorProxy* actorProxy = (DestructibleActorProxy*)desc->mApexActors[0];
			if (actorProxy->getOwner()->getObjTypeID() == DestructibleAsset::mAssetTypeID)
			{
				const DestructibleScene* ds = actorProxy->impl.getDestructibleScene();
				DestructibleStructure::Chunk* chunk = (DestructibleStructure::Chunk*)desc->userData;
				if (chunk == NULL || chunk->destructibleID == DestructibleStructure::InvalidID || chunk->destructibleID > ds->mDestructibles.capacity())
				{
					return NULL;
				}

				DestructibleActor* destructible = ds->mDestructibles.direct(chunk->destructibleID);
				if (destructible == NULL)
				{
					return NULL;
				}

				if (chunkIndex)
				{
					*chunkIndex = (physx::PxI32)chunk->indexInAsset;
				}
				return destructible->getAPI();
			}
		}
	}

	return NULL;
}

void ModuleDestructible::applyRadiusDamage(NxApexScene& scene, physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, physx::PxF32 radius, bool falloff)
{
	NX_WRITE_ZONE();
	DestructibleScene* ds = getDestructibleScene(scene);
	if (ds)
	{
		ds->applyRadiusDamage(damage, momentum, position, radius, falloff);
	}
}

void ModuleDestructible::setMaxActorCreatesPerFrame(physx::PxU32 maxActorsPerFrame)
{
	NX_WRITE_ZONE();
	m_maxActorsCreateablePerFrame = maxActorsPerFrame;
}

void ModuleDestructible::setMaxFracturesProcessedPerFrame(physx::PxU32 maxFracturesProcessedPerFrame)
{
	NX_WRITE_ZONE();
	m_maxFracturesProcessedPerFrame = maxFracturesProcessedPerFrame;
}

void ModuleDestructible::setIntValue(physx::PxU32 parameterIndex, physx::PxU32 value)
{
	NX_WRITE_ZONE();
	Module::setIntValue(parameterIndex, value);
	switch (parameterIndex)
	{
	case NX_DESTRUCTIBLE_LOD_CHUNK_ISLAND_NUM:
	{
		physx::PxF32 value = getCurrentValue(NxRange<physx::PxU32>(10, 1000), parameterIndex);
		setMaxDynamicChunkIslandCount((physx::PxU32)(value + 0.5f));
	}
	break;
	case NX_DESTRUCTIBLE_LOD_CHUNK_DEPTH:
	{
		physx::PxF32 value = getCurrentValue(NxRange<physx::PxU32>(2, 0), parameterIndex);
		setMaxChunkDepthOffset((physx::PxU32)(value + 0.5f));
	}
	break;
	case NX_DESTRUCTIBLE_LOD_CHUNK_SEPARATION:
	{
		physx::PxF32 value = getCurrentValue(NxRange<physx::PxU32>(0, 1), parameterIndex);
		setMaxChunkSeparationLOD(value);
	}
	break;
	}
}

#if 0 // dead code
void ModuleDestructible::releaseBufferedConvexMeshes()
{
	for (physx::PxU32 i = 0; i < convexMeshKillList.size(); i++)
	{
#if NX_SDK_VERSION_MAJOR == 2
		NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMeshKillList[i]);
#elif NX_SDK_VERSION_MAJOR == 3
		convexMeshKillList[i]->release();
#endif
	}
	convexMeshKillList.clear();
}
#endif

void ModuleDestructible::destroy()
{
#ifndef WITHOUT_APEX_AUTHORING
	if (mFractureTools != NULL)
	{
		PX_DELETE(mFractureTools);
		mFractureTools = NULL;
	}
#endif

	NxParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (mModuleParams != NULL)
	{
		mModuleParams->destroy();
		mModuleParams = NULL;
	}

	if (mApexDestructiblePreviewParams != NULL)
	{
		mApexDestructiblePreviewParams->destroy();
		mApexDestructiblePreviewParams = NULL;
	}

	// base class
	Module::destroy();

#if 0 //dead code
	releaseBufferedConvexMeshes();
#endif

	delete this;

	if (traits)
	{
#		define PARAM_CLASS(clas) PARAM_CLASS_REMOVE_FACTORY(traits, clas)
#		include "DestructibleParamClasses.inc"
	}

}

/*** ModuleDestructible::SyncParams ***/
bool ModuleDestructible::setSyncParams(UserDamageEventHandler * userDamageEventHandler, UserFractureEventHandler * userFractureEventHandler, UserChunkMotionHandler * userChunkMotionHandler)
{
	NX_WRITE_ZONE();
	bool validEntry = false;
	validEntry = ((NULL != userChunkMotionHandler) ? (NULL != userDamageEventHandler || NULL != userFractureEventHandler) : true);
	if(validEntry)
	{
		mSyncParams.userDamageEventHandler		= userDamageEventHandler;
		mSyncParams.userFractureEventHandler	= userFractureEventHandler;
		mSyncParams.userChunkMotionHandler		= userChunkMotionHandler;
	}
	return validEntry;
}

typedef ModuleDestructible::SyncParams SyncParams;

SyncParams::SyncParams()
:userDamageEventHandler(NULL)
,userFractureEventHandler(NULL)
,userChunkMotionHandler(NULL)
{
}

SyncParams::~SyncParams()
{
    userChunkMotionHandler = NULL;
	userFractureEventHandler = NULL;
	userDamageEventHandler = NULL;
}

UserDamageEventHandler * SyncParams::getUserDamageEventHandler() const
{
	return userDamageEventHandler;
}

UserFractureEventHandler * SyncParams::getUserFractureEventHandler() const
{
	return userFractureEventHandler;
}

UserChunkMotionHandler * SyncParams::getUserChunkMotionHandler() const
{
	return userChunkMotionHandler;
}

template<typename T> physx::PxU32 SyncParams::getSize() const
{
	return sizeof(T);
}

template physx::PxU32 SyncParams::getSize<NxApexDamageEventHeader>		() const;
template physx::PxU32 SyncParams::getSize<NxApexDamageEventUnit>		() const;
template physx::PxU32 SyncParams::getSize<NxApexFractureEventHeader>	() const;
template physx::PxU32 SyncParams::getSize<NxApexFractureEventUnit>		() const;
template physx::PxU32 SyncParams::getSize<NxApexChunkTransformHeader>	() const;
template physx::PxU32 SyncParams::getSize<NxApexChunkTransformUnit>		() const;

const SyncParams & ModuleDestructible::getSyncParams() const
{
    return mSyncParams;
}

void ModuleDestructible::setUseLegacyChunkBoundsTesting(bool useLegacyChunkBoundsTesting)
{
	NX_WRITE_ZONE();
	mUseLegacyChunkBoundsTesting = useLegacyChunkBoundsTesting;
}

void ModuleDestructible::setUseLegacyDamageRadiusSpread(bool useLegacyDamageRadiusSpread)
{
	NX_WRITE_ZONE();
	mUseLegacyDamageRadiusSpread = useLegacyDamageRadiusSpread;
}

bool ModuleDestructible::setMassScaling(physx::PxF32 massScale, physx::PxF32 scaledMassExponent, NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		return dscene->setMassScaling(massScale, scaledMassExponent);
	}

	return false;
}

void ModuleDestructible::invalidateBounds(const physx::PxBounds3* bounds, physx::PxU32 boundsCount, NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		dscene->invalidateBounds(bounds, boundsCount);
	}
}

void ModuleDestructible::setDamageApplicationRaycastFlags(physx::NxDestructibleActorRaycastFlags::Enum flags, NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		dscene->setDamageApplicationRaycastFlags(flags);
	}
}

bool ModuleDestructible::setRenderLockMode(NxApexRenderLockMode::Enum renderLockMode, NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		return dscene->setRenderLockMode(renderLockMode);
	}

	return false;
}

NxApexRenderLockMode::Enum ModuleDestructible::getRenderLockMode(const NxApexScene& apexScene) const
{
	NX_READ_ZONE();
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		return dscene->getRenderLockMode();
	}

	return NxApexRenderLockMode::NO_RENDER_LOCK;
}

bool ModuleDestructible::lockModuleSceneRenderLock(NxApexScene& apexScene)
{	
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		return dscene->lockModuleSceneRenderLock();
	}

	return false;
}

bool ModuleDestructible::unlockModuleSceneRenderLock(NxApexScene& apexScene)
{
	DestructibleScene* dscene = getDestructibleScene(apexScene);

	if (dscene != NULL)
	{
		return dscene->unlockModuleSceneRenderLock();
	}

	return false;
}

bool ModuleDestructible::setChunkCollisionHullCookingScale(const physx::PxVec3& scale)
{
	NX_WRITE_ZONE();
	if (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f)
	{
		return false;
	}

	mChunkCollisionHullCookingScale = scale;

	return true;
}


/*******************************
* DestructibleModuleCachedData *
*******************************/

DestructibleModuleCachedData::DestructibleModuleCachedData(NxAuthObjTypeID moduleID) :
	mModuleID(moduleID)
{
}

DestructibleModuleCachedData::~DestructibleModuleCachedData()
{
	clear();
}

NxParameterized::Interface* DestructibleModuleCachedData::getCachedDataForAssetAtScale(NxApexAsset& asset, const physx::PxVec3& scale)
{
	DestructibleAsset& dasset = DYNAMIC_CAST(DestructibleAssetProxy*)(&asset)->impl;

	DestructibleAssetCollision* collisionSet = findAssetCollisionSet(asset.getName());
	if (collisionSet == NULL)
	{
		collisionSet = PX_NEW(DestructibleAssetCollision);
		collisionSet->setDestructibleAssetToCook(&dasset);
		mAssetCollisionSets.pushBack(collisionSet);
	}

	return collisionSet->getCollisionAtScale(scale);
}

physx::PxFileBuf& DestructibleModuleCachedData::serialize(physx::PxFileBuf& stream) const
{
	stream << (physx::PxU32)Version::Current;

	stream << (physx::PxU32)mModuleID;

	stream << mAssetCollisionSets.size();
	for (physx::PxU32 i = 0; i < mAssetCollisionSets.size(); ++i)
	{
		mAssetCollisionSets[i]->serialize(stream);
	}

	return stream;
}

physx::PxFileBuf& DestructibleModuleCachedData::deserialize(physx::PxFileBuf& stream)
{
	clear(false);	// false => don't delete cached data for referenced sets

	/*const physx::PxU32 version =*/
	stream.readDword();	// Original version

	mModuleID = stream.readDword();

	const physx::PxU32 dataSetCount = stream.readDword();
	for (physx::PxU32 i = 0; i < dataSetCount; ++i)
	{
		DestructibleAssetCollision* collisionSet = PX_NEW(DestructibleAssetCollision);
		collisionSet->deserialize(stream, NULL);
		// See if we already have a set for the asset
		DestructibleAssetCollision* destinationSet = NULL;
		for (physx::PxU32 j = 0; j < mAssetCollisionSets.size(); ++j)
		{
			if (!physx::string::stricmp(mAssetCollisionSets[j]->getAssetName(), collisionSet->getAssetName()))
			{
				destinationSet = mAssetCollisionSets[j];
				break;
			}
		}
		if (destinationSet == NULL)
		{
			// We don't have a set for this asset.  Simply add it.
			destinationSet = mAssetCollisionSets.pushBack(collisionSet);
		}
		else
		{
			// We already have a set for this asset.  Merge.
			destinationSet->merge(*collisionSet);
			PX_DELETE(collisionSet);
		}
		destinationSet->cookAll();	// This should only cook what isn't already cooked
	}

	return stream;
}

physx::PxFileBuf& DestructibleModuleCachedData::serializeSingleAsset(NxApexAsset& asset, physx::PxFileBuf& stream)
{
	DestructibleAssetCollision* collisionSet = findAssetCollisionSet(asset.getName());
	if( collisionSet )
	{
		collisionSet->serialize(stream);
	}

	return stream;
}

physx::PxFileBuf& DestructibleModuleCachedData::deserializeSingleAsset(NxApexAsset& asset, physx::PxFileBuf& stream)
{
	DestructibleAssetCollision* collisionSet = findAssetCollisionSet(asset.getName());
	if (collisionSet == NULL)
	{
		collisionSet = PX_NEW(DestructibleAssetCollision);
		mAssetCollisionSets.pushBack(collisionSet);
	}
	collisionSet->deserialize(stream, asset.getName());

	return stream;
}

void DestructibleModuleCachedData::clear(bool force)
{
	if (force)
	{
		// force == true, so delete everything
		for (physx::PxU32 i = mAssetCollisionSets.size(); i--;)
		{
			DestructibleAssetCollision* collisionSet = mAssetCollisionSets[i];
			if (collisionSet != NULL)
			{
				// Spit out warnings to the error stream for any referenced sets
				collisionSet->reportReferencedSets();
				collisionSet->clearUnreferencedSets();
			}
			PX_DELETE(mAssetCollisionSets[i]);
		}
		mAssetCollisionSets.reset();
		
		return;
	}

	// If !force, then we have more work to do:

	for (physx::PxU32 i = mAssetCollisionSets.size(); i--;)
	{
		DestructibleAssetCollision* collisionSet = mAssetCollisionSets[i];
		if (collisionSet != NULL)
		{
			collisionSet->clearUnreferencedSets();
		}
	}
}

void DestructibleModuleCachedData::clearAssetCollisionSet(const DestructibleAsset& asset)
{
	for (physx::PxU32 i = mAssetCollisionSets.size(); i--;)
	{
		if (!mAssetCollisionSets[i] || !mAssetCollisionSets[i]->getAssetName() || !physx::string::stricmp(mAssetCollisionSets[i]->getAssetName(), asset.getName()))
		{
			PX_DELETE(mAssetCollisionSets[i]);
			mAssetCollisionSets.replaceWithLast(i);
		}
	}
}

physx::Array<NxConvexMesh*>* DestructibleModuleCachedData::getConvexMeshesForActor(const DestructibleActor& destructible)
{
	const DestructibleAsset* asset = destructible.getAsset();
	if (asset == NULL)
	{
		return NULL;
	}

	DestructibleAssetCollision* collisionSet = getAssetCollisionSet(*asset);
	if (collisionSet == NULL)
	{
		return NULL;
	}

	physx::Array<NxConvexMesh*>* convexMeshes = collisionSet->getConvexMeshesAtScale(destructible.getScale());

	const int scaleIndex = collisionSet->getScaleIndex(destructible.getScale(), kDefaultDestructibleAssetCollisionScaleTolerance);
	collisionSet->incReferenceCount(scaleIndex);

	return convexMeshes;
}

void DestructibleModuleCachedData::releaseReferencesToConvexMeshesForActor(const DestructibleActor& destructible)
{
	const DestructibleAsset* asset = destructible.getAsset();
	if (asset == NULL)
	{
		return;
	}

	DestructibleAssetCollision* collisionSet = getAssetCollisionSet(*asset);
	if (collisionSet == NULL)
	{
		return;
	}

	const int scaleIndex = collisionSet->getScaleIndex(destructible.getScale(), kDefaultDestructibleAssetCollisionScaleTolerance);
	collisionSet->decReferenceCount(scaleIndex);
}

physx::Array<NxConvexMesh*>* DestructibleModuleCachedData::getConvexMeshesForScale(const DestructibleAsset& asset, const physx::PxVec3& scale, bool incRef)
{
	DestructibleAssetCollision* collisionSet = getAssetCollisionSet(asset);
	if (collisionSet == NULL)
	{
		return NULL;
	}

	physx::Array<NxConvexMesh*>* convexMeshes =  collisionSet->getConvexMeshesAtScale(scale);

	// The NxDestructibleActor::cacheModuleData() method needs to avoid incrementing the ref count
	if (incRef)
	{
		const int scaleIndex = collisionSet->getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance);
		collisionSet->incReferenceCount(scaleIndex);
	}

	return convexMeshes;
}

DestructibleAssetCollision* DestructibleModuleCachedData::getAssetCollisionSet(const DestructibleAsset& asset)
{
	DestructibleAssetCollision* collisionSet = findAssetCollisionSet(asset.getName());

	if (collisionSet == NULL)
	{
		collisionSet = PX_NEW(DestructibleAssetCollision);
		mAssetCollisionSets.pushBack(collisionSet);
	}
	collisionSet->setDestructibleAssetToCook(const_cast<DestructibleAsset*>(&asset));

	return collisionSet;
}

DestructibleAssetCollision* DestructibleModuleCachedData::findAssetCollisionSet(const char* name)
{
	for (physx::PxU32 i = 0; i < mAssetCollisionSets.size(); ++i)
	{
		if (mAssetCollisionSets[i] && mAssetCollisionSets[i]->getAssetName() && !physx::string::stricmp(mAssetCollisionSets[i]->getAssetName(), name))
		{
			return mAssetCollisionSets[i];
		}
	}

	return NULL;
}

#endif

}
}
} // end namespace physx::apex

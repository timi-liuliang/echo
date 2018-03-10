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

#include "NiApexSDK.h"
#include "NxGroundEmitterAsset.h"
#include "NxGroundEmitterPreview.h"
#include "NxIofxAsset.h"
#include "GroundEmitterAsset.h"
#include "GroundEmitterActor.h"
#include "GroundEmitterAssetPreview.h"
//#include "ApexSharedSerialization.h"
#include "EmitterScene.h"
#include "NxParamUtils.h"

#if NX_SDK_VERSION_MAJOR == 2
#elif NX_SDK_VERSION_MAJOR == 3
typedef physx::PxFilterData NxGroupsMask;
#endif
namespace physx
{
namespace apex
{
namespace emitter
{

void GroundEmitterAsset::copyLodDesc(NxEmitterLodParamDesc& dst, const GroundEmitterAssetParametersNS::emitterLodParamDesc_Type& src)
{
	if (src.version != dst.current)
	{
		APEX_DEBUG_WARNING("NxEmitterLodParamDesc version mismatch");
	}
	dst.bias			= src.bias;
	dst.distanceWeight	= src.distanceWeight;
	dst.lifeWeight		= src.lifeWeight;
	dst.maxDistance		= src.maxDistance;
	dst.separationWeight = src.separationWeight;
	dst.speedWeight		= src.speedWeight;
}

void GroundEmitterAsset::copyLodDesc(GroundEmitterAssetParametersNS::emitterLodParamDesc_Type& dst, const NxEmitterLodParamDesc& src)
{
	dst.version			= src.current;
	dst.bias			= src.bias;
	dst.distanceWeight	= src.distanceWeight;
	dst.lifeWeight		= src.lifeWeight;
	dst.maxDistance		= src.maxDistance;
	dst.separationWeight = src.separationWeight;
	dst.speedWeight		= src.speedWeight;
}


void GroundEmitterAsset::postDeserialize(void* userData)
{
	PX_UNUSED(userData);

	/* Resolve the authored collision group name into the actual ID */
	if (mParams->raycastCollisionGroupMaskName != NULL &&
	        mParams->raycastCollisionGroupMaskName[0] != 0)
	{
		NiResourceProvider* nrp = mModule->mSdk->getInternalResourceProvider();
#if NX_SDK_VERSION_MAJOR == 2
		NxResID cgmns = mModule->mSdk->getCollisionGroupMaskNameSpace();
		NxResID cgresid = nrp->createResource(cgmns, mParams->raycastCollisionGroupMaskName);
		mRaycastCollisionGroups = (physx::PxU32)(size_t) nrp->getResource(cgresid);
#endif
		NxResID cgmns128 = mModule->mSdk->getCollisionGroup128NameSpace();
		NxResID cgmresid128 = nrp->createResource(cgmns128, mParams->raycastCollisionGroupMaskName);
		void* tmpCGM = nrp->getResource(cgmresid128);
		if (tmpCGM)
		{
			mRaycastCollisionGroupsMask = *(static_cast<NxGroupsMask*>(tmpCGM));
			mShouldUseGroupsMask = true;
		}
	}
	else
	{
		mRaycastCollisionGroups = 0xFFFFFFFF;
	}

	initializeAssetNameTable();
}

void GroundEmitterAsset::initializeAssetNameTable()
{
	/* initialize the iofx and ios asset name to resID tables */
	for (physx::PxU32 i = 0; i < (*mMaterialFactoryMaps).size(); i++)
	{
		mIofxAssetTracker.addAssetName((*mMaterialFactoryMaps)[i].iofxAssetName->name(), false);
		mIosAssetTracker.addAssetName((*mMaterialFactoryMaps)[i].iosAssetName->className(),
		                              (*mMaterialFactoryMaps)[i].iosAssetName->name());
	}
}

physx::PxU32 GroundEmitterAsset::forceLoadAssets()
{
	physx::PxU32 assetLoadedCount = 0;

	assetLoadedCount += mIofxAssetTracker.forceLoadAssets();
	assetLoadedCount += mIosAssetTracker.forceLoadAssets();

	return assetLoadedCount;
}

GroundEmitterAsset::GroundEmitterAsset(ModuleEmitter* m, NxResourceList& list, const char* name) :
	mIofxAssetTracker(m->mSdk, NX_IOFX_AUTHORING_TYPE_NAME),
	mIosAssetTracker(m->mSdk),
	mModule(m),
	mName(name),
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL),
	mShouldUseGroupsMask(false)
{
	using namespace GroundEmitterAssetParametersNS;

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = (GroundEmitterAssetParameters*)traits->createNxParameterized(GroundEmitterAssetParameters::staticClassName());

	PX_ASSERT(mParams);

	mMaterialFactoryMaps =
	    PX_NEW(NxParamArray<materialFactoryMapping_Type>)(mParams,
	            "materialFactoryMapList",
	            (NxParamDynamicArrayStruct*) & (mParams->materialFactoryMapList));
	list.add(*this);
}

GroundEmitterAsset::GroundEmitterAsset(ModuleEmitter* m,
                                       NxResourceList& list,
                                       NxParameterized::Interface* params,
                                       const char* name) :
	mIofxAssetTracker(m->mSdk, NX_IOFX_AUTHORING_TYPE_NAME),
	mIosAssetTracker(m->mSdk),
	mModule(m),
	mName(name),
	mParams((GroundEmitterAssetParameters*)params),
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL),
	mShouldUseGroupsMask(false)
{
	using namespace GroundEmitterAssetParametersNS;

	mMaterialFactoryMaps =
	    PX_NEW(NxParamArray<materialFactoryMapping_Type>)(mParams,
	            "materialFactoryMapList",
	            (NxParamDynamicArrayStruct*) & (mParams->materialFactoryMapList));
	PX_ASSERT(mMaterialFactoryMaps);

	postDeserialize();

	list.add(*this);
}


void GroundEmitterAsset::destroy()
{
	/* Assets that were forceloaded or loaded by actors will be automatically
	 * released by the ApexAssetTracker member destructors.
	 */

	delete mMaterialFactoryMaps;
	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}

	if (mDefaultActorParams)
	{
		mDefaultActorParams->destroy();
		mDefaultActorParams = 0;
	}

	if (mDefaultPreviewParams)
	{
		mDefaultPreviewParams->destroy();
		mDefaultPreviewParams = 0;
	}

	delete this;
}

GroundEmitterAsset::~GroundEmitterAsset()
{
}

NxParameterized::Interface* GroundEmitterAsset::getDefaultActorDesc()
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultActorParams)
	{
		const char* className = GroundEmitterActorParameters::staticClassName();
		NxParameterized::Interface* param = traits->createNxParameterized(className);
		mDefaultActorParams = static_cast<GroundEmitterActorParameters*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	// copy the parameters from the asset parameters
	mDefaultActorParams->densityRange.min	= mParams->densityRange.min;
	mDefaultActorParams->densityRange.max	= mParams->densityRange.max;
	mDefaultActorParams->radius				= mParams->radius;
	mDefaultActorParams->raycastHeight		= mParams->raycastHeight;
	mDefaultActorParams->spawnHeight		= mParams->spawnHeight;
	mDefaultActorParams->maxRaycastsPerFrame = mParams->maxRaycastsPerFrame;

	NxParameterized::setParamString(*mDefaultActorParams, "raycastCollisionGroupMaskName", mParams->raycastCollisionGroupMaskName);

	return mDefaultActorParams;
}

NxParameterized::Interface* GroundEmitterAsset::getDefaultAssetPreviewDesc()
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultPreviewParams)
	{
		const char* className = EmitterAssetPreviewParameters::staticClassName();
		NxParameterized::Interface* param = traits->createNxParameterized(className);
		mDefaultPreviewParams = static_cast<EmitterAssetPreviewParameters*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

NxApexActor* GroundEmitterAsset::createApexActor(const NxParameterized::Interface& parms, NxApexScene& apexScene)
{
	if (!isValidForActorCreation(parms, apexScene))
	{
		return NULL;
	}

	NxApexActor* ret = 0;

	const char* className = parms.className();
	if (strcmp(className, GroundEmitterActorParameters::staticClassName()) == 0)
	{
		NxGroundEmitterActorDesc desc;
		const GroundEmitterActorParameters* pDesc = static_cast<const GroundEmitterActorParameters*>(&parms);

		desc.densityRange.minimum		= pDesc->densityRange.min;
		desc.densityRange.maximum		= pDesc->densityRange.max;
		desc.radius					= pDesc->radius;
		desc.raycastHeight			= pDesc->raycastHeight;
		desc.spawnHeight			= pDesc->spawnHeight;
		desc.maxRaycastsPerFrame	= pDesc->maxRaycastsPerFrame;
		desc.attachRelativePosition	= pDesc->attachRelativePosition;
		desc.initialPosition		= pDesc->globalPose.t;
		desc.rotation				= pDesc->globalPose;
		desc.rotation.t = physx::PxVec3(0.0f);

		/* Resolve the authored collision group mask name into the actual ID */
#if NX_SDK_VERSION_MAJOR == 2
		if (pDesc->raycastCollisionGroupMaskName != NULL &&
		        pDesc->raycastCollisionGroupMaskName[0] != 0)
		{
			NiResourceProvider* nrp = mModule->mSdk->getInternalResourceProvider();
			NxResID cgmns = mModule->mSdk->getCollisionGroupMaskNameSpace();
			NxResID cgresid = nrp->createResource(cgmns, pDesc->raycastCollisionGroupMaskName);
			desc.raycastCollisionGroups = (PxU32)(size_t) nrp->getResource(cgresid);
		}
#endif

		ret = createActor(desc, apexScene);

		/* Resolve the authored collision group name into the actual ID */
		NxGroupsMask* raycastGroupsMask = 0;
		if (pDesc->raycastCollisionGroupMaskName != NULL &&
		        pDesc->raycastCollisionGroupMaskName[0] != 0)
		{
			NiResourceProvider* nrp = mModule->mSdk->getInternalResourceProvider();
			NxResID cgmns = mModule->mSdk->getCollisionGroup128NameSpace();
			NxResID cgresid = nrp->createResource(cgmns, pDesc->raycastCollisionGroupMaskName);
			raycastGroupsMask = static_cast<NxGroupsMask*>(nrp->getResource(cgresid));
		}
		else if (mShouldUseGroupsMask)
		{
			raycastGroupsMask = &mRaycastCollisionGroupsMask;
		}

		// check the NxGroupsMask specified in parms, set in actor if diff than default
		if (raycastGroupsMask && ret )
		{
			NxGroundEmitterActor* gea = static_cast<NxGroundEmitterActor*>(ret);
			gea->setRaycastCollisionGroupsMask(raycastGroupsMask);
		}
	}

	return ret;
}


NxApexAssetPreview* GroundEmitterAsset::createApexAssetPreview(const NxParameterized::Interface& parms, NxApexAssetPreviewScene* previewScene)
{
	NxApexAssetPreview* ret = 0;

	const char* className = parms.className();
	if (strcmp(className, EmitterAssetPreviewParameters::staticClassName()) == 0)
	{
		NxGroundEmitterPreviewDesc desc;
		const EmitterAssetPreviewParameters* pDesc = static_cast<const EmitterAssetPreviewParameters*>(&parms);

		desc.mPose	= pDesc->pose;
		desc.mScale = pDesc->scale;

		ret = createEmitterPreview(desc, previewScene);
	}

	return ret;
}


NxGroundEmitterActor* GroundEmitterAsset::createActor(const NxGroundEmitterActorDesc& desc, NxApexScene& scene)
{
	if (!desc.isValid())
	{
		return NULL;
	}
	EmitterScene* es = mModule->getEmitterScene(scene);
	GroundEmitterActor* ret = PX_NEW(GroundEmitterActor)(desc, *this, mEmitterActors, *es);

	if (!ret->isValid())
	{
		ret->destroy();
		return NULL;
	}

	return ret;
}

void GroundEmitterAsset::releaseActor(NxGroundEmitterActor& nxactor)
{
	GroundEmitterActor* actor = DYNAMIC_CAST(GroundEmitterActor*)(&nxactor);
	actor->destroy();
}

NxGroundEmitterPreview* GroundEmitterAsset::createEmitterPreview(const NxGroundEmitterPreviewDesc& desc, NxApexAssetPreviewScene* previewScene)
{
	if (!desc.isValid())
	{
		return NULL;
	}

	GroundEmitterAssetPreview* p = PX_NEW(GroundEmitterAssetPreview)(desc, *this, NxGetApexSDK(), previewScene);
	if (p && !p->isValid())
	{
		p->destroy();
		p = NULL;
	}
	return p;
}

void GroundEmitterAsset::releaseEmitterPreview(NxGroundEmitterPreview& nxpreview)
{
	GroundEmitterAssetPreview* preview = DYNAMIC_CAST(GroundEmitterAssetPreview*)(&nxpreview);
	preview->destroy();
}


void GroundEmitterAsset::release()
{
	mModule->mSdk->releaseAsset(*this);
}

#ifndef WITHOUT_APEX_AUTHORING
void GroundEmitterAssetAuthoring::release()
{
	mModule->mSdk->releaseAssetAuthoring(*this);
}

void GroundEmitterAssetAuthoring::addMeshForGroundMaterial(const NxMaterialFactoryMappingDesc& desc)
{
	NxParameterized::Handle arrayHandle(*mParams), indexHandle(*mParams), childHandle(*mParams);
	NxParameterized::Interface* refPtr;
	// resize map array
	physx::PxU32 newIdx = (*mMaterialFactoryMaps).size();
	(*mMaterialFactoryMaps).resize(newIdx + 1);

	// copy new desc in place
	// floats
	(*mMaterialFactoryMaps)[ newIdx ].weight = desc.weight;
	(*mMaterialFactoryMaps)[ newIdx ].maxSlopeAngle = desc.maxSlopeAngle;

	// lod params
	copyLodDesc((*mMaterialFactoryMaps)[ newIdx ].lodParamDesc, mCurLodParamDesc);

	// strings
	// get a handle for the material factory map for this index into the dynamic array
	mParams->getParameterHandle("materialFactoryMapList", arrayHandle);
	arrayHandle.getChildHandle((physx::PxI32)newIdx, indexHandle);

	indexHandle.getChildHandle(mParams, "iofxAssetName", childHandle);
	mParams->initParamRef(childHandle, NULL, true);
	mParams->getParamRef(childHandle, refPtr);
	PX_ASSERT(refPtr);
	if (refPtr)
	{
		refPtr->setName(desc.instancedObjectEffectsAssetName);
	}

	indexHandle.getChildHandle(mParams, "iosAssetName", childHandle);
	mParams->initParamRef(childHandle, desc.instancedObjectSimulationTypeName, true);
	mParams->getParamRef(childHandle, refPtr);
	PX_ASSERT(refPtr);
	if (refPtr)
	{
		refPtr->setName(desc.instancedObjectSimulationAssetName);
	}

	//setParamString( childHandle, desc.instancedObjectSimulationAssetName );

	indexHandle.getChildHandle(mParams, "physMatName", childHandle);
	mParams->setParamString(childHandle, desc.physicalMaterialName);
}

GroundEmitterAssetAuthoring::~GroundEmitterAssetAuthoring()
{
}
#endif

}
}
} // namespace physx::apex

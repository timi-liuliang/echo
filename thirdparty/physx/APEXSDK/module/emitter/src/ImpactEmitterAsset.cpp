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
#include "NxIofxAsset.h"

#include "ImpactEmitterAsset.h"
#include "ImpactEmitterActor.h"
#include "PxMemoryBuffer.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "ExplosionEnvParameters.h"
#include "ExplosionAsset.h"
#include "NxExplosionAsset.h"
#endif

#include "NxParamUtils.h"
#include "PsShare.h"

namespace physx
{
namespace apex
{
namespace emitter
{



void ImpactEmitterAsset::initializeAssetNameTable()
{
	/* initialize the exlosion, iofx, and ios asset names to resID tables */
	NxParameterized::Handle eventSetHandle(*mParams);
	int numSets;

	mParams->getParameterHandle("eventSetList", eventSetHandle);
	PX_ASSERT(eventSetHandle.isValid());

	mParams->getArraySize(eventSetHandle, numSets);
	for (int i = 0; i < numSets; i++)
	{
		NxParameterized::Handle ih(*mParams);
		NxParameterized::Interface* eventPtr = 0;

		eventSetHandle.getChildHandle(i, ih);
		PX_ASSERT(ih.isValid());

		mParams->getParamRef(ih, eventPtr);
		PX_ASSERT(eventPtr);

		ApexSimpleString tmpClassName(eventPtr->className());


		if (tmpClassName == "ImpactExplosionEvent")
		{
#if NX_SDK_VERSION_MAJOR == 2
			ImpactExplosionEvent* paramPtr = (ImpactExplosionEvent*)eventPtr;
			mExplosionAssetTracker.addAssetName(paramPtr->parameters().explosionAssetName->name(), false);
#elif NX_SDK_VERSION_MAJOR == 3
			APEX_DEBUG_WARNING("Invalid asset. ImpactExplosionEvent is not supported under PhysX 3.");
			PX_ALWAYS_ASSERT();
#endif
		}
		else if (tmpClassName == "ImpactObjectEvent")
		{
			ImpactObjectEvent* paramPtr = (ImpactObjectEvent*)eventPtr;

			mIofxAssetTracker.addAssetName(paramPtr->parameters().iofxAssetName->name(), false);
			mIosAssetTracker.addAssetName(paramPtr->parameters().iosAssetName->className(),
			                              paramPtr->parameters().iosAssetName->name());
		}
	}
}

void ImpactEmitterAsset::preSerialize(void* userData)
{
	PX_UNUSED(userData);
}

void ImpactEmitterAsset::postDeserialize(void* userData)
{
	PX_UNUSED(userData);
	initializeAssetNameTable();
	buildEventNameIndexMap();
}

void ImpactEmitterAsset::buildEventNameIndexMap()
{
	// destroy old mappings
	for (physx::PxU32 j = 0; j < mEventNameIndexMaps.size(); j++)
	{
		if (mEventNameIndexMaps[j])
		{
			delete mEventNameIndexMaps[j];
		}
	}

	// loop through all event sets, build a mapping for each new name found
	// and store all indices for that event set
	NxParamArray<NxParameterized::Interface*> assetEventSets(mParams,
	        "eventSetList",
	        (NxParamDynamicArrayStruct*) & (mParams->eventSetList));

	for (physx::PxU32 i = 0; i < assetEventSets.size(); i++)
	{
		NxParameterized::Handle hEventSetName(*assetEventSets[i]);
		const char* paramEventSetName = 0;

		NxParameterized::getParamString(*(assetEventSets[i]), "eventSetName", paramEventSetName);
		if (!paramEventSetName)
		{
			APEX_DEBUG_WARNING("Invalid eventSetName from Impact Emitter event sets");
			continue;
		}

		/* first see if the name is already here */
		bool foundEventSet = false;
		for (physx::PxU32 j = 0; j < mEventNameIndexMaps.size(); j++)
		{
			if (mEventNameIndexMaps[j]->eventSetName == paramEventSetName)
			{
				// add index to list
				mEventNameIndexMaps[j]->eventIndices.pushBack((physx::PxU16)i);
				foundEventSet = true;
			}
		}

		if (!foundEventSet)
		{
			/* now add it to the list */
			EventNameIndexMap* newMap = PX_NEW(EventNameIndexMap)();
			newMap->eventSetName = paramEventSetName;
			newMap->eventIndices.pushBack((physx::PxU16)i);
			mEventNameIndexMaps.pushBack(newMap);
		}
	}
}

ImpactEmitterAsset::ImpactEmitterAsset(ModuleEmitter* m, NxResourceList& list, const char* name) :
	mModule(m),
	mName(name),
	mIofxAssetTracker(m->mSdk, NX_IOFX_AUTHORING_TYPE_NAME),
	mIosAssetTracker(m->mSdk),
#if NX_SDK_VERSION_MAJOR == 2
	mExplosionAssetTracker(m->mSdk, NX_EXPLOSION_AUTHORING_TYPE_NAME),
#endif
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL)
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = (ImpactEmitterAssetParameters*)traits->createNxParameterized(ImpactEmitterAssetParameters::staticClassName());

	PX_ASSERT(mParams);

	mParams->setSerializationCallback(this);
	list.add(*this);
}

ImpactEmitterAsset::ImpactEmitterAsset(ModuleEmitter* m,
                                       NxResourceList& list,
                                       NxParameterized::Interface* params,
                                       const char* name) :
	mModule(m),
	mName(name),
	mIofxAssetTracker(m->mSdk, NX_IOFX_AUTHORING_TYPE_NAME),
	mIosAssetTracker(m->mSdk),
#if NX_SDK_VERSION_MAJOR == 2
	mExplosionAssetTracker(m->mSdk, NX_EXPLOSION_AUTHORING_TYPE_NAME),
#endif
	mParams((ImpactEmitterAssetParameters*)params),
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL)
{
	// this may no longer make any sense
	mParams->setSerializationCallback(this);

	// call this now to "initialize" the asset
	postDeserialize();

	list.add(*this);
}

void ImpactEmitterAsset::destroy()
{
	/* Assets that were forceloaded or loaded by actors will be automatically
	 * released by the ApexAssetTracker member destructors.
	 */

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

ImpactEmitterAsset::~ImpactEmitterAsset()
{
	// cleanup the eventset name to indices map
	for (physx::PxU32 j = 0; j < mEventNameIndexMaps.size(); j++)
	{
		if (mEventNameIndexMaps[j])
		{
			delete mEventNameIndexMaps[j];
		}
	}
}

NxParameterized::Interface* ImpactEmitterAsset::getDefaultActorDesc()
{
	NX_WRITE_ZONE();
	NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
	PX_UNUSED(error);

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultActorParams)
	{
		const char* className = ImpactEmitterActorParameters::staticClassName();
		NxParameterized::Interface* param = traits->createNxParameterized(className);
		mDefaultActorParams = static_cast<ImpactEmitterActorParameters*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}
	NxParameterized::Handle hDest(*mDefaultActorParams);
	
#if NX_SDK_VERSION_MAJOR == 2
	NxParameterized::Interface* explParams = 0;
	error = mDefaultActorParams->getParameterHandle("explosionEnv", hDest);
	PX_ASSERT(NxParameterized::ERROR_NONE == error);

	mDefaultActorParams->initParamRef(hDest, hDest.parameterDefinition()->refVariantVal(0), true);
	mDefaultActorParams->getParamRef(hDest, explParams);

	PX_ASSERT(explParams);
	if (explParams)
	{
		explParams->initDefaults();
	}
#endif

	return mDefaultActorParams;
}

NxParameterized::Interface* ImpactEmitterAsset::getDefaultAssetPreviewDesc()
{
	NX_WRITE_ZONE();
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

NxApexActor* ImpactEmitterAsset::createApexActor(const NxParameterized::Interface& parms, NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	if (!isValidForActorCreation(parms, apexScene))
	{
		return NULL;
	}

	NxApexActor* ret = 0;

	const char* className = parms.className();
	if (strcmp(className, ImpactEmitterActorParameters::staticClassName()) == 0)
	{
		EmitterScene* es = mModule->getEmitterScene(apexScene);
		ImpactEmitterActor* actor = PX_NEW(ImpactEmitterActor)(parms, *this, mEmitterActors, *es);
		if (!actor->isValid())
		{
			actor->destroy();
			return NULL;
		}

		ret = actor;
	}
	else
	{
		APEX_INVALID_OPERATION("%s is not a valid descriptor class, expecting %s", className, ImpactEmitterActorParameters::staticClassName());
	}

	return ret;
}

NxApexAssetPreview* ImpactEmitterAsset::createApexAssetPreview(const NxParameterized::Interface& /*params*/, NxApexAssetPreviewScene* /*previewScene*/)
{
	NX_WRITE_ZONE();
	APEX_INVALID_OPERATION("Not yet implemented!");
	return NULL;
}

physx::PxU32 ImpactEmitterAsset::querySetID(const char* setName)
{
	NX_WRITE_ZONE();
	for (physx::PxU32 i = 0; i < mEventNameIndexMaps.size(); ++i)
	{
		if (mEventNameIndexMaps[i]->eventSetName == setName)
		{
			return i;
		}
	}
	return 0;   // return the default event set if no match is found
}

void ImpactEmitterAsset::getSetNames(const char** outSetNames, physx::PxU32& inOutNameCount) const
{
	NX_READ_ZONE();
	if (mEventNameIndexMaps.size() > inOutNameCount)
	{
		inOutNameCount = 0xffffffff;
		return;
	}

	inOutNameCount = mEventNameIndexMaps.size();
	for (physx::PxU32 i = 0; i < mEventNameIndexMaps.size(); ++i)
	{
		outSetNames[i] = mEventNameIndexMaps[i]->eventSetName.c_str();
	}
}

physx::PxU32 ImpactEmitterAsset::forceLoadAssets()
{
	NX_WRITE_ZONE();
	physx::PxU32 assetLoadedCount = 0;

	assetLoadedCount += mIofxAssetTracker.forceLoadAssets();
	assetLoadedCount += mIosAssetTracker.forceLoadAssets();
#if NX_SDK_VERSION_MAJOR == 2
	assetLoadedCount += mExplosionAssetTracker.forceLoadAssets();
#endif

	return assetLoadedCount;
}


void ImpactEmitterAsset::release()
{
	mModule->mSdk->releaseAsset(*this);
}

#ifndef WITHOUT_APEX_AUTHORING
void ImpactEmitterAssetAuthoring::release()
{
	mModule->mSdk->releaseAssetAuthoring(*this);
}

#endif

}
}
} // namespace physx::apex

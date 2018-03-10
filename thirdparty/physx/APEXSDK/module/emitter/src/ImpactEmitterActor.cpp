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

#include "NxImpactEmitterAsset.h"
#include "NxImpactEmitterActor.h"
#include "NiApexScene.h"
#include "NiApexRenderDebug.h"
#include "NiInstancedObjectSimulation.h"
#include "ImpactEmitterAsset.h"
#include "ImpactEmitterActor.h"
#include "EmitterScene.h"
#include "NxIofxAsset.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxExplosionAsset.h"
#include "ExplosionAsset.h"
#include "ExplosionEnvParameters.h"
#include "NxExplosionActor.h"
#include "ImpactExplosionEvent.h"
#endif

#include "NxParameterized.h"
#include "ImpactObjectEvent.h"
#include "NxParamArray.h"
#include "NxParamUtils.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

////////////////////////////////////////////////////////////////////////////////

#define PI (3.1415926535897932384626433832795f)

////////////////////////////////////////////////////////////////////////////////
namespace physx
{
namespace apex
{
namespace emitter
{

bool ImpactEmitterActor::QueuedImpactEvent::trigger(physx::PxF32 time)
{
	if (time > triggerTime)
	{
		eventDef->trigger(triggerParams);
		return true;
	}
	else
	{
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
class ParticleEventTask : public physx::PxTask, public physx::UserAllocated
{
public:
	ParticleEventTask(ImpactEmitterParticleEvent* owner) : mOwner(owner) {}

	const char* getName() const
	{
		return "ImpactEmitterActor::ParticleEventTask";
	}
	void run()
	{
		mOwner->run();
	}

protected:
	ImpactEmitterParticleEvent* mOwner;
};


void ImpactEmitterParticleEvent::trigger(const ImpactEventTriggerParams& params)
{
	if (mEventTask != NULL)
	{
		mTriggerQueue.pushBack(params);
	}
}

void ImpactEmitterParticleEvent::run()
{
	for (TriggerQueue::ConstIterator it = mTriggerQueue.begin(); it != mTriggerQueue.end(); ++it)
	{
		const ImpactEventTriggerParams& params = *it;
		trigger(params.hitPos, params.hitDir, params.hitNorm);
	}
	mTriggerQueue.clear();
}

void ImpactEmitterParticleEvent::submitTasks(physx::PxTaskManager* tm)
{
	if (mEventTask != NULL && mTriggerQueue.size() > 0)
	{
		tm->submitUnnamedTask(*mEventTask);
	}
}

void ImpactEmitterParticleEvent::setTaskDependencies(physx::PxTask*)
{
	if (mEventTask != NULL && mTriggerQueue.size() > 0)
	{
		//mEventTask->startAfter( tickTask->getTaskID() );
		mEventTask->finishBefore(mParticleInjector->getCompletionTaskID());
	}
}

ImpactEmitterParticleEvent::ImpactEmitterParticleEvent(NxParameterized::Interface* eventParamPtr, ImpactEmitterAsset& asset, ImpactEmitterActor* emitterActor): 
	ImpactEmitterBaseEvent(emitterActor),
	mParticleInjector(NULL)

{
	PX_ASSERT(eventParamPtr);
	PX_ASSERT(!strcmp(eventParamPtr->className(), ImpactObjectEvent::staticClassName()));

	mEventTask = NULL;

	ImpactObjectEvent* eventPtr = (ImpactObjectEvent*)eventParamPtr;
	NxParameterized::Handle hEnum(*eventPtr);

	mAxis = AXIS_REFLECTION;
	eventPtr->getParameterHandle("impactAxis", hEnum);
	PX_ASSERT(hEnum.isValid());

	// this assumes that the enums line up correctly, they do
	physx::PxI32 axisInt = hEnum.parameterDefinition()->enumValIndex(eventPtr->impactAxis);
	if (-1 != axisInt)
	{
		mAxis = (eAxisType)axisInt;
	}

	mMinAngle = physx::degToRad(eventPtr->parameters().angleRange.min);
	mMaxAngle = physx::degToRad(eventPtr->parameters().angleRange.max);
	mMinSpeed = eventPtr->parameters().speedRange.min;
	mMaxSpeed = eventPtr->parameters().speedRange.max;
	mMinLife = eventPtr->parameters().lifeRange.min;
	mMaxLife = eventPtr->parameters().lifeRange.max;
	mParticleSpawnCount = eventPtr->parameters().particleSpawnCount;


	const char* iofxAssetName = eventPtr->parameters().iofxAssetName->name();
	NxIofxAsset* iofxAsset = static_cast<NxIofxAsset*>(asset.mIofxAssetTracker.getAssetFromName(iofxAssetName));
	NxIosAsset* iosAsset = asset.mIosAssetTracker.getIosAssetFromName(eventPtr->parameters().iosAssetName->className(),
	                       eventPtr->parameters().iosAssetName->name());
	if (!iosAsset || !iofxAsset)
	{
		return;
	}

	NxApexActor* nxActor = iosAsset->createIosActor(*(mOwner->getApexScene()), iofxAsset);
	NiInstancedObjectSimulation* iosActor = NULL;
	if (nxActor)
	{
		ApexActor* aa = NiGetApexSDK()->getApexActor(nxActor);
		if (aa)
		{
			iosActor = DYNAMIC_CAST(NiInstancedObjectSimulation*)(aa);
		}
	}
	if (!iosActor)
	{
		return;
	}

	mParticleInjector = iosActor->allocateInjector(iofxAsset);
	if (mParticleInjector)
	{
		mParticleInjector->setLODWeights(eventPtr->parameters().lodParamDesc.maxDistance,
		                                 eventPtr->parameters().lodParamDesc.distanceWeight,
		                                 eventPtr->parameters().lodParamDesc.speedWeight,
		                                 eventPtr->parameters().lodParamDesc.lifeWeight,
		                                 eventPtr->parameters().lodParamDesc.separationWeight,
		                                 eventPtr->parameters().lodParamDesc.bias);

		mParticleInjector->addSelfToContext(*mOwner);
		mEventTask = PX_NEW(ParticleEventTask)(this);
		PX_ASSERT(mEventTask);

		mOwner->mActiveParticleInjectors.pushBack(mParticleInjector);
	}
	else
	{
		return;
	}

	mValid = true;
}

ImpactEmitterParticleEvent::~ImpactEmitterParticleEvent()
{
	if (mEventTask)
	{
		delete mEventTask;
	}
	mEventTask = NULL;

	if (mParticleInjector)
	{
		mOwner->mActiveParticleInjectors.findAndReplaceWithLast(mParticleInjector);

		mParticleInjector->release();
	}
	mParticleInjector = NULL;
}

void ImpactEmitterParticleEvent::trigger(const physx::PxVec3& hitPos, const physx::PxVec3& hitDir, const physx::PxVec3& hitNorm)
{
	if (!mParticleInjector)
	{
		return;
	}

	physx::PxVec3 eventBasis[3];
	if (mAxis == AXIS_INCIDENT)
	{
		eventBasis[0] = -hitDir;
	}
	else if (mAxis == AXIS_NORMAL)
	{
		eventBasis[0] = hitNorm;
	}
	else if (mAxis == AXIS_REFLECTION)
	{
		// this is also found in NxRay::ComputeReflexionVector()
		eventBasis[0] = hitDir - (2 * hitDir.dot(hitNorm) * hitNorm);
	}
	else
	{
		// Error
		return;
	}
	eventBasis[0].normalize();
	eventBasis[1] = physx::PxVec3(eventBasis[0].y, eventBasis[0].z, eventBasis[0].x).cross(eventBasis[0]);
	eventBasis[1].normalize();
	eventBasis[2] = eventBasis[1].cross(eventBasis[0]);
	eventBasis[2].normalize();

	IosNewObject* particles = (IosNewObject*) PX_ALLOC(sizeof(IosNewObject) * mParticleSpawnCount, PX_DEBUG_EXP("IosNewObject"));
	IosNewObject* currParticle = particles;
	for (int i = 0; i < (int)mParticleSpawnCount; ++i, ++currParticle)
	{
		initParticle(hitPos, eventBasis, currParticle->initialPosition, currParticle->initialVelocity, currParticle->lifetime);

		currParticle->iofxActorID	= NiIofxActorID(0);
		currParticle->lodBenefit	= 0.0f;
		currParticle->userData		= 0;
	}
	mParticleInjector->createObjects(mParticleSpawnCount, particles);

	PX_FREE(particles);
}

void ImpactEmitterParticleEvent::initParticle(const physx::PxVec3& pos, const physx::PxVec3 basis[], physx::PxVec3& outPos, physx::PxVec3& outVel, physx::PxF32& outLife)
{
	physx::PxF32 theta = mOwner->mRand.getScaled(0.0f, 2.0f * PI);
	physx::PxF32 phi = mOwner->mRand.getScaled(mMinAngle, mMaxAngle);
	physx::PxF32 speed = mOwner->mRand.getScaled(mMinSpeed, mMaxSpeed);

	physx::PxVec3 vel = speed * (physx::PxCos(phi) * basis[0] + physx::PxSin(phi) * physx::PxCos(theta) * basis[1] + physx::PxSin(phi) * physx::PxSin(theta) * basis[2]);

	outPos = pos;
	outVel = vel;
	outLife = mOwner->mRand.getScaled(mMinLife, mMaxLife);
}

void ImpactEmitterParticleEvent::setPreferredRenderVolume(physx::apex::NxApexRenderVolume* vol)
{
	if (mParticleInjector)
	{
		mParticleInjector->setPreferredRenderVolume(vol);
	}
}


////////////////////////////////////////////////////////////////////////////////
#if NX_SDK_VERSION_MAJOR == 2
ImpactEmitterExplosionEvent::ImpactEmitterExplosionEvent(NxParameterized::Interface* eventParamPtr, ImpactEmitterAsset& asset, ImpactEmitterActor* emitterActor) : ImpactEmitterBaseEvent(emitterActor)
{
	PX_ASSERT(eventParamPtr);
	PX_ASSERT(!strcmp(eventParamPtr->className(), ImpactExplosionEvent::staticClassName()));

	ImpactExplosionEvent* eventPtr = (ImpactExplosionEvent*)eventParamPtr;

	mDuration = eventPtr->parameters().duration;
	void* tmp = asset.mExplosionAssetTracker.getAssetFromName(eventPtr->parameters().explosionAssetName->name());
	mExplosionAsset = static_cast<NxExplosionAsset*>(tmp);

	if (!tmp)
	{
		return;
	}

	mValid = true;
}

ImpactEmitterExplosionEvent::~ImpactEmitterExplosionEvent()
{
}

void ImpactEmitterExplosionEvent::trigger(const ImpactEventTriggerParams& params)
{
	if (mExplosionAsset)
	{
		NxParameterized::Interface* desc = mExplosionAsset->getDefaultActorDesc();
		PX_ASSERT(desc);

		physx::PxMat44 pose44 = physx::PxMat44::createIdentity();
		pose44.setPosition(params.hitPos);

		NxParameterized::setParamMat44(*desc, "initialPose", pose44);

		mOwner->createExplosion(mExplosionAsset, *desc, mDuration);
	}
}
#endif
////////////////////////////////////////////////////////////////////////////////

ImpactEmitterEventSet::~ImpactEmitterEventSet()
{
	for (physx::PxU32 e = 0; e < entries.size(); ++e)
	{
		delete entries[e].evnt;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool ImpactEmitterEventSet::AddEvent(NxParameterized::Interface* eventParamPtr, ImpactEmitterAsset& asset, ImpactEmitterActor* emitterActor)
{
	ImpactEmitterBaseEvent* e = NULL;
	physx::PxF32 delay = 0;

	PX_ASSERT(eventParamPtr);

#if NX_SDK_VERSION_MAJOR == 2
	if (!strcmp(eventParamPtr->className(), ImpactExplosionEvent::staticClassName()))
	{
		e = PX_NEW(ImpactEmitterExplosionEvent)(eventParamPtr, asset, emitterActor);
		if (!e->isValid())
		{
			PX_DELETE(e);
			return false;
		}
		ImpactExplosionEvent* explEvent = (ImpactExplosionEvent*)eventParamPtr;
		delay = explEvent->parameters().delay;
	}
	else if (!strcmp(eventParamPtr->className(), ImpactObjectEvent::staticClassName()))
#elif NX_SDK_VERSION_MAJOR == 3
	if(!strcmp(eventParamPtr->className(), ImpactObjectEvent::staticClassName()))
#endif
	{
		e = PX_NEW(ImpactEmitterParticleEvent)(eventParamPtr, asset, emitterActor);
		if (!e->isValid())
		{
			PX_DELETE(e);
			return false;
		}
		ImpactObjectEvent* objEvent = (ImpactObjectEvent*)eventParamPtr;
		delay = objEvent->parameters().delay;
	}

	if (e != NULL)
	{
		entries.pushBack(EventSetEntry(delay, e));
	}
	else
	{
		return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterTickTask : public physx::PxTask, public physx::UserAllocated
{
public:
	ImpactEmitterTickTask(ImpactEmitterActor* actor) : mActor(actor) {}

	const char* getName() const
	{
		return "ImpactEmitterActor::TickTask";
	}
	void run()
	{
		mActor->tick();
	}

protected:
	ImpactEmitterActor* mActor;
};

#if NX_SDK_VERSION_MAJOR == 2
void ImpactEmitterActor::ActiveExplosion::releaseActor()
{
	if (expActor)
	{
		expActor->release();
		expActor = NULL;
	}
}

void ImpactEmitterActor::ActiveExplosion::updateScale(physx::PxF32 time)
{
	physx::PxF32 diff = (endTime - time) / duration;
	physx::PxF32 scale = 2.0f / (2.0f - diff) - 1.0f;
	expActor->setScale(scale);
}
#endif

ImpactEmitterActor::ImpactEmitterActor(const NxParameterized::Interface& desc, ImpactEmitterAsset& asset, NxResourceList& list, EmitterScene& scene) :
	mTime(0.0f),
	mAsset(&asset),
	mScene(&scene),
	mTickTask(NULL)
{
	mRand.setSeed(scene.mApexScene->getSeed());

	/* Read default values from descriptor or authored asset */
	if (strcmp(desc.className(), ImpactEmitterActorParameters::staticClassName()) == 0)
	{
#if NX_SDK_VERSION_MAJOR == 2
		NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
		PX_ASSERT(traits);

		mExplEnvParams = traits->createNxParameterized(explosion::ExplosionEnvParameters::staticClassName());
		NxParameterized::Interface* descExplEnvParams = 0;
		NxParameterized::Interface* tmpDesc = const_cast<NxParameterized::Interface*>(&desc);
		NxParameterized::getParamRef(*tmpDesc, "explosionEnv", descExplEnvParams);
		PX_ASSERT(descExplEnvParams);

		mExplEnvParams->copy(*descExplEnvParams);
#endif
	}
	else
	{
		APEX_INVALID_OPERATION("%s is not a valid descriptor class", desc.className());
	}


	// Insert self into data structures
	list.add(*this);            // Add self to asset's list of actors
	addSelfToContext(*scene.mApexScene->getApexContext());    // Add self to ApexScene
	addSelfToContext(scene);    // Add self to EmitterScene's list of actors

	// Initialize data members
	mTime = 0.f;

	NxParamArray<NxParameterized::Interface*> assetEventSets(asset.mParams,
	        "eventSetList",
	        (NxParamDynamicArrayStruct*) & (asset.mParams->eventSetList));
	//mEventSets.resize( asset.mEventSets.size() );
	mEventSets.resize(asset.mEventNameIndexMaps.size());

	for (physx::PxU32 s = 0; s < asset.mEventNameIndexMaps.size(); ++s)
	{
		for (physx::PxU32 e = 0; e < asset.mEventNameIndexMaps[s]->eventIndices.size(); ++e)
		{
			physx::PxU32 t = asset.mEventNameIndexMaps[s]->eventIndices[e];

			if (!mEventSets[s].AddEvent(assetEventSets[t], asset, this))
			{
				return;
			}
		}
	}

	mTickTask = PX_NEW(ImpactEmitterTickTask)(this);
	PX_ASSERT(mTickTask);

	mValid = true;
}

ImpactEmitterActor::~ImpactEmitterActor()
{
	delete mTickTask;
#if NX_SDK_VERSION_MAJOR == 2
	for (int e = 0; e < (int)mActiveExplosions.size(); ++e)
	{
		mActiveExplosions[e].releaseActor();
	}
#endif
}

void ImpactEmitterActor::setPreferredRenderVolume(NxApexRenderVolume* vol)
{
	NX_WRITE_ZONE();
	for (PxU32 i = 0 ; i < mEventSets.size() ; i++)
	{
		for (PxU32 e = 0 ; e < mEventSets[i].entries.size() ; e++)
		{
			mEventSets[i].entries[e].evnt->setPreferredRenderVolume(vol);
		}
	}
}

void ImpactEmitterActor::submitTasks()
{
	physx::PxF32 dt = mScene->mApexScene->getElapsedTime();
	mTime += dt;

	// Check queued events, triggering and removing those that have reached their trigger time
	for (physx::PxU32 e = 0; e < mPendingEvents.size();)
	{
		if (mPendingEvents[e].trigger(mTime))
		{
			mPendingEvents.replaceWithLast(e);
		}
		else
		{
			e++;
		}
	}

	//
	physx::PxTaskManager* tm = mScene->mApexScene->getTaskManager();

	tm->submitUnnamedTask(*mTickTask);

	// submitTasks for events
	for (physx::PxU32 i = 0 ; i < mEventSets.size() ; i++)
	{
		ImpactEmitterEventSet& set = mEventSets[i];
		for (physx::PxU32 s = 0 ; s < set.entries.size() ; s++)
		{
			ImpactEmitterEventSet::EventSetEntry& entry = set.entries[s];
			entry.evnt->submitTasks(tm);
		}
	}
}

void ImpactEmitterActor::setTaskDependencies()
{
	physx::PxTaskManager* tm = mScene->mApexScene->getTaskManager();

	physx::PxTaskID completionTaskID = tm->getNamedTask(AST_PHYSX_SIMULATE);
	mTickTask->finishBefore(completionTaskID);

	// setTaskDependencies for events
	for (physx::PxU32 i = 0 ; i < mEventSets.size() ; i++)
	{
		ImpactEmitterEventSet& set = mEventSets[i];
		for (physx::PxU32 s = 0 ; s < set.entries.size() ; s++)
		{
			ImpactEmitterEventSet::EventSetEntry& entry = set.entries[s];
			entry.evnt->setTaskDependencies(mTickTask);
		}
	}
}

void ImpactEmitterActor::fetchResults()
{
}


NxApexAsset*            ImpactEmitterActor::getOwner() const
{
	NX_READ_ZONE();
	return (NxApexAsset*) mAsset;
}

NxImpactEmitterAsset*   ImpactEmitterActor::getEmitterAsset() const
{
	NX_READ_ZONE();
	return mAsset;
}

NiApexScene* ImpactEmitterActor::getApexScene() const
{
	return mScene->mApexScene;
}

void ImpactEmitterActor::getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const
{
	NX_READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

physx::PxF32 ImpactEmitterActor::getActivePhysicalLod() const
{
	NX_READ_ZONE();
	APEX_INVALID_OPERATION("NxImpactEmitterActor does not support this operation");
	return -1.0f;
}

void ImpactEmitterActor::forcePhysicalLod(physx::PxF32 lod)
{
	NX_WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

void ImpactEmitterActor::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	destroy();
}


void ImpactEmitterActor::destroy()
{
	/* Order is important here, pay attention */
#if NX_SDK_VERSION_MAJOR == 2
	if (mExplEnvParams)
	{
		mExplEnvParams->destroy();
		mExplEnvParams = NULL;
	}
#endif

	// Remove ourselves from all contexts, so they don't get stuck trying to release us
	ApexActor::destroy();

	ApexContext::removeAllActors();

	delete this;
}

void ImpactEmitterActor::removeActorAtIndex(physx::PxU32 index)
{
	// A particle injector has been removed
	for (physx::PxU32 i = 0 ; i < mEventSets.size() ; i++)
	{
		ImpactEmitterEventSet& set = mEventSets[i];
		for (physx::PxU32 s = 0 ; s < set.entries.size() ; s++)
		{
			ImpactEmitterEventSet::EventSetEntry& entry = set.entries[s];
			entry.evnt->removeActorReference(mActorArray[ index ]);
		}
	}

	ApexContext::removeActorAtIndex(index);

	release();
}

physx::PxF32 ImpactEmitterActor::getBenefit()
{
	return LODCollection<NiIosInjector>::computeSumBenefit(mActiveParticleInjectors.begin(), mActiveParticleInjectors.end());
}

physx::PxF32 ImpactEmitterActor::setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit)
{
	PX_UNUSED(maxRemaining);
	PX_UNUSED(relativeBenefit);

	return LODCollection<NiIosInjector>::distributeResource(mActiveParticleInjectors.begin(), mActiveParticleInjectors.end(), LODNode::getCachedBenefit(), relativeBenefit, suggested);
}

void ImpactEmitterActor::tick()
{
#if NX_SDK_VERSION_MAJOR == 2
	// Update active explosions, expiring those that have passed their durations
	for (physx::PxU32 e = 0; e < mActiveExplosions.size();)
	{
		if (mActiveExplosions[e].checkExpire(mTime))
		{
			mActiveExplosions.replaceWithLast(e);
		}
		else
		{
			mActiveExplosions[e].updateScale(mTime);
			e++;
		}
	}
#endif
}

void ImpactEmitterActor::registerImpact(const physx::PxVec3& hitPos, const physx::PxVec3& hitDir, const physx::PxVec3& surfNorm, physx::PxU32 surfType)
{
	NX_WRITE_ZONE();
	if (surfType < (PxU32) mEventSets.size())
	{
		// Check for non finite values (even in release build)
		bool hitPosFinite	= hitPos.isFinite();
		bool hitDirFinite	= hitDir.isFinite();
		bool surfNormFinite = surfNorm.isFinite();
		if (hitPosFinite && hitDirFinite && surfNormFinite)
		{
			ImpactEmitterEventSet& eventSet = mEventSets[surfType];
			for (physx::PxU32 e = 0; e < eventSet.entries.size(); ++e)
			{
				QueuedImpactEvent newEvent(hitPos, hitDir, surfNorm, mTime + eventSet.entries[e].dly, eventSet.entries[e].evnt);
				mPendingEvents.pushBack(newEvent);
			}
		}
		else
		{
			// Release and debug builds should output a warning message

			APEX_INVALID_PARAMETER("Asset Name: %s, hitPos: %s, hitDir: %s, surfNorm: %s, surface type: %d",
			                       mAsset->mName.c_str(),
			                       hitPosFinite	? "finite" : "nonFinite",
			                       hitDirFinite	? "finite" : "nonFinite",
			                       surfNormFinite	? "finite" : "nonFinite",
			                       surfType);

			// We really want debug builds to catch the culprits here
			PX_ASSERT(hitPos.isFinite());
			PX_ASSERT(hitDir.isFinite());
			PX_ASSERT(surfNorm.isFinite());
		}
	}
}

#if NX_SDK_VERSION_MAJOR == 2
void ImpactEmitterActor::createExplosion(NxExplosionAsset* asset, NxParameterized::Interface& desc, physx::PxF32 duration)
{
	PX_ASSERT(mExplEnvParams);

	// copy our explosion env desc to the default actor desc
	NxParameterized::Interface* descExplEnvParams = 0;
	NxParameterized::getParamRef(desc, "envSetting", descExplEnvParams);
	PX_ASSERT(descExplEnvParams);
	descExplEnvParams->copy(*mExplEnvParams);

	NxApexActor* apexActor = asset->createApexActor(desc, *(mScene->mApexScene));
	PX_ASSERT(apexActor);

	mActiveExplosions.pushBack(ActiveExplosion(static_cast<NxExplosionActor*>(apexActor), duration, mTime));
}
#endif

void ImpactEmitterActor::visualize(NiApexRenderDebug& renderDebug)
{
	if ( !mEnableDebugVisualization ) return;
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
#else
	renderDebug.setCurrentUserPointer((void*)(NxApexActor*)(this));
	renderDebug.pushRenderState();
	renderDebug.setCurrentColor(renderDebug.getDebugColor(DebugColors::Orange));

#if NX_SDK_VERSION_MAJOR == 2
	for (physx::PxU32 index = 0; index < mActiveExplosions.size(); ++index)
	{
		PxMat44 cameraFacingPose((mScene->mApexScene->getViewMatrix(0)).inverseRT());

		PxVec3 rayTo	= mActiveExplosions[index].expActor->getPose().column3.getXYZ();
		PxVec3 rayFrom	= mScene->mApexScene->getEyePosition(0) + mScene->mApexScene->getEyeDirection(0) + cameraFacingPose.column1.getXYZ() * -0.4f;

		//visualize raycast
		if (mScene->mEmitterDebugRenderParams->impactEmitterParameters.VISUALIZE_IMPACT_EMITTER_RAYCAST)
		{
			renderDebug.debugThickRay(rayFrom, rayTo);
		}
		cameraFacingPose.setPosition(rayTo);
		//visualize actor name
		if (mScene->mEmitterDebugRenderParams->impactEmitterParameters.VISUALIZE_IMPACT_EMITTER_ACTOR_NAME &&
		        mScene->mEmitterDebugRenderParams->impactEmitterParameters.THRESHOLD_DISTANCE_IMPACT_EMITTER_ACTOR_NAME > (-mScene->mApexScene->getEyePosition(0) + rayTo).magnitude())
		{
			renderDebug.debugOrientedText(cameraFacingPose, " %s %s", this->getOwner()->getObjTypeName(), this->getOwner()->getName());
		}

	}
#endif
	renderDebug.popRenderState();

	renderDebug.setCurrentUserPointer(NULL);
#endif
}

}
}
} // namespace physx::apex

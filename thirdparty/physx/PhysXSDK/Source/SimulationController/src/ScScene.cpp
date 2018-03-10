/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#define NOMINMAX

#include "ScPhysics.h"
#include "ScScene.h"
#include "ScClient.h"
#include "PxvBroadPhase.h"
#include "ScStaticSim.h"
#include "ScConstraintSim.h"
#include "ScConstraintProjectionManager.h"
#include "ScConstraintCore.h"
#include "ScArticulationCore.h"
#include "ScArticulationJointCore.h"
#include "ScMaterialCore.h"
#include "ScArticulationSim.h"
#include "ScArticulationJointSim.h"
#include "PsTime.h"
#include "CmEventProfiler.h"
#include "PxProfileEventId.h"
#include "ScConstraintInteraction.h"
#include "ScSimStats.h"
#include "ScTriggerPairs.h"
#include "ScObjectIDTracker.h"
#include "PxsArticulation.h"
#include "ScShapeIterator.h"

#ifdef PX_X360
	#include "xbox360\ScScene_XBOX.h"
#elif defined(PX_WINDOWS)
	#include "windows\ScScene_WIN.h"
#else
	#define PIX_PROFILE_ZONE(name)
#endif 

#if defined(__APPLE__) && defined(__POWERPC__)
#include <ppc_intrinsics.h>
#endif

#ifdef PX_PS3
#include "CellTimerMarker.h"
#endif

#if PX_SUPPORT_GPU_PHYSX
#include "PxPhysXGpu.h"
#endif

////////////

#include "ScShapeInstancePairLL.h"
#include "PxsContext.h"
#include "PxvBroadPhase.h"

#if PX_USE_PARTICLE_SYSTEM_API
#include "ScParticleSystemCore.h"
#include "ScParticleSystemSim.h"
#endif

#if PX_USE_CLOTH_API
#include "ScClothCore.h"
#include "ScClothSim.h"
#include "Factory.h"
#include "Fabric.h"
#include "Solver.h"
#include "Cloth.h"
#endif  // PX_USE_CLOTH_API

#if PX_SUPPORT_GPU_PHYSX
#include "PxGpuDispatcher.h"
#include "PxSceneGpu.h"
#endif

#include "PxRigidDynamic.h"

#include "PxsAABBManager.h"
#include "PxsDynamics.h"

#ifdef PX_PS3
#include "CellTimerMarker.h"
#endif

using namespace physx;
using namespace physx::shdfnd;
using namespace physx::Cm;

Sc::Scene::Scene(const PxSceneDesc& desc, Cm::EventProfiler& eventBuffer) :
	mBodyGravityDirty				(true),	
	mDt								(0),
	mOneOverDt						(0),
	mGlobalTime						(0.0f),
	mTimeStamp						(1),		// PT: has to start to 1 to fix determinism bug. I don't know why yet but it works.
	mReportShapePairTimeStamp		(0),
	mTriggerBufferAPI				(PX_DEBUG_EXP("sceneTriggerBufferAPI")),
	mRemovedShapeCountAtSimStart	(0),
	mArticulations					(PX_DEBUG_EXP("sceneArticulations")),
#if PX_USE_PARTICLE_SYSTEM_API
	mParticleSystems				(PX_DEBUG_EXP("sceneParticleSystems")),
	mEnabledParticleSystems			(PX_DEBUG_EXP("sceneEnabledParticleSystems")),
#endif
#if PX_USE_CLOTH_API
	mCloths							(PX_DEBUG_EXP("sceneCloths")),
#endif
	mBrokenConstraints				(PX_DEBUG_EXP("sceneBrokenConstraints")),
	mActiveBreakableConstraints		(PX_DEBUG_EXP("sceneActiveBreakableConstraints")),
	mMemBlock128Pool				(PX_DEBUG_EXP("PxsContext ConstraintBlock128Pool")),
	mMemBlock256Pool				(PX_DEBUG_EXP("PxsContext ConstraintBlock256Pool")),
	mMemBlock384Pool				(PX_DEBUG_EXP("PxsContext ConstraintBlock384Pool")),
	mNPhaseCore						(NULL),
	mSleepBodies					(PX_DEBUG_EXP("sceneSleepBodies")),
	mWokeBodies						(PX_DEBUG_EXP("sceneWokeBodies")),
	mEnableStabilization			(desc.flags & PxSceneFlag::eENABLE_STABILIZATION),
	mClients						(PX_DEBUG_EXP("sceneClients")),
	mEventProfiler					(eventBuffer),
	mInternalFlags					(SCENE_DEFAULT),
	mPublicFlags					(desc.flags),
	mStaticAnchor					(NULL),
	mBatchRemoveState				(NULL),
	mLostTouchPairs					(PX_DEBUG_EXP("sceneLostTouchPairs")),
	mOutOfBoundsIDs					(PX_DEBUG_EXP("sceneOutOfBoundsIds")),
	mErrorState						(0),
	mVisualizationScale				(0.0f),
	mVisualizationParameterChanged	(false),
	mNbRigidStatics					(0),
	mNbRigidDynamics				(0),
	mSimulateOrder					(desc.simulationOrder),
	mCollisionTask                  ("ScScene.collision"),
	mClothPreprocessing				(this, "ScScene.clothPreprocessing"),
	mPostNarrowPhase				(this, "ScScene.postNarrowPhase"),
	mParticlePostCollPrep			("ScScene.particlePostCollPrep"),
	mParticlePostShapeGen			(this, "ScScene.particlePostShapeGen"),
	mFinalizationPhase				(this, "ScScene.finalizationPhase"),
	mUpdateCCDMultiPass				(this, "ScScene.updateCCDMultiPass"),
	mPostSolver						(this, "ScScene.postSolver"),
	mSolver							(this, "ScScene.rigidBodySolver"),
	mPostIslandGenSecondPass		(this, "ScScene.postIslandGenSecondPass"),
	mIslandGenSecondPass			(this, "ScScene.islandGenSecondPass"),
	mUpdateDynamics					(this, "ScScene.updateDynamics"),
	mPostIslandGen					(this, "ScScene.postIslandGen"),
	mIslandGen						(this, "ScScene.islandGen"),
	mRigidBodyNarrowPhase			(this, "ScScene.rigidBodyNarrowPhase"),
	mPostBroadPhase					(this, "ScScene.postBroadPhase"),
	mBroadPhase						(this, "ScScene.broadPhase"),
	mSolveStep						(this, "ScScene.solveStep"),
	mCollideStep					(this, "ScScene.collideStep"),	
	mTaskPool						(16384),
	mContactReportsNeedPostSolverVelocity(false),
	mConstraintIndex				(0)
	{

#if PX_USE_CLOTH_API
	PxMemZero(mClothSolvers, sizeof(mClothSolvers));
	PxMemZero(mClothTasks, sizeof(mClothTasks));
	PxMemZero(mClothFactories, sizeof(mClothFactories));
#endif

	mInteractionScene			= PX_NEW(InteractionScene)(*this);
	mStats						= PX_NEW(SimStats);
	mShapeIDTracker				= PX_NEW(ObjectIDTracker);
	mRigidIDTracker				= PX_NEW(ObjectIDTracker);

	mTriggerBufferExtraData		= reinterpret_cast<TriggerBufferExtraData*>(PX_ALLOC(sizeof(TriggerBufferExtraData), "ScScene::TriggerBufferExtraData"));
	new(mTriggerBufferExtraData) TriggerBufferExtraData(PX_DEBUG_EXP("ScScene::TriggerPairExtraData"));

	mStaticSimPool				= PX_NEW(PreallocatingPool<StaticSim>)(64, "StaticSim");
	mBodySimPool				= PX_NEW(PreallocatingPool<BodySim>)(64, "BodySim");
	mShapeSimPool				= PX_NEW(PreallocatingPool<ShapeSim>)(64, "ShapeSim");
	mConstraintSimPool			= PX_NEW(Ps::Pool<ConstraintSim>)(PX_DEBUG_EXP("ScScene::ConstraintSim"));
	mConstraintInteractionPool	= PX_NEW(Ps::Pool<ConstraintInteraction>)(PX_DEBUG_EXP("ScScene::ConstraintInteraction"));

	mSimStateDataPool			= PX_NEW(Ps::Pool<SimStateData>)(PX_DEBUG_EXP("ScScene::SimStateData"));

	mClients.pushBack(PX_NEW(Client)());
	mProjectionManager = PX_NEW(ConstraintProjectionManager)();

	mTaskManager = PxTaskManager::createTaskManager(desc.cpuDispatcher, desc.gpuDispatcher, desc.spuDispatcher);

	for(PxU32 i=0; i<PxGeometryType::eGEOMETRY_COUNT; i++)
		mNbGeometries[i] = 0;
	
	if(!getInteractionScene().init(desc, mTaskManager, &mTaskPool, mEventProfiler))
		return;

	setSolverBatchSize(desc.solverBatchSize);
	setMeshContactMargin(desc.meshContactMargin);
	PxsContext* llContext = getInteractionScene().getLowLevelContext();
	llContext->setCorrelationDistance(desc.contactCorrelationDistance);
	PxsDynamicsContext* dynamicContext = llContext->getDynamicsContext();
	dynamicContext->setFrictionOffsetThreshold(desc.frictionOffsetThreshold);

	const PxTolerancesScale& scale = Physics::getInstance().getTolerancesScale();
	llContext->setToleranceLength(scale.length);

	// the original descriptor uses 
	//    bounce iff impact velocity  > threshold
	// but LL use 
	//    bounce iff separation velocity < -threshold 
	// hence we negate here.

	llContext->setBounceThreshold(-desc.bounceThresholdVelocity);

	StaticCore* anchorCore = PX_NEW(StaticCore)(PxTransform(PxIdentity));

	mStaticAnchor = mStaticSimPool->construct(*this, *anchorCore);

	mNPhaseCore = PX_NEW(NPhaseCore)(*this, desc);

	initDominanceMatrix();
		
//	DeterminismDebugger::begin();

	mWokeBodyListValid = true;
	mSleepBodyListValid = true;

	//load from desc:
	setLimits(desc.limits);

	// Create broad phase
	setBroadPhaseCallback(desc.broadPhaseCallback, PX_DEFAULT_CLIENT);

	setGravity(desc.gravity);

	setFrictionType(desc.frictionType);

	setPCM(desc.flags & PxSceneFlag::eENABLE_PCM);

	setContactCache(!(desc.flags & PxSceneFlag::eDISABLE_CONTACT_CACHE));
	setSimulationEventCallback(desc.simulationEventCallback, PX_DEFAULT_CLIENT);
	setContactModifyCallback(desc.contactModifyCallback);
	setCCDContactModifyCallback(desc.ccdContactModifyCallback);
	setCCDMaxPasses(desc.ccdMaxPasses);
	PX_ASSERT(mNPhaseCore); // refactor paranoia
	
	PX_ASSERT(	((desc.filterShaderData) && (desc.filterShaderDataSize > 0)) ||
				(!(desc.filterShaderData) && (desc.filterShaderDataSize == 0))	);
	if (desc.filterShaderData)
	{
		mFilterShaderData = PX_ALLOC(desc.filterShaderDataSize, PX_DEBUG_EXP("SceneDesc filterShaderData"));
		PxMemCopy(mFilterShaderData, desc.filterShaderData, desc.filterShaderDataSize);
		mFilterShaderDataSize = desc.filterShaderDataSize;
	}
	else
	{
		mFilterShaderData = NULL;
		mFilterShaderDataSize = 0;
	}
	mFilterShader = desc.filterShader;
	mFilterCallback = desc.filterCallback;

#if EXTRA_PROFILING
	mExtraProfileFile = fopen("extraProfile.txt", "w");
	mLineNum = 0;
#endif

#if PX_USE_CLOTH_API
	createClothSolver();
#endif  // PX_USE_CLOTH_API
}

void Sc::Scene::release()
{
	// TODO: PT: check virtual stuff

	mTimeStamp++;

	//collisionSpace.purgeAllPairs();

	//purgePairs();
	//releaseTagData();

	// We know release all the shapes before the collision space
	//collisionSpace.deleteAllShapes();

	//collisionSpace.release();

	//DeterminismDebugger::end();

	///clear broken constraint list:
	clearBrokenConstraintBuffer();

	PX_DELETE_AND_RESET(mNPhaseCore);

	PX_FREE_AND_RESET(mFilterShaderData);

	if (mStaticAnchor)
	{
		StaticCore& core = mStaticAnchor->getStaticCore();
		mStaticSimPool->destroy(mStaticAnchor);
		delete &core;
	}

#if EXTRA_PROFILING
	fclose(mExtraProfileFile);
#endif

	// Free object IDs and the deleted object id map
	postReportsCleanup();

	if (mTaskManager)
		mTaskManager->release();

	PX_DELETE_AND_RESET(mProjectionManager);

	for(PxU32 i=0;i<mClients.size(); i++)
		PX_DELETE_AND_RESET(mClients[i]);

	PX_DELETE(mConstraintInteractionPool);
	PX_DELETE(mConstraintSimPool);
	PX_DELETE(mSimStateDataPool);
	PX_DELETE(mStaticSimPool);
	PX_DELETE(mShapeSimPool);
	PX_DELETE(mBodySimPool);

#if PX_USE_CLOTH_API
	for(PxU32 i=0; i<mNumClothSolvers; ++i)
	{
		if(mClothSolvers[i])
			PX_DELETE(mClothSolvers[i]);

		if(i>0 && mClothFactories[i]) // don't delete sw factory
			PX_DELETE(mClothFactories[i]);
	}
#endif  // PX_USE_CLOTH_API

	mTriggerBufferExtraData->~TriggerBufferExtraData();
	PX_FREE(mTriggerBufferExtraData);

	PX_DELETE(mRigidIDTracker);
	PX_DELETE(mShapeIDTracker);
	PX_DELETE(mStats);
	PX_DELETE(mInteractionScene);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::Scene::preAllocate(PxU32 nbStatics, PxU32 nbBodies, PxU32 nbStaticShapes, PxU32 nbDynamicShapes, PxU32 nbAggregates)
{
	mStaticSimPool->preAllocate(nbStatics);

	mBodySimPool->preAllocate(nbBodies);

	mInteractionScene->preAllocate(nbBodies, nbStaticShapes, nbDynamicShapes, nbAggregates);
	mShapeSimPool->preAllocate(nbStaticShapes + nbDynamicShapes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxBroadPhaseType::Enum Sc::Scene::getBroadPhaseType() const
{
	PxvBroadPhase* bp = mInteractionScene->getLowLevelContext()->getAABBManager()->getBroadPhase();
	return bp->getType();
}

bool Sc::Scene::getBroadPhaseCaps(PxBroadPhaseCaps& caps) const
{
	PxvBroadPhase* bp = mInteractionScene->getLowLevelContext()->getAABBManager()->getBroadPhase();
	return bp->getCaps(caps);
}

PxU32 Sc::Scene::getNbBroadPhaseRegions() const
{
	PxvBroadPhase* bp = mInteractionScene->getLowLevelContext()->getAABBManager()->getBroadPhase();
	return bp->getNbRegions();
}

PxU32 Sc::Scene::getBroadPhaseRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	PxvBroadPhase* bp = mInteractionScene->getLowLevelContext()->getAABBManager()->getBroadPhase();
	return bp->getRegions(userBuffer, bufferSize, startIndex);
}

PxU32 Sc::Scene::addBroadPhaseRegion(const PxBroadPhaseRegion& region, bool populateRegion)
{
	PxvBroadPhase* bp = mInteractionScene->getLowLevelContext()->getAABBManager()->getBroadPhase();
	return bp->addRegion(region, populateRegion);
}

bool Sc::Scene::removeBroadPhaseRegion(PxU32 handle)
{
	PxvBroadPhase* bp = mInteractionScene->getLowLevelContext()->getAABBManager()->getBroadPhase();
	return bp->removeRegion(handle);
}

Ps::Array<void*>& Sc::Scene::getOutOfBoundsObjects()
{
	PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	return aabbMgr->getOutOfBoundsObjects();
}

Ps::Array<void*>& Sc::Scene::getOutOfBoundsAggregates()
{
	PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	return aabbMgr->getOutOfBoundsAggregates();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Cm::RenderBuffer& Sc::Scene::getRenderBuffer()
{
	return mInteractionScene->getLowLevelContext()->getRenderBuffer();
}

#if CACHE_LOCAL_CONTACTS_XP
	void PxcClearContactCacheStats();
	void PxcDisplayContactCacheStats();
#endif


void Sc::Scene::prepareSimulate()
{
	mReportShapePairTimeStamp++;	// deleted actors/shapes should get separate pair entries in contact reports
	mContactReportsNeedPostSolverVelocity = false;

	mRemovedShapeCountAtSimStart = mShapeIDTracker->getDeletedIDCount();

	getRenderBuffer().clear();

	///clear broken constraint list:
	clearBrokenConstraintBuffer();

	updateFromVisualizationParameters();

#if PX_USE_PARTICLE_SYSTEM_API
	// Build list of enabled particle systems
	mEnabledParticleSystems.clear();
	mEnabledParticleSystems.reserve(mParticleSystems.size());
	for(PxU32 i=0; i < mParticleSystems.size(); i++)
	{
		ParticleSystemCore* ps = mParticleSystems[i];
		if (ps->getFlags() & PxParticleBaseFlag::eENABLED)
		{
			mEnabledParticleSystems.pushBack(ps->getSim());
		}
	}
#endif	// PX_USE_PARTICLE_SYSTEM_API

	visualizeStartStep();
	
#ifdef DUMP_PROFILER
	dumpProfiler(this);
#endif

#if CACHE_LOCAL_CONTACTS_XP
	PxcClearContactCacheStats();
#endif

}
  

void Sc::Scene::simulate(PxReal timeStep, PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eSOLVER);
#endif

	if(timeStep != 0.f)
	{
		mDt = timeStep;
		mOneOverDt = 0.0f < mDt ? 1.0f/mDt : 0.0f;

		prepareSimulate();
		stepSetupSimulate(); 

		mSolveStep.setContinuation(continuation);
		mCollideStep.setContinuation(&mSolveStep);

		mSolveStep.removeReference();
		mCollideStep.removeReference();
	}

#ifdef PX_PS3
	stopTimerMarker(eSOLVER);
#endif
}

void Sc::Scene::solve(PxReal timeStep, PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eSIMULATE);
#endif

	if(timeStep != 0.0f)
	{
		mDt = timeStep;
		mOneOverDt = 0.0f < mDt ? 1.0f/mDt : 0.0f;

		stepSetupSolve(); 
		
		mSolveStep.setContinuation(continuation);
		mSolveStep.removeReference();
	}
 
#ifdef PX_PS3
	stopTimerMarker(eSIMULATE);
#endif
}

void Sc::Scene::setBounceThresholdVelocity(const PxReal t)
{
	getInteractionScene().getLowLevelContext()->setBounceThreshold(-t);
}

PxReal Sc::Scene::getBounceThresholdVelocity() const
{
	return -getInteractionScene().getLowLevelContext()->getBounceThreshold();
}

void Sc::Scene::collide(PxReal timeStep, PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eCOLLIDE);
#endif

	mDt = timeStep;
	// no need to update mOneOverDt

	prepareSimulate();

	//simulateCloth(continuation);
	mStats->simStart();

	getInteractionScene().getLowLevelContext()->beginUpdate();

	mCollideStep.setContinuation(continuation);
	mCollideStep.removeReference();

#ifdef PX_PS3
	stopTimerMarker(eCOLLIDE);
#endif
}

void Sc::Scene::setFrictionType(PxFrictionType::Enum model)
{
	getInteractionScene().getLowLevelContext()->setFrictionType(model);
}

PxFrictionType::Enum Sc::Scene::getFrictionType() const
{
	return getInteractionScene().getLowLevelContext()->getFrictionType();
}

void Sc::Scene::setPCM(bool enabled)
{
	getInteractionScene().getLowLevelContext()->setPCM(enabled);
}

void Sc::Scene::setContactCache(bool enabled)
{
	getInteractionScene().getLowLevelContext()->setContactCache(enabled);
}

void Sc::Scene::endSimulation()
{
	// Handle user contact filtering
	// Note: Do this before the contact callbacks get fired since the filter callback might
	//       trigger contact reports (touch lost due to re-filtering)
	mNPhaseCore->fireCustomFilteringCallbacks();

	mNPhaseCore->preparePersistentContactEventListForNextFrame();

	endStep();	// - Update time stamps

#if CACHE_LOCAL_CONTACTS_XP
	PxcDisplayContactCacheStats();
#endif
}


void Sc::Scene::flush(bool sendPendingReports)
{
	if (sendPendingReports)
	{
		fireQueuedContactCallbacks(true);
		fireTriggerCallbacks();
	}
	else
	{
		mNPhaseCore->clearContactReportActorPairs(true);  // To clear the actor pair set
	}
	postReportsCleanup();
	mNPhaseCore->freeContactReportStreamMemory();

	mConstraintArray.shrink();

	mTriggerBufferAPI.reset();
	mTriggerBufferExtraData->reset();

	clearBrokenConstraintBuffer();
	mBrokenConstraints.reset();

	clearSleepWakeBodies();  //!!! If we send out these reports on flush then this would not be necessary
	mSleepBodies.reset();
	mWokeBodies.reset();

	mClients.shrink();

	mShapeIDTracker->reset();
	mRigidIDTracker->reset();

	processLostTouchPairs();  // Processes the lost touch bodies
	PX_ASSERT(mLostTouchPairs.size() == 0);
	mLostTouchPairs.reset();
	// Does not seem worth deleting the bitmap for the lost touch pair list

	getInteractionScene().retrieveMemory();

	//!!! TODO: look into retrieving memory from the NPhaseCore & Broadphase class (all the pools in there etc.)

#if PX_USE_PARTICLE_SYSTEM_API
	mParticleSystems.shrink();
	mEnabledParticleSystems.reset();
#endif

#ifndef PX_PS3
	getInteractionScene().getLowLevelContext()->getNpMemBlockPool().releaseUnusedBlocks();
#endif
}


// User callbacks

void Sc::Scene::setSimulationEventCallback(PxSimulationEventCallback* callback, PxClientID client)
{
	PX_ASSERT(client < mClients.size());
	PxSimulationEventCallback*& current = mClients[client]->simulationEventCallback;
	if (!current && callback)
	{
		// if there was no callback before, the sleeping bodies have to be prepared for potential notification events (no shortcut possible anymore)
		for(PxU32 i=0; i < mSleepBodies.size(); i++)
			mSleepBodies[i]->getSim()->raiseInternalFlag(BodySim::BF_SLEEP_NOTIFY);
	}

	current = callback;
}

PxSimulationEventCallback* Sc::Scene::getSimulationEventCallback(PxClientID client) const
{
	PX_ASSERT(client < mClients.size());
	return mClients[client]->simulationEventCallback;
}

void Sc::Scene::setContactModifyCallback(PxContactModifyCallback* callback)
{
	getInteractionScene().getLowLevelContext()->setContactModifyCallback(callback);
}

PxContactModifyCallback* Sc::Scene::getContactModifyCallback() const
{
	return getInteractionScene().getLowLevelContext()->getContactModifyCallback();
}

void Sc::Scene::setCCDContactModifyCallback(PxCCDContactModifyCallback* callback)
{
	getInteractionScene().getLowLevelContext()->setCCDContactModifyCallback(callback);
}

PxCCDContactModifyCallback* Sc::Scene::getCCDContactModifyCallback() const
{
	return getInteractionScene().getLowLevelContext()->getCCDContactModifyCallback();
}

void Sc::Scene::setCCDMaxPasses(PxU32 ccdMaxPasses)
{
	getInteractionScene().getLowLevelContext()->setCCDMaxPasses(ccdMaxPasses);
}

PxU32 Sc::Scene::getCCDMaxPasses() const
{
	return getInteractionScene().getLowLevelContext()->getCCDMaxPasses();
}


void Sc::Scene::setBroadPhaseCallback(PxBroadPhaseCallback* callback, PxClientID client)
{
	PX_ASSERT(client < mClients.size());
	mClients[client]->broadPhaseCallback = callback;
}

PxBroadPhaseCallback* Sc::Scene::getBroadPhaseCallback(PxClientID client) const
{
	PX_ASSERT(client < mClients.size());
	return mClients[client]->broadPhaseCallback;
}

void Sc::Scene::removeBody(BodySim& body)	//this also notifies any connected joints!
{
	ConstraintGroupNode* node = body.getConstraintGroup();
	if (node)
	{
		//invalidate the constraint group:
		//this adds all constraints of the group to the dirty list such that groups get re-generated next frame
		getProjectionManager().invalidateGroup(*node, NULL);
	}

	BodyCore& core = body.getBodyCore();

	// Remove from sleepBodies array
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,API,findAndReplaceWithLast);
		mSleepBodies.findAndReplaceWithLast(&core);
	}

	PX_ASSERT(mSleepBodies.find(&core) == mSleepBodies.end());

	// Remove from wokeBodies array
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,API,findAndReplaceWithLast);
		mWokeBodies.findAndReplaceWithLast(&core);
	}
	PX_ASSERT(mWokeBodies.find(&core) == mWokeBodies.end());

	markReleasedBodyIDForLostTouch(body.getID());
}


void Sc::Scene::addConstraint(ConstraintCore& constraint, RigidCore* body0, RigidCore* body1)
{
	{
		Sc::RigidSim& rs0 = body0 ? *body0->getSim() : getStaticAnchor();
		Sc::RigidSim& rs1 = body1 ? *body1->getSim() : getStaticAnchor();
		if(!testInteractionCounts(rs0, rs1))
			return;
	}

	ConstraintSim* sim = mConstraintSimPool->construct(constraint, body0, body1, *this);

	// PT: TODO: getLowLevelConstraint() never returns NULL, code is unreachable
	if (sim && (sim->getLowLevelConstraint() == NULL))
	{
		mConstraintSimPool->destroy(sim);
		return;
	}

	mConstraintArray.pushBack(&constraint);
}


void Sc::Scene::removeConstraint(ConstraintCore& constraint)
{
	ConstraintSim* cSim = constraint.getSim();

	if (cSim)
	{
		BodySim* b = cSim->getAnyBody();
		ConstraintGroupNode* n = b->getConstraintGroup();
		
		if (n)
			getProjectionManager().invalidateGroup(*n, cSim);
		mConstraintSimPool->destroy(cSim);
	}

	mConstraintArray.findAndReplaceWithLast(&constraint);
}


void Sc::Scene::addArticulation(ArticulationCore& articulation, BodyCore& root)
{
	ArticulationSim* sim = PX_NEW(ArticulationSim)(articulation, *this, root);

	if (sim && (sim->getLowLevelArticulation() == NULL))
	{
		PX_DELETE(sim);
		return;
	}
	mArticulations.pushBack(&articulation);
}


void Sc::Scene::removeArticulation(ArticulationCore& articulation)
{
	ArticulationSim* a = articulation.getSim();
	if (a)
		PX_DELETE(a);
	mArticulations.findAndReplaceWithLast(&articulation);
}


void Sc::Scene::addArticulationJoint(ArticulationJointCore& joint, BodyCore& parent, BodyCore& child)
{
	ArticulationJointSim* sim = PX_NEW(ArticulationJointSim)(joint, *parent.getSim(), *child.getSim());
	PX_UNUSED(sim);
}


void Sc::Scene::removeArticulationJoint(ArticulationJointCore& joint)
{
	if (joint.getSim())
		joint.getSim()->destroy();  // Deletes the sim object as well
}




void Sc::Scene::addBrokenConstraint(Sc::ConstraintCore* c)
{
	PX_ASSERT(mBrokenConstraints.find(c) == mBrokenConstraints.end());
	mBrokenConstraints.pushBack(c);
}

void Sc::Scene::addActiveBreakableConstraint(Sc::ConstraintSim* c)
{
	PX_ASSERT(mActiveBreakableConstraints.find(c) == mActiveBreakableConstraints.end());
	mActiveBreakableConstraints.pushBack(c);
	c->setFlag(ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED);
}

void Sc::Scene::removeActiveBreakableConstraint(Sc::ConstraintSim* c)
{
	PX_ASSERT(mActiveBreakableConstraints.find(c) != mActiveBreakableConstraints.end());

	// the following is done as a linear search but the assumption is:
	// - This only happens when a breakable constraint is deactivated or deleted
	// - The list of active breakable constraints should be rather short
	mActiveBreakableConstraints.findAndReplaceWithLast(c);
	c->clearFlag(ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED);
}

void* Sc::Scene::allocateConstraintBlock(PxU32 size)
{
	if(size<=128)
		return mMemBlock128Pool.construct();
	else if(size<=256)
		return mMemBlock256Pool.construct();
	else  if(size<=384)
		return mMemBlock384Pool.construct();
	else
		return PX_ALLOC(size, PX_DEBUG_EXP("ConstraintBlock"));
}

void Sc::Scene::deallocateConstraintBlock(void* ptr, PxU32 size)
{
	if(size<=128)
		mMemBlock128Pool.destroy(reinterpret_cast<MemBlock128*>(ptr));
	else if(size<=256)
		mMemBlock256Pool.destroy(reinterpret_cast<MemBlock256*>(ptr));
	else  if(size<=384)
		mMemBlock384Pool.destroy(reinterpret_cast<MemBlock384*>(ptr));
	else
		PX_FREE(ptr);
}

PxBaseTask& Sc::Scene::scheduleCloth(PxBaseTask& continuation, bool afterBroadPhase)
{
	PX_UNUSED(continuation);
	PX_UNUSED(afterBroadPhase);
#if PX_USE_CLOTH_API
	if(*mClothSolvers)
	{
#if defined(PX_PS3)
		mClothSolvers[0]->setSpuCount(getSceneParamInt(PxPS3ConfigParam::eSPU_CLOTH));
#endif
		
		bool hasCollision = false;
		for (PxU32 i = 0; !hasCollision && i < mCloths.size(); ++i)
			hasCollision |= bool(mCloths[i]->getClothFlags() & PxClothFlag::eSCENE_COLLISION);
		
		if(hasCollision == afterBroadPhase)
		{
			// if no cloth uses scene collision, kick off cloth processing task
			PxBaseTask* solverTask = &mClothSolvers[0]->simulate(mDt, continuation);
			mClothPreprocessing.setContinuation(solverTask);
			solverTask->removeReference();
			return mClothPreprocessing;
		}
	}
#endif // PX_USE_CLOTH_API

	continuation.addReference();
	return continuation;
}


void Sc::Scene::scheduleClothGpu(PxBaseTask& continuation)
{
#if PX_USE_CLOTH_API

	for(PxU32 i=1; i<mNumClothSolvers; ++i)
	{
		if(!mClothSolvers[i])
			continue;

		// if there was a CUDA error last fame then we switch   
		// all cloth instances to Sw and destroy the GpuSolver 
		if (mClothSolvers[i]->hasError())
		{
			shdfnd::getFoundation().error(PxErrorCode::eDEBUG_WARNING, 
				__FILE__, __LINE__, "GPU cloth pipeline failed, switching to software");

			for (PxU32 j = 0; j < mCloths.size(); ++j)
				mCloths[j]->setClothFlag(PxClothFlag::eGPU, false);

			PX_DELETE_AND_RESET(mClothSolvers[i]);
			mClothTasks[i] = NULL;
		}
		else
		{
			mClothTasks[i] = &mClothSolvers[i]->simulate(mDt, continuation);
		}

		PX_ASSERT(*mClothSolvers); // make sure cpu cloth is scheduled if gpu cloth is
	}

#else
	PX_UNUSED(continuation);
#endif // PX_USE_CLOTH_API
}

PxBaseTask& Sc::Scene::scheduleParticleShapeGeneration(PxBaseTask& broadPhaseDependent, PxBaseTask& dynamicsCpuDependent)
{
	mParticlePostShapeGen.addDependent(broadPhaseDependent);
	mParticlePostShapeGen.addDependent(dynamicsCpuDependent);
	mParticlePostShapeGen.removeReference();

#if PX_USE_PARTICLE_SYSTEM_API
	if (mEnabledParticleSystems.size() > 0)
	{	
		PxBaseTask& task = Sc::ParticleSystemSim::scheduleShapeGeneration(getInteractionScene(), mEnabledParticleSystems, mParticlePostShapeGen);
		mParticlePostShapeGen.removeReference();
		return task;
	}
#endif

	return mParticlePostShapeGen;
}

PxBaseTask& Sc::Scene::scheduleParticleDynamicsCpu(PxBaseTask& continuation)
{
#if PX_USE_PARTICLE_SYSTEM_API
	if (mEnabledParticleSystems.size() > 0)
	{	
		return Sc::ParticleSystemSim::scheduleDynamicsCpu(getInteractionScene(), mEnabledParticleSystems, continuation);
	}
#endif

	continuation.addReference();
	return continuation;
}

PxBaseTask& Sc::Scene::scheduleParticleCollisionPrep(PxBaseTask& collisionCpuDependent,
	PxBaseTask& gpuDependent)
{
	mParticlePostCollPrep.addDependent(collisionCpuDependent);
	mParticlePostCollPrep.addDependent(gpuDependent);
	mParticlePostCollPrep.removeReference();

#if PX_USE_PARTICLE_SYSTEM_API
	if (mEnabledParticleSystems.size() > 0)
	{
		PxBaseTask& task = Sc::ParticleSystemSim::scheduleCollisionPrep(getInteractionScene(), mEnabledParticleSystems, mParticlePostCollPrep);
		mParticlePostCollPrep.removeReference();
		return task;
	}
#endif

	return mParticlePostCollPrep;
}

PxBaseTask& Sc::Scene::scheduleParticleCollisionCpu(PxBaseTask& continuation)
{
#if PX_USE_PARTICLE_SYSTEM_API
	if (mEnabledParticleSystems.size() > 0)
	{
		return Sc::ParticleSystemSim::scheduleCollisionCpu(getInteractionScene(), mEnabledParticleSystems, continuation);
	}
#endif

	continuation.addReference();
	return continuation;
}

PxBaseTask& Sc::Scene::scheduleParticleGpu(PxBaseTask& continuation)
{
#if PX_USE_PARTICLE_SYSTEM_API && PX_SUPPORT_GPU_PHYSX
	if (mEnabledParticleSystems.size() > 0)
	{
		return Sc::ParticleSystemSim::schedulePipelineGpu(getInteractionScene(), mEnabledParticleSystems, continuation);
	}
#endif

	continuation.addReference();
	return continuation;
}

//int testAxisConstraint(Sc::Scene& scene);
//int testCasts(Shape* shape);
//int testCasts(Shape& shape);

/*-------------------------------*\
| Adam's explanation of the RB solver:
| This is a novel idea of mine, 
| a combination of ideas on
| Milenkovic's Optimization
| Based Animation, and Trinkle's 
| time stepping schemes.
|
| A time step goes like this:
|
| Taking no substeps:
| 0) Compute contact points.
| 1) Update external forces. This may include friction.
| 2) Integrate external forces to current velocities.
| 3) Solve for impulses at contacts which will prevent 
|	interpenetration at next timestep given some 
|	velocity integration scheme.
| 4) Use the integration scheme on velocity to
|	reach the next state. Here we should not have any
|   interpenetration at the old contacts, but perhaps
|	at new contacts. If interpenetrating at new contacts,
|	just add these to the contact list; no need to repeat
|	the time step, because the scheme will get rid of the
|	penetration by the next step.
|
|
| Advantages:
| + Large steps, LOD realism.
| + very simple.
|
\*-------------------------------*/

void Sc::Scene::solveStep(PxBaseTask* continuation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,solveQueueTasks);

	PIX_PROFILE_ZONE(solveStep)

	if(mDt!=0.0f)
	{
		mFinalizationPhase.addDependent(*continuation);
		mFinalizationPhase.removeReference();

		if(mPublicFlags & PxSceneFlag::eENABLE_CCD)
		{
			mUpdateCCDMultiPass.setContinuation(&mFinalizationPhase);
			mPostSolver.setContinuation(&mUpdateCCDMultiPass);
			mUpdateCCDMultiPass.removeReference();
		}
		else
		{
			mPostSolver.setContinuation(&mFinalizationPhase);
		}

		PxBaseTask& clothTask = scheduleCloth(mPostSolver, true);
		mUpdateDynamics.setContinuation(&clothTask);
		mSolver.setContinuation(&mUpdateDynamics);
		mPostIslandGen.setContinuation(&mSolver);
		mIslandGen.setContinuation(&mPostIslandGen);

		mFinalizationPhase.removeReference();
		mPostSolver.removeReference();
		clothTask.removeReference();
		mUpdateDynamics.removeReference();
		mSolver.removeReference();
		mPostIslandGen.removeReference();
		mIslandGen.removeReference();
	}
}

/*-------------------------------*\
| For generating a task graph of the runtime task 
| execution have a look at the DOT_LOG define and
| https://wiki.nvidia.com/engwiki/index.php/PhysX/sdk/InternalDoc_Example_TaskGraph
|
| A method for understanding the code used to schedule tasks 
| is to read from the bottom to the top.
| Functions like Task& taskA = scheduleTask(taskB, taskC)
| can be read as "taskB and taskC depend on taskA"
\*-------------------------------*/

void Sc::Scene::collideStep(PxBaseTask* continuation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,collideQueueTasks);
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::Getcollision())

	PIX_PROFILE_ZONE(collideStep)

	mStats->simStart();
	getInteractionScene().getLowLevelContext()->beginUpdate();

	prepareParticleSystems();

	mFinalizationPhase.setTaskManager(*continuation->getTaskManager());
	mFinalizationPhase.addReference();

	mPostNarrowPhase.setContinuation(continuation);
	mRigidBodyNarrowPhase.setContinuation(&mPostNarrowPhase);
	mPostBroadPhase.addDependent(mRigidBodyNarrowPhase);
	mBroadPhase.setContinuation(&mPostBroadPhase);
	mCollisionTask.addDependent(mBroadPhase);

	if(hasParticleSystems())
	{
		PxBaseTask& particleGpuTask = scheduleParticleGpu(mFinalizationPhase);
		PxBaseTask& particleCollisionCpuTask = scheduleParticleCollisionCpu(mPostNarrowPhase);
		PxBaseTask& particleCollisionPrepTask = scheduleParticleCollisionPrep(particleCollisionCpuTask, particleGpuTask);
		PxBaseTask& particleDynamicsCpuTask = scheduleParticleDynamicsCpu(particleCollisionCpuTask);
		PxBaseTask& particleShapeGenTask = scheduleParticleShapeGeneration(mBroadPhase, particleDynamicsCpuTask);

		mPostBroadPhase.addDependent(particleCollisionPrepTask);
		mPostBroadPhase.removeReference();
		mCollisionTask.addDependent(particleShapeGenTask);
		mCollisionTask.removeReference();

		particleGpuTask.removeReference();
		particleCollisionCpuTask.removeReference();
		particleCollisionPrepTask.removeReference();
		particleDynamicsCpuTask.removeReference();
		particleShapeGenTask.removeReference();
	}

	scheduleClothGpu(mFinalizationPhase);
	PxBaseTask& clothTask = scheduleCloth(mFinalizationPhase, false);
	mCollisionTask.addDependent(clothTask);
	mCollisionTask.removeReference();
	clothTask.removeReference();

#if PX_SUPPORT_GPU_PHYSX
	//workaround to prevent premature launching of gpu launch task
	if (PxGpuDispatcher* gpuDispatcher = getTaskManager().getGpuDispatcher())
	{
		gpuDispatcher->getPreLaunchTask().removeReference();
	}
#endif

	mPostNarrowPhase.removeReference();
	mRigidBodyNarrowPhase.removeReference();
	mPostBroadPhase.removeReference();
	mBroadPhase.removeReference();
	mCollisionTask.removeReference();
}


void Sc::Scene::clothPreprocessing(PxBaseTask* /*continuation*/)
{
#if PX_USE_CLOTH_API
	for (PxU32 i = 0; i < mCloths.size(); ++i)
		mCloths[i]->getSim()->startStep();

	for(PxU32 i=0; i<mNumClothSolvers; ++i)
	{
		if (mClothTasks[i])
			mClothTasks[i]->removeReference();
	}
#endif
}

void Sc::Scene::broadPhase(PxBaseTask* continuation)
{
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetbroadPhase())
#if PX_USE_CLOTH_API
	for (PxU32 i = 0; i < mCloths.size(); ++i)
		mCloths[i]->getSim()->updateBounds();
#endif

	// - Updates bounds
	// - Deletes obsolete interactions
	getInteractionScene().getLowLevelContext()->updateBroadPhase(continuation, false);
}


void Sc::Scene::postBroadPhase(PxBaseTask* /*continuation*/)
{
	getInteractionScene().getLowLevelContext()->unMarkAllShapes();
	
	// - Finishes broadphase update
	// - Adds new interactions (and thereby contact managers if needed)
	finishBroadPhase(0); 
	
	// - Wakes actors that lost touch if appropriate
	processLostTouchPairs();

	// reset thread context before any tasks are spawned that fetch it (see US6664)
	getInteractionScene().getLowLevelContext()->resetThreadContexts();
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetbroadPhase())
} 

void Sc::Scene::rigidBodyNarrowPhase(PxBaseTask* continuation)
{
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetnarrowPhase())
	PxsContext* context=getInteractionScene().getLowLevelContext();
	context->updateContactManager(mDt, continuation); // Starts update of contact managers
}

void Sc::Scene::postNarrowPhase(PxBaseTask* /*continuation*/)
{

#ifdef PX_PS3
	startTimerMarker(ePOSTNARROWPHASE);
#endif

	if(hasParticleSystems())
	{
		getInteractionScene().getLowLevelContext()->getBodyTransformVault().update();
	}

	getInteractionScene().releaseConstraints(false);

#ifdef PX_PS3
	stopTimerMarker(ePOSTNARROWPHASE);
#endif
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetnarrowPhase())
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::Getcollision())
}

void Sc::Scene::particlePostShapeGen(PxBaseTask* /*continuation*/)
{
#if PX_USE_PARTICLE_SYSTEM_API
	for (PxU32 i = 0; i < mEnabledParticleSystems.size(); ++i)
		mEnabledParticleSystems[i]->processShapesUpdate();
#endif
}


void Sc::Scene::processNarrowPhaseTouchEvents()
{
	PxsContext* context = getInteractionScene().getLowLevelContext();

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,preIslandGen);

		// Update touch states from LL
		PxI32 newTouchCount, lostTouchCount;
		PxI32 ccdTouchCount = 0;
		context->getManagerTouchEventCount(&newTouchCount, &lostTouchCount, NULL);
		PX_ALLOCA(newTouches, PxvContactManagerTouchEvent, newTouchCount);
		PX_ALLOCA(lostTouches, PxvContactManagerTouchEvent, lostTouchCount);

		// Note: For contact notifications it is important that the new touch pairs get processed before the lost touch pairs.
		//       This allows to know for sure if a pair of actors lost all touch (see eACTOR_PAIR_LOST_TOUCH).
		context->fillManagerTouchEvents(newTouches, newTouchCount, lostTouches, lostTouchCount, NULL, ccdTouchCount);
		PX_ASSERT(!ccdTouchCount);
		for(PxI32 i=0; i<newTouchCount; ++i)
		{
			ShapeInstancePairLL* sipLL = (ShapeInstancePairLL*)newTouches[i].userData;
			PX_ASSERT(sipLL);
			sipLL->managerNewTouch(0, true);
		}
		for(PxI32 i=0; i<lostTouchCount; ++i)
		{
			ShapeInstancePairLL* sipLL = (ShapeInstancePairLL*)lostTouches[i].userData;
			PX_ASSERT(sipLL);
			if (sipLL->managerLostTouch(0, true)  && !sipLL->readIntFlag(ShapeInstancePairLL::CONTACTS_RESPONSE_DISABLED) )
				addToLostTouchList(sipLL->getShape0().getBodySim(), sipLL->getShape1().getBodySim());
		}

		mNPhaseCore->processPersistentContactEvents();
	}
}


void Sc::Scene::islandGen(PxBaseTask* continuation)
{
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetrigidBodySolver())
	PxsContext* context = getInteractionScene().getLowLevelContext();

	processNarrowPhaseTouchEvents();

#ifdef PX_PS3
	startTimerMarker(eUPDATE_ISLANDS);
#endif
	context->updateIslands(mDt, continuation);
}

PX_FORCE_INLINE void Sc::Scene::putObjectsToSleep(PxsIslandManager& islandManager, PxU32 infoFlag)
{
	//Set to sleep all bodies that were in awake islands that have just been put to sleep.
	const PxU8*const* PX_RESTRICT bodiesToSleep=islandManager.getBodiesToSleep();
	const PxU32 numBodiesToSleep=islandManager.getNumBodiesToSleep();
	for(PxU32 i=0;i<numBodiesToSleep;i++)
	{
		const PxU8* bodyPtr = bodiesToSleep[i];
		void* rBodyOwner = NULL;
		void* articOwner = NULL;
		PxsIslandManager::getBodyToWakeOrSleep(bodyPtr, rBodyOwner, articOwner);

		if(rBodyOwner)
		{
			BodySim* bodySim = (BodySim*)rBodyOwner;
			bodySim->setActive(false, infoFlag);
			PX_ASSERT(bodySim->sleepStateIntegrityCheck());
		}
		else
		{
			ArticulationSim* articSim = (ArticulationSim*)articOwner;
			articSim->setActive(false, infoFlag);
		}
	}
}

PX_FORCE_INLINE void Sc::Scene::wakeObjectsUp(PxsIslandManager& islandManager, PxU32 infoFlag)
{
	//Wake up all bodies that were in sleeping islands that have just been hit by a moving object.
	const PxU8*const* PX_RESTRICT bodiesToWake=islandManager.getBodiesToWake();
	const PxU32 numBodiesToWake=islandManager.getNumBodiesToWake();
	for(PxU32 i=0;i<numBodiesToWake;i++)
	{
		const PxU8* bodyPtr = bodiesToWake[i];
		void* rBodyOwner = NULL;
		void* articOwner = NULL;
		PxsIslandManager::getBodyToWakeOrSleep(bodyPtr, rBodyOwner, articOwner);

		if(rBodyOwner)
		{
			BodySim* bodySim = (BodySim*)rBodyOwner;
			bodySim->setActive(true, infoFlag);
			PX_ASSERT(bodySim->sleepStateIntegrityCheck());
		}
		else
		{
			ArticulationSim* articSim = (ArticulationSim*)articOwner;
			articSim->setActive(true, infoFlag);
		}
	}
}

void Sc::Scene::postIslandGen(PxBaseTask*)
{
#ifdef PX_PS3
	stopTimerMarker(eUPDATE_ISLANDS);
	startTimerMarker(ePOSTISLANDGEN);
#endif

	PxsIslandManager& islandManager = getInteractionScene().getLLIslandManager();

	putObjectsToSleep(islandManager, Actor::AS_PART_OF_ISLAND_GEN_PASS_1);
	wakeObjectsUp(islandManager, Actor::AS_PART_OF_ISLAND_GEN_PASS_1);

	// - Performs collision detection for trigger interactions
	mNPhaseCore->narrowPhase(); 

	//The shape pairs of all bodies that just woke up will have been given contact managers again.
	//Finish off the solver islands with the latest contact manager ptrs.
	islandManager.setWokenPairContactManagers();

	//Now perform a 2nd narrowphase pass on the pairs that were just woken up.
	NarrowPhaseContactManager* npCMs = islandManager.getNarrowPhaseContactManagers();
	const PxU32 numNpCMs = islandManager.getNumNarrowPhaseContactManagers();
	if(numNpCMs)
	{
		CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetrigidBodySolver())
		CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetnarrowPhase())

		mPostIslandGenSecondPass.setContinuation(&mSolver);
		mIslandGenSecondPass.setContinuation(&mPostIslandGenSecondPass);
		PxsContext* context = getInteractionScene().getLowLevelContext();
		context->secondPassUpdateContactManager(mDt, npCMs, numNpCMs, &mIslandGenSecondPass); // Starts update of contact managers
		mPostIslandGenSecondPass.removeReference();
		mIslandGenSecondPass.removeReference();
	}

#ifdef PX_PS3
	stopTimerMarker(ePOSTISLANDGEN);
#endif
}

void Sc::Scene::islandGenSecondPass(PxBaseTask* continuation)
{
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetnarrowPhase())
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetrigidBodySolver())

	PxsIslandManager& islandManager = getInteractionScene().getLLIslandManager();

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,preIslandGenSecondPass);

		//
		// After first island gen pass, all the bodies that got woken up will go through their interactions and activate them.
		// This will mark the edge as connected, if narrowphase ever ran for the corresponding contact manager and found a touch
		// the last time it ran (or unconnected if the last time it ran no touch was found). For pairs that never ran narrowphase
		// because they were added asleep, the edge will be set to unconnected. Now that narrowphase ran on the activated interactions,
		// we have to connect/unconnect the edges if the touch status changed.
		//

		PxI32 newTouchCount, lostTouchCount;
		PxI32 ccdTouchCount = 0;
		PxsContext* context = getInteractionScene().getLowLevelContext();
		context->getManagerTouchEventCount(&newTouchCount, &lostTouchCount, NULL);

		PX_ALLOCA(newTouches, PxvContactManagerTouchEvent, newTouchCount);
		PX_ALLOCA(lostTouches, PxvContactManagerTouchEvent, lostTouchCount);
		context->fillManagerTouchEvents(newTouches, newTouchCount, lostTouches, lostTouchCount, NULL, ccdTouchCount);
		PX_ASSERT(!ccdTouchCount);
		for(PxI32 i=0; i<newTouchCount; ++i)
		{
			ShapeInstancePairLL* sip = (ShapeInstancePairLL*)newTouches[i].userData;
			sip->postNarrowPhaseSecondPass(islandManager, true);
		}
		for(PxI32 i=0; i<lostTouchCount; ++i)
		{
			ShapeInstancePairLL* sip = (ShapeInstancePairLL*)lostTouches[i].userData;
			sip->postNarrowPhaseSecondPass(islandManager, false);
		}
	}

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,updateIslandsSecondPass);

		PxU32 numSpus = 0;
#ifdef PX_PS3
		numSpus = getSceneParamInt(PxPS3ConfigParam::eSPU_ISLAND_GEN);
#endif

		// Now touch information is available for islands that were initially asleep, they need to get processed again because they might get split into
		// separate subislands and some of them might be allowed to go to sleep again.
		islandManager.updateIslandsSecondPass(continuation, numSpus);
	}
}

void Sc::Scene::postIslandGenSecondPass(PxBaseTask*)
{
	PxsIslandManager& islandManager = getInteractionScene().getLLIslandManager();

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,postIslandGenSecondPass);

		//
		// If islands got split up, some subislands might be sleeping again because there was no real connectivity/touch to awake objects.
		// -> put those bodies to sleep again
		//

		PX_ASSERT(islandManager.getNumBodiesToWake() == 0);  // all the bodies in second pass islands are awake at the beginning, so there can be no wake up

		putObjectsToSleep(islandManager, Actor::AS_PART_OF_ISLAND_GEN_PASS_2);

		//
		// get the list of all second pass contact managers that did not end up in a sleeping island in the
		// second pass. We can now run the post narrowphase events on them (contact reports etc.)
		//
		islandManager.removeSleepingPairContactManagers();
		NarrowPhaseContactManager* npCMs = islandManager.getNarrowPhaseContactManagers();
		const PxU32 numNpCMs = islandManager.getNumNarrowPhaseContactManagers();
		PX_ASSERT(numNpCMs);
		for(PxU32 i=0; i < numNpCMs; i++)
		{
			PxsContactManager* cm = npCMs[i].mCM;
			ShapeInstancePairLL* sip = reinterpret_cast<ShapeInstancePairLL*>(cm->getUserData());

			sip->postIslandGenSecondPass();
		}
	}
}

void Sc::Scene::solver(PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eBEFORESOLVER);
#endif

	//Update forces per body in parallel. This can overlap with the other work in this phase.
	beforeSolver(continuation);

#ifdef PX_PS3
	stopTimerMarker(eBEFORESOLVER);
	startTimerMarker(eFINISHSOLVERISLANDS);
#endif
	//Recompute islands by removing contact managers that have a null constraint ptr.
	getInteractionScene().getLLIslandManager().finishSolverIslands();

	//Narrowphase is completely finished so the streams can be swapped.
#ifdef PX_PS3
	stopTimerMarker(eFINISHSOLVERISLANDS);
	startTimerMarker(eRUNMODIFIABLECONTACTMGRS);
#endif

	//Run contact modification (this might leave some contact managers with a null constraint ptr)
	getInteractionScene().getLowLevelContext()->runModifiableContactManagers();

#ifdef PX_PS3
	stopTimerMarker(eRUNMODIFIABLECONTACTMGRS);
	startTimerMarker(eSWAPSTREAMS);
#endif

	//Narrowphase is completely finished so the streams can be swapped.
	getInteractionScene().getLowLevelContext()->swapStreams();

#ifdef PX_PS3
	stopTimerMarker(eSWAPSTREAMS);
#endif

}

void Sc::Scene::updateDynamics(PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eUPDATE_DYNAMICS);
#endif

	getInteractionScene().getLowLevelContext()->updateDynamics(mDt, continuation, mMaterialManager);

#ifdef PX_PS3
	stopTimerMarker(eUPDATE_DYNAMICS);
#endif
}


void Sc::Scene::updateCCDMultiPass(PxBaseTask* parentContinuation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,updateCCDMultiPass);

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,IslandGen,freeBuffers);
		getInteractionScene().getLLIslandManager().freeBuffers();
	}

	// second run of the broadphase for making sure objects we have integrated did not tunnel.
	if(mPublicFlags & PxSceneFlag::eENABLE_CCD)
	{
		if (mContactReportsNeedPostSolverVelocity)
		{
			// the CCD code will overwrite the post solver body velocities, hence, we need to extract the info
			// first if any CCD enabled pair requested it.
			collectPostSolverVelocitiesBeforeCCD();
		}

		//We use 2 CCD task chains to be able to chain together an arbitrary number of ccd passes
		if(mPostCCDPass.size() != 2)
		{
			mPostCCDPass.clear();
			mUpdateCCDSinglePass.clear();
			mCCDBroadPhase.clear();
			mPostCCDPass.reserve(2);
			mUpdateCCDSinglePass.reserve(2);
			mCCDBroadPhase.reserve(2);
			for (int j = 0; j < 2; j++)
			{
				mPostCCDPass.pushBack(
					Cm::DelegateTask<Sc::Scene, &Sc::Scene::postCCDPass>(
					this, "ScScene.postCCDPass"));
				mUpdateCCDSinglePass.pushBack(
					Cm::DelegateTask<Sc::Scene, &Sc::Scene::updateCCDSinglePass>(
					this, "ScScene.updateCCDSinglePass"));
				mCCDBroadPhase.pushBack(
					Cm::DelegateTask<Sc::Scene, &Sc::Scene::ccdBroadPhase>(
					this, "ScScene.ccdBroadPhase"));
			}
		}


		//reset thread context in a place we know all tasks possibly accessing it, are in sync with. (see US6664)
		getInteractionScene().getLowLevelContext()->resetThreadContexts();

		getInteractionScene().getLowLevelContext()->updateCCDBegin();

		mCCDBroadPhase[0].setContinuation(parentContinuation);
		mCCDBroadPhase[0].removeReference();

	}

}


void Sc::Scene::ccdBroadPhase(PxBaseTask* continuation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,ccdBroadPhase);
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Sim::GetccdBroadPhaseComplete());

	PxsContext* context = getInteractionScene().getLowLevelContext();
	PxU32 currentPass = context->getCurrentCCDPass();
	const PxU32 ccdMaxPasses = context->getCCDMaxPasses();

	//If we are on the 1st pass or we had some sweep hits previous CCD pass, we need to run CCD again
	if( currentPass == 0 || context->getNumSweepHits())
	{
		const PxU32 currIndex = currentPass & 1;
		const PxU32 nextIndex = 1 - currIndex;
		//Initialize the CCD task chain unless this is the final pass
		if(currentPass != (ccdMaxPasses - 1))
		{
			mCCDBroadPhase[nextIndex].setContinuation(continuation);
		}
		mPostCCDPass[currIndex].setContinuation(currentPass == ccdMaxPasses-1 ? continuation : &mCCDBroadPhase[nextIndex]);
		mUpdateCCDSinglePass[currIndex].setContinuation(&mPostCCDPass[currIndex]);

		//Do the actual broad phase
		context->updateBroadPhase(&mUpdateCCDSinglePass[currIndex], true);

		//Allow the CCD task chain to continue
		mPostCCDPass[currIndex].removeReference();
		mUpdateCCDSinglePass[currIndex].removeReference();
		if(currentPass != (ccdMaxPasses - 1))
			mCCDBroadPhase[nextIndex].removeReference();
	}
}


void Sc::Scene::updateCCDSinglePass(PxBaseTask* continuation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,updateCCDSinglePass);
	mReportShapePairTimeStamp++;  // This will makes sure that new report pairs will get created instead of re-using the existing ones.

	PxsContext* context = getInteractionScene().getLowLevelContext();
	const PxU32 currentPass = context->getCurrentCCDPass() + 1;  // 0 is reserved for discrete collision phase
	finishBroadPhase(currentPass);
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Sim::GetccdBroadPhaseComplete());

	//reset thread context in a place we know all tasks possibly accessing it, are in sync with. (see US6664)
	context->resetThreadContexts();

	context->updateCCD(mDt, continuation, (mPublicFlags & PxSceneFlag::eDISABLE_CCD_RESWEEP));
}


void Sc::Scene::postSolver(PxBaseTask* continuation)
{
	PxsContext* llContext = getInteractionScene().getLowLevelContext();
	PxcNpMemBlockPool& blockPool = llContext->getNpMemBlockPool();

#ifdef PX_PS3
	//Remove nulls from deadArrays
	blockPool.releaseNullConstraintBlocks();
	blockPool.releaseNullFrictionBlocks();
	blockPool.updateSpuConstraintBlockCount();
	blockPool.updateSpuFrictionBlockCount();
#endif

	//Merge...
	llContext->getDynamicsContext()->mergeResults();
	//Swap friction!
	blockPool.swapFrictionStreams();

#if PX_ENABLE_SIM_STATS
	llContext->getSimStats().mPeakConstraintBlockAllocations = blockPool.getPeakConstraintBlockCount();
#endif

	// - Performs joint projection
	// - Finalize body motion
	// - Performs sleep check
	afterIntegration(continuation);
	// - Updates touch flags
	afterSolver(0); 		
}

void Sc::Scene::postCCDPass(PxBaseTask* /*continuation*/)
{
	// - Performs sleep check
	// - Updates touch flags

	PxsContext* context = getInteractionScene().getLowLevelContext();
	PxU32 currentPassforReport = context->getCurrentCCDPass();
	PX_ASSERT(currentPassforReport > 0); // to make sure changes to the CCD pass counting get noticed. For contact reports, 0 means discrete collision phase.

	int newTouchCount, lostTouchCount, ccdTouchCount;
	context->getManagerTouchEventCount(&newTouchCount, &lostTouchCount, &ccdTouchCount);
	PX_ALLOCA(newTouches, PxvContactManagerTouchEvent, newTouchCount);
	PX_ALLOCA(lostTouches, PxvContactManagerTouchEvent, lostTouchCount);
	PX_ALLOCA(ccdTouches, PxvContactManagerTouchEvent, ccdTouchCount);

	// Note: For contact notifications it is important that the new touch pairs get processed before the lost touch pairs.
	//       This allows to know for sure if a pair of actors lost all touch (see eACTOR_PAIR_LOST_TOUCH).
	context->fillManagerTouchEvents(newTouches, newTouchCount, lostTouches, lostTouchCount, ccdTouches, ccdTouchCount);
	for(PxI32 i=0; i<newTouchCount; ++i)
	{
		ShapeInstancePairLL* sipLL = (ShapeInstancePairLL*)newTouches[i].userData;
		PX_ASSERT(sipLL);
		sipLL->managerNewTouch(currentPassforReport, true);
	}
	for(PxI32 i=0; i<lostTouchCount; ++i)
	{
		ShapeInstancePairLL* sipLL = (ShapeInstancePairLL*)lostTouches[i].userData;
		PX_ASSERT(sipLL);
		if (sipLL->managerLostTouch(currentPassforReport, true) && !sipLL->readIntFlag(ShapeInstancePairLL::CONTACTS_RESPONSE_DISABLED)	)
			addToLostTouchList(sipLL->getShape0().getBodySim(), sipLL->getShape1().getBodySim());
	}
	for(PxI32 i=0; i<ccdTouchCount; ++i)
	{
		ShapeInstancePairLL* sipLL = (ShapeInstancePairLL*)ccdTouches[i].userData;
		PX_ASSERT(sipLL);
		sipLL->sendCCDRetouch(currentPassforReport);
	}
	afterSolver(currentPassforReport);
	{
		PxsTransformCache& cache = this->getInteractionScene().getLowLevelContext()->getTransformCache();
		Cm::Range<Actor*const> range = getInteractionScene().getActiveBodies();

		{
			CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,syncBodiesAfterSolver);

			Actor*const* actorPrefetch = &range.front() + 8;
			for(; !range.empty(); range.popFront())		
			{
				Actor *const activeActor = range.front();
				if(actorPrefetch <= &range.back())
					Ps::prefetch(*actorPrefetch++, 512);

				PX_ASSERT(activeActor->isDynamicRigid());

				BodySim* body = static_cast<BodySim*>(activeActor);

				PX_ASSERT(body->getBody2World().p.isFinite());
				PX_ASSERT(body->getBody2World().q.isFinite());

				body->updateCachedTransforms(cache);
			}
		}

		for(PxU32 i=0;i<mArticulations.size();i++)
		{
			mArticulations[i]->getSim()->updateCachedTransforms(cache, NULL);
		}
	}

}

void Sc::Scene::finalizationPhase(PxBaseTask* /*continuation*/)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,sceneFinalization);

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,IslandGen,freeBuffers);
		getInteractionScene().getLLIslandManager().freeBuffers();
	}

	checkConstraintBreakage(); // Performs breakage tests on breakable constraints

	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetrigidBodySolver())

	// Finish particleSystem simulation step
	// - Apply forces to rigid bodies (two-way interaction)
	// - Update particle id management structures
	// - Update packet bounds
	finishParticleSystems();

	visualizeEndStep();

	//saveLastCCDTransforms();

	mTaskPool.clear();

	mReportShapePairTimeStamp++;	// important to do this before fetchResults() is called to make sure that delayed deleted actors/shapes get
									// separate pair entries in contact reports
}

void Sc::Scene::saveLastCCDTransforms()
{
	// save last transforms for all rigid bodies to ccd state
	Cm::Range<Actor*const> range = getInteractionScene().getActiveBodies();
	{
		Actor*const* actorPrefetch = &range.front() + 8;
		for(; !range.empty(); range.popFront())		
		{
			if(actorPrefetch <= &range.back())
				Ps::prefetch(*actorPrefetch++, 512);

			Actor *const activeActor = range.front();

			PX_ASSERT(activeActor->isDynamicRigid());

			BodySim* body = static_cast<BodySim*>(activeActor);
			body->getLowLevelBody().saveLastCCDTransform();
		}
	}
}

void Sc::Scene::postReportsCleanup()
{
	mShapeIDTracker->processPendingReleases();
	mShapeIDTracker->clearDeletedIDMap();

	mRigidIDTracker->processPendingReleases();
	mRigidIDTracker->clearDeletedIDMap();
}

// Let the particle systems do some preparations before doing the "real" stuff.
// - Creation / deletion of particles
// - Particle update
// ...
void Sc::Scene::prepareParticleSystems()
{
#if PX_USE_PARTICLE_SYSTEM_API
	for(PxU32 i=0; i < mEnabledParticleSystems.size(); i++)
	{
		mEnabledParticleSystems[i]->startStep();
	}
#endif
}

// Do some postprocessing on particle systems.
void Sc::Scene::finishParticleSystems()
{
#if PX_USE_PARTICLE_SYSTEM_API
	for(PxU32 i=0; i < mEnabledParticleSystems.size(); i++)
	{
		mEnabledParticleSystems[i]->endStep();
	}
#endif
}

void Sc::Scene::kinematicsSetup()
{
	InteractionScene& is = getInteractionScene();
	PxU32 nbKinematics = is.getActiveOneWayDominatorCount();
	Actor*const* kinematics = is.getActiveOneWayDominatorBodies();
	Actor*const* kineEnd = kinematics + nbKinematics;
	Actor*const* kinePrefetch = kinematics + 16;
	for(PxU32 i = 0; i < nbKinematics; ++i)
	{
		if(kinePrefetch < kineEnd)
		{
			Ps::prefetch(static_cast<BodySim*>(*kinePrefetch), 1024);
			kinePrefetch++;
		}

		BodySim* b = static_cast<BodySim*>(kinematics[i]);
		PX_ASSERT(b->isKinematic());
		PX_ASSERT(b->isActive());

		b->calculateKinematicVelocity(mOneOverDt);
	}
}

void Sc::Scene::stepSetupSimulate()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,stepSetupSimulate);

	// Update timestamp
	mGlobalTime += mDt;

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,projectionTreeUpdates);
		mProjectionManager->processPendingUpdates(getInteractionScene().getLowLevelContext()->getScratchAllocator());
	}

	kinematicsSetup();
	// Update all dirty interactions
	mNPhaseCore->updateDirtyInteractions();
	mInternalFlags &= ~(SCENE_SIP_STATES_DIRTY_DOMINANCE | SCENE_SIP_STATES_DIRTY_VISUALIZATION);
}

//stepSetup is called in solve, but not collide
void Sc::Scene::stepSetupSolve()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,stepSetupSolve);

	kinematicsSetup();
}

void Sc::Scene::stepSetupCollide()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,stepSetupCollide);

	// Update timestamp
	mGlobalTime += mDt;

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,projectionTreeUpdates);
		mProjectionManager->processPendingUpdates(getInteractionScene().getLowLevelContext()->getScratchAllocator());
	}

	kinematicsSetup();
	// Update all dirty interactions
	mNPhaseCore->updateDirtyInteractions();
	mInternalFlags &= ~(SCENE_SIP_STATES_DIRTY_DOMINANCE | SCENE_SIP_STATES_DIRTY_VISUALIZATION);
}



void Sc::Scene::processLostTouchPairs()
{
#ifdef PX_PS3
	startTimerMarker(ePROCESS_LOST_TOUCH_PAIRS);
#endif

	for (PxU32 i=0; i<mLostTouchPairs.size(); ++i)
	{
		// If one has been deleted, we wake the other one
		const Ps::IntBool deletedBody1 = mLostTouchPairsDeletedBodyIDs.boundedTest(mLostTouchPairs[i].body1ID);
		const Ps::IntBool deletedBody2 = mLostTouchPairsDeletedBodyIDs.boundedTest(mLostTouchPairs[i].body2ID);
		if (deletedBody1 || deletedBody2)
		{
			if (!deletedBody1) mLostTouchPairs[i].body1->internalWakeUp();
			if (!deletedBody2) mLostTouchPairs[i].body2->internalWakeUp();
			continue;
		}

		// If both are sleeping, we let them sleep
		// (for example, two sleeping objects touch and the user teleports one (without waking it up))
		if (mLostTouchPairs[i].body1->isSleeping() &&
			mLostTouchPairs[i].body2->isSleeping())
		{
			continue;
		}

		// If only one has fallen asleep, we wake them both
		if (mLostTouchPairs[i].body1->isSleeping() ||
			mLostTouchPairs[i].body2->isSleeping())
		{
			mLostTouchPairs[i].body1->internalWakeUp();
			mLostTouchPairs[i].body2->internalWakeUp();
		}
	}


	mLostTouchPairs.clear();
	mLostTouchPairsDeletedBodyIDs.clear();

#ifdef PX_PS3
	stopTimerMarker(ePROCESS_LOST_TOUCH_PAIRS);
#endif
}

class ScBeforeSolverTask :  public Cm::Task
{
	static const PxU32 MaxBodiesPerTask = 128;
	const PxsRigidBody* const* mBodies;
	const PxVec3& mGravity;
	const bool mSimUsesAdaptiveForce;
	const bool mBodyGravityDirty;
	const PxReal mDt;
	const PxReal mOneOverDt;
	PxsContext* mContext;

public:
	bool mHasStaticTouch[MaxBodiesPerTask];
	PxU32 mNumBodies;
	

public:

	ScBeforeSolverTask(PxsRigidBody* const* bodies, const PxVec3& gravity,
		const bool bodyGravityDirty, const bool simUsesAdaptiveForce, PxReal dt, PxReal oneOverDt, PxsContext* context) : 
	  mBodies(bodies), mGravity(gravity), mSimUsesAdaptiveForce(simUsesAdaptiveForce), 
		  mBodyGravityDirty(bodyGravityDirty), mDt(dt), mOneOverDt(oneOverDt), mContext(context)
	{
	}

	virtual void runInternal()
	{		
		const PxU32 rigidBodyOffset = Sc::BodySim::getRigidBodyOffset();
		for(PxU32 i = 0; i < mNumBodies; i++)
		{
			Sc::BodySim* bodySim = (Sc::BodySim*)((PxU8*)mBodies[i] - rigidBodyOffset);
			bodySim->updateForces(mDt, mOneOverDt, mBodyGravityDirty, mGravity, mHasStaticTouch[i], mSimUsesAdaptiveForce);
			bodySim->getLowLevelBody().saveLastCCDTransform();
		}
	}

	virtual const char* getName() const
	{
		return "ScScene.beforeSolver";
	}

private:
	ScBeforeSolverTask& operator = (const ScBeforeSolverTask&);
};


void Sc::Scene::beforeSolver(PxBaseTask* continuation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,updateForces);

	for(PxU32 i=0;i<mArticulations.size();i++)
		mArticulations[i]->getSim()->checkResize();

	PxsContext* context = getInteractionScene().getLowLevelContext();

	// Note: For contact notifications it is important that force threshold checks are done after new/lost touches have been processed
	//       because pairs might get added to the list processed below

	// Atoms that passed contact force threshold
	PxsThresholdStream& thresholdStream = context->getThresholdStream();
	thresholdStream.clear();


	const PxsIslandManager& islandManager = getInteractionScene().getLowLevelContext()->getIslandManager();
	const PxsIslandObjects& islandObjects =islandManager.getIslandObjects();
	PxsRigidBody*const* PX_RESTRICT rbodies = islandObjects.bodies;
	ArticulationSim*const* PX_RESTRICT articSims = (ArticulationSim*const*)islandObjects.articulationOwners;
	const PxsIslandIndices* PX_RESTRICT islandIndices = islandManager.getIslandIndices();
	const PxU32 islandIndicesSize = islandManager.getIslandCount();

	const bool bBodyGravityDirty = (mBodyGravityDirty != 0);

	if(rbodies)
	{
		Ps::prefetchLine((PxU8*)rbodies[0],0);
		Ps::prefetchLine((PxU8*)rbodies[0],128);
	}

	const bool simUsesAdaptiveForce = readPublicFlag(PxSceneFlag::eADAPTIVE_FORCE);
	const PxVec3& gravity = getGravityFast();

	//bool hadStaticTouch = islandIndices[0].getHasStaticContact();

	PxU32 startIndex = 0;
	PxU32 currIndex = 0;

	Cm::FlushPool& flushPool = context->getTaskPool();

	const PxU32 MaxBodiesPerTask = 128;
	PxU32 ind = 0;

	ScBeforeSolverTask* task = PX_PLACEMENT_NEW(flushPool.allocate(sizeof(ScBeforeSolverTask)), ScBeforeSolverTask(rbodies, gravity, bBodyGravityDirty, simUsesAdaptiveForce, mDt, mOneOverDt, context));
	for(PxU32 i = 0; i < islandIndicesSize; i++)
	{
		PX_ASSERT(rbodies);  // to make static code analysis tool happy

		{
			const PxU32 start = islandIndices[i].bodies;
			const PxU32 end = islandIndices[i+1].bodies;

			bool hasStaticContact = islandIndices[i].getHasStaticContact();

			PxU32 j = start;
			while(j < end)
			{
				const PxU32 numToGrab = PxMin(MaxBodiesPerTask - (currIndex - startIndex), end-j);
				j += numToGrab;

				currIndex += numToGrab;

				for(PxU32 a = 0; a < numToGrab; ++a)
				{
					task->mHasStaticTouch[ind++] = hasStaticContact;
				}

				if((currIndex - startIndex) == MaxBodiesPerTask)
				{
					task->mNumBodies = MaxBodiesPerTask;
					task->setContinuation(continuation);
					task->removeReference();
					startIndex = currIndex;
					task = PX_PLACEMENT_NEW(flushPool.allocate(sizeof(ScBeforeSolverTask)), ScBeforeSolverTask(rbodies+startIndex, gravity, bBodyGravityDirty, simUsesAdaptiveForce, mDt, mOneOverDt, context));
					ind = 0;
				}
			}
		}
	}

	if((currIndex - startIndex) > 0)
	{
		task->mNumBodies = currIndex - startIndex;
		task->setContinuation(continuation);
		task->removeReference();
	}

	//Only enter loop if we had articulations
	if(islandIndices[islandIndicesSize].articulations)
	{
		for(PxU32 i = 0; i < islandIndicesSize; i++)
		{
			const bool hasStaticTouch = islandIndices[i].getHasStaticContact();
			{
				const PxU32 start = islandIndices[i].articulations;
				const PxU32 end = islandIndices[i+1].articulations;
				for(PxU32 j = start; j < end; j++)
				{
					// avoid indexing 1 past the end of rbodies array in the last island
					if (j < end-1)
					{
						Ps::prefetchLine(articSims[j+1],0);
						Ps::prefetchLine(articSims[j+1],128);
					}
					ArticulationSim* PX_RESTRICT articSim=(ArticulationSim*)articSims[j];
					articSim->updateForces(mDt, mOneOverDt, bBodyGravityDirty, gravity, hasStaticTouch, simUsesAdaptiveForce);
					articSim->saveLastCCDTransform();
				}
			}
		}
	}

	mBodyGravityDirty = false;
}


#ifdef PX_DEBUG
bool DEBUG_solverlock = false;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ScAfterIntegrationTask :  public Cm::Task
{

	const PxsRigidBody* const* mBodies;
	const PxU32 mNumBodies;
	const PxReal mDt;
	const PxReal mOneOverDt;
	const bool mEnableStabilization;
	PxsContext* mContext;
	PxsTransformCache& mCache;
	

public:

	static const PxU32 MaxRBodiesPerTask = 128;

	ScAfterIntegrationTask(PxsRigidBody* const* bodies, PxU32 numBodies, const bool enableStabilization, PxReal dt, PxReal oneOverDt, PxsContext* context,
		PxsTransformCache& cache) : 
	  mBodies(bodies), mNumBodies(numBodies), mDt(dt), mOneOverDt(oneOverDt), mEnableStabilization(enableStabilization), mContext(context), mCache(cache)
	{
	}

	virtual void runInternal()
	{		
		const PxU32 rigidBodyOffset = Sc::BodySim::getRigidBodyOffset();

		PxsThreadContext* mThreadContext = mContext->getThreadContext();

		Cm::BitMap& localChangedShapes = mThreadContext->getLocalChangedActors();
		localChangedShapes.clear();

		PxsIslandManagerNodeHook nodesReadyForSleeping[MaxRBodiesPerTask];
		PxU32 nbReadyForSleeping = 0;
		PxsIslandManagerNodeHook nodesNotReadyForSleeping[MaxRBodiesPerTask];
		PxU32 nbNotReadyForSleeping = 0;

		bool hasActiveShape = false;

		for(PxU32 i = 0; i < mNumBodies; i++)
		{
			Sc::BodySim* bodySim = (Sc::BodySim*)((PxU8*)mBodies[i] - rigidBodyOffset);
			bool readyForSleeping;
			bool notReadyForSleeping;
			bodySim->sleepCheck(mDt, mOneOverDt, mEnableStabilization, readyForSleeping, notReadyForSleeping);
			PX_ASSERT(!readyForSleeping || !notReadyForSleeping);
			if(readyForSleeping)
			{
				nodesReadyForSleeping[nbReadyForSleeping] = bodySim->getLLIslandManagerNodeHook();
				nbReadyForSleeping++;
			}
			else if(notReadyForSleeping)
			{
				nodesNotReadyForSleeping[nbNotReadyForSleeping] = bodySim->getLLIslandManagerNodeHook();
				nbNotReadyForSleeping++;
			}
			if(!(bodySim->getBodyCore().getCore().mInternalFlags & PxsRigidCore::eFROZEN))
			{
				PxcBpHandle handle = bodySim->getLowLevelBody().getAABBMgrId().mActorHandle;
				if(handle != PX_INVALID_BP_HANDLE)
				{
					localChangedShapes.growAndSet(handle);
					hasActiveShape = true;
				}
			}
			bodySim->updateCachedTransforms(mCache);
		}

		if(hasActiveShape || nbReadyForSleeping > 0 || nbNotReadyForSleeping > 0)
		{
			Ps::Mutex::ScopedLock lock(mContext->getDynamicsContext()->mLock);

			mContext->mergeChangedActorMap(localChangedShapes);

			//Write ready for sleeping.
			PxsIslandManager& islandManager = mContext->getIslandManager();
			for(PxU32 i = 0; i < nbReadyForSleeping; i++)
			{
				islandManager.notifyReadyForSleeping(nodesReadyForSleeping[i]);
			}
			//Write not ready for sleeping.
			for(PxU32 i = 0; i < nbNotReadyForSleeping; i++)
			{
				islandManager.notifyNotReadyForSleeping(nodesNotReadyForSleeping[i]);
			}
		}

		mContext->putThreadContext(mThreadContext);

	}

	virtual const char* getName() const
	{
		return "ScScene.afterIntegration";
	}

private:
	ScAfterIntegrationTask& operator = (const ScAfterIntegrationTask&);
};



void Sc::Scene::afterIntegration(PxBaseTask* continuation)
{

	const PxsIslandManager& islandManager = getInteractionScene().getLowLevelContext()->getIslandManager();
	const PxsIslandObjects& islandObjects =islandManager.getIslandObjects();
	PxsRigidBody*const* PX_RESTRICT rbodies = islandObjects.bodies;
	const PxsIslandIndices* PX_RESTRICT islandIndices = islandManager.getIslandIndices();
	const PxU32 islandIndicesSize = islandManager.getIslandCount();

	if(rbodies)
	{
		Ps::prefetchLine((PxU8*)rbodies[0],0);
		Ps::prefetchLine((PxU8*)rbodies[0],128);
	}

	PxsContext* context = getInteractionScene().getLowLevelContext();
	Cm::FlushPool& flushPool = context->getTaskPool();

	const PxU32 MaxBodiesPerTask = ScAfterIntegrationTask::MaxRBodiesPerTask;

	PxU32 numBodies = islandIndicesSize == 0 ? 0u : (PxU32)islandIndices[islandIndicesSize].bodies;

	PxsTransformCache& cache = this->getInteractionScene().getLowLevelContext()->getTransformCache();

	{
		InteractionScene& is = getInteractionScene();
		PxU32 nbKinematics = is.getActiveOneWayDominatorCount();
		Actor*const* kinematics = is.getActiveOneWayDominatorBodies();
		Actor*const* kineEnd = kinematics + nbKinematics;
		Actor*const* kinePrefetch = kinematics + 16;

		Cm::BitMap& shapeChangedMap = context->getChangedShapeMap();

		shapeChangedMap.clear();

		for(PxU32 i = 0; i < nbKinematics; ++i)
		{
			if(kinePrefetch < kineEnd)
			{
				Ps::prefetch(static_cast<BodySim*>(*kinePrefetch), 1024);
				kinePrefetch++;
			}

			BodySim* b = static_cast<BodySim*>(kinematics[i]);
			PX_ASSERT(b->isKinematic());
			PX_ASSERT(b->isActive());

			b->getLowLevelBody().saveLastCCDTransform();
			b->updateKinematicPose();
			if(PX_INVALID_BP_HANDLE!=b->getLowLevelBody().getAABBMgrId().mActorHandle)
				shapeChangedMap.growAndSet(b->getLowLevelBody().getAABBMgrId().mActorHandle);
			b->updateCachedTransforms(cache);
		}
	}

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,syncBodiesAfterIntegration);

		PxU32 secondPassBodyCount = 0;
		Cm::Range<Actor*const> range = getInteractionScene().getActiveBodies();
		PX_ALLOCA(secondPassBodies, ConstraintGroupNode*, range.size());

		Actor*const* actorPrefetch = &range.front() + 8;
		for(; !range.empty(); range.popFront())		
		{
			if(actorPrefetch <= &range.back())
				Ps::prefetch(*actorPrefetch++, 512);
			Actor *const activeActor = range.front();
			PX_ASSERT(activeActor->isDynamicRigid());
			BodySim* body = static_cast<BodySim*>(activeActor);
			// Weird stuff to handle that the root might be sleeping (hopefully only happens when it's kinematic)
			// That means that another body might have to add its root, unless it's already added
			if (body->getConstraintGroup())
			{
				ConstraintGroupNode& root = body->getConstraintGroup()->getRoot();
				if(!root.readFlag(ConstraintGroupNode::eIN_PROJECTION_PASS_LIST) && root.hasProjectionTreeRoot())
				{
					secondPassBodies[secondPassBodyCount++] = &root;
					root.raiseFlag(ConstraintGroupNode::eIN_PROJECTION_PASS_LIST);
				}
			}
		}

		{
			CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,projectConstraints);
			while(secondPassBodyCount--)
			{
				PX_ASSERT(secondPassBodies[secondPassBodyCount]->hasProjectionTreeRoot());//otherwise just don't put it into the second pass list.
				ConstraintGroupNode::projectPose(*secondPassBodies[secondPassBodyCount]);
				secondPassBodies[secondPassBodyCount]->clearFlag(ConstraintGroupNode::eIN_PROJECTION_PASS_LIST);
			}
		}

	}


	for(PxU32 i = 0; i < numBodies; i+=MaxBodiesPerTask)
	{
		ScAfterIntegrationTask* task = PX_PLACEMENT_NEW(flushPool.allocate(sizeof(ScAfterIntegrationTask)), ScAfterIntegrationTask(rbodies+i, PxMin(numBodies - i, MaxBodiesPerTask), mEnableStabilization, mDt, mOneOverDt, context, cache));
		task->setContinuation(continuation);
		task->removeReference();
	}
	
	

	if(mArticulations.size())
	{
		Ps::Mutex::ScopedLock lock(context->getDynamicsContext()->mLock);

		PxsThreadContext* threadContext = context->getThreadContext();
		Cm::BitMap& localShapeChangedMap = threadContext->getLocalChangedActors();
		for(PxU32 i=0;i<mArticulations.size();i++)
		{
			mArticulations[i]->getSim()->sleepCheck(mDt, mOneOverDt, mEnableStabilization);
			mArticulations[i]->getSim()->updateCachedTransforms(cache, &localShapeChangedMap);
		}

		context->mergeChangedActorMap(localShapeChangedMap);
		context->putThreadContext(threadContext);
	}
}


void Sc::Scene::afterSolver(const PxU32 ccdPass)
{
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::Getdynamics())
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,getSimEvents);

		PxsContext* context = getInteractionScene().getLowLevelContext();

		// Note: For contact notifications it is important that force threshold checks are done after new/lost touches have been processed
		//       because pairs might get added to the list processed below

		// Atoms that passed contact force threshold
		PxsThresholdStream& thresholdStream = context->getThresholdStream();
		bool haveThresholding = thresholdStream.size()!=0;

		PxsThresholdTable& thresholdTable = context->getThresholdTable();
		thresholdTable.build(thresholdStream);

		// Go through slow SIPs for contact reports
		ShapeInstancePairLL*const* pairArrays[2];
		PxU32 pairSizes[2];
		const NPhaseCore* npc = getNPhaseCore();
		pairArrays[0] = npc->getForceThresholdContactEventPairs();
		pairSizes[0] = npc->getForceThresholdContactEventPairCount();
		pairArrays[1] = npc->getAllPersistentContactEventPairs();  // need to get all because force threshold pairs can be in there too
		pairSizes[1] = npc->getAllPersistentContactEventPairCount();

		for(PxU32 j=0; j < 2; j++)
		{
			ShapeInstancePairLL*const* contactEventPairs = pairArrays[j];
			PxU32 size = pairSizes[j];
			while(size--)
			{
				ShapeInstancePairLL* sip = *contactEventPairs++;
				if(size)
				{
					ShapeInstancePairLL* nextSip = *contactEventPairs;
					Ps::prefetchLine(nextSip);
				}	

				PxU32 pairFlags = sip->getPairFlags();
				if (pairFlags & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS)
				{
					sip->swapAndClearForceThresholdExceeded();

					if (haveThresholding)
					{
						// PT: TODO: remove L2s when fetching those objects.
						PxsRigidBody* atomA = sip->getActor0().isDynamicRigid() ? &static_cast<BodySim&>(sip->getActor0()).getLowLevelBody() : 0;
						PxsRigidBody* atomB = sip->getActor1().isDynamicRigid() ? &static_cast<BodySim&>(sip->getActor1()).getLowLevelBody() : 0;	

						if (thresholdTable.check(thresholdStream, atomA, atomB, mDt))
						{
							sip->raiseFlag(ShapeInstancePairLL::FORCE_THRESHOLD_EXCEEDED_NOW);

							PX_ASSERT(sip->hasTouch());
						
							if ((!sip->readIntFlag(ShapeInstancePairLL::FORCE_THRESHOLD_EXCEEDED_BEFORE)) && (pairFlags & PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND))
							{
								sip->processUserNotification(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND, 0, false, ccdPass);
							}
							else if (sip->readIntFlag(ShapeInstancePairLL::FORCE_THRESHOLD_EXCEEDED_BEFORE) && (pairFlags & PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS))
							{
								sip->processUserNotification(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS, 0, false, ccdPass);
							}

							continue;
						}
					}

					if (sip->readIntFlag(ShapeInstancePairLL::FORCE_THRESHOLD_EXCEEDED_BEFORE) && (pairFlags & PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST))
					{
						sip->processUserNotification(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST, 0, false, ccdPass);
					}
				}
			}
		}
	}
	//getInteractionScene().getLLIslandManager().freeBuffers();
}


void Sc::Scene::endStep()
{
	mTimeStamp++;
//  INVALID_SLEEP_COUNTER is 0xffffffff. Therefore the last bit is masked. Look at Body::isForcedToSleep() for example.
//	if(timeStamp==PX_INVALID_U32)	timeStamp = 0;	// Reserve INVALID_ID for something else
	mTimeStamp &= 0x7fffffff;

	mReportShapePairTimeStamp++;  // to make sure that deleted shapes/actors after fetchResults() create new report pairs
}

void Sc::Scene::resizeReleasedBodyIDMaps(PxU32 maxActors, PxU32 numActors)
{ 
	mLostTouchPairsDeletedBodyIDs.resize(maxActors);
	mRigidIDTracker->resizeDeletedIDMap(maxActors,numActors); 
	mShapeIDTracker->resizeDeletedIDMap(maxActors,numActors);
}

/**
Render objects before simulation starts
*/
void Sc::Scene::visualizeStartStep()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,visualizeStartStep);

#if PX_ENABLE_DEBUG_VISUALIZATION
	if(!getVisualizationScale())
	{
		// make sure visualization inside simulate was skipped
		PX_ASSERT(getRenderBuffer().empty()); 
		return; // early out if visualization scale is 0
	}

	Cm::RenderOutput out(getRenderBuffer());

	if(getVisualizationParameter(PxVisualizationParameter::eCOLLISION_COMPOUNDS))
	{
		PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
		aabbMgr->visualize(out);
	}

	// Visualize joints
	for(PxU32 i=0;i<mConstraintArray.size(); i++)
		mConstraintArray[i]->getSim()->visualize(getRenderBuffer());

	mNPhaseCore->visualize(out);

	#if PX_USE_PARTICLE_SYSTEM_API
	for(PxU32 i=0; i < mParticleSystems.size(); i++)
		mParticleSystems[i]->getSim()->visualizeStartStep(out);
	#endif	// PX_USE_PARTICLE_SYSTEM_API
#endif
}

/**
Render objects after simulation finished. Use this for data that is only available after simulation.
*/
void Sc::Scene::visualizeEndStep()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,visualizeEndStep);

#if PX_ENABLE_DEBUG_VISUALIZATION
	if(!getVisualizationScale())
	{
		// make sure any visualization before was skipped
		PX_ASSERT(getRenderBuffer().empty()); 
		return; // early out if visualization scale is 0
	}

	Cm::RenderOutput out(getRenderBuffer());

#if PX_USE_PARTICLE_SYSTEM_API
	for(PxU32 i=0; i < mParticleSystems.size(); i++)
		mParticleSystems[i]->getSim()->visualizeEndStep(out);
#endif	// PX_USE_PARTICLE_SYSTEM_API
#endif
}


void Sc::Scene::collectPostSolverVelocitiesBeforeCCD()
{
	if (mContactReportsNeedPostSolverVelocity)
	{
		ActorPair*const* actorPairs = mNPhaseCore->getContactReportActorPairs();
		PxU32 nbActorPairs = mNPhaseCore->getNbContactReportActorPairs();
		for(PxU32 i=0; i < nbActorPairs; i++)
		{
			if (i < (nbActorPairs - 1))
				Ps::prefetchLine(actorPairs[i+1]);

			ActorPair* aPair = actorPairs[i];

			ContactStreamManager& cs = aPair->getContactStreamManager();

			PxU32 streamManagerFlag = cs.getFlags();
			if(streamManagerFlag & ContactStreamManagerFlag::eINVALID_STREAM)
				continue;

			PxU8* stream = mNPhaseCore->getContactReportPairData(cs.bufferIndex);
			
			if(i + 1 < nbActorPairs)
				Ps::prefetch(&(actorPairs[i+1]->getContactStreamManager()));

			if (!cs.extraDataSize)
				continue;
			else if (streamManagerFlag & ContactStreamManagerFlag::eNEEDS_POST_SOLVER_VELOCITY)
				cs.setContactReportPostSolverVelocity(stream, aPair->getActorA(), aPair->getActorB());
		}
	}
}


/*
Threading: called in the context of the user thread, but only after the physics thread has finished its run
*/
void Sc::Scene::fireQueuedContactCallbacks(bool asPartOfFlush)
{
	//if(contactNotifyCallback) //TODO: not sure if this is a key optimization, but to do something like this, we'd have to check if there are ANY contact reports set for any client.
	{
		// if buffered shape removals occured, then the criteria for testing the contact stream for events with removed shape pointers needs to be more strict.
		PX_ASSERT(asPartOfFlush || (mRemovedShapeCountAtSimStart <= mShapeIDTracker->getDeletedIDCount()));
		bool reducedTestForRemovedShapes = asPartOfFlush || (mRemovedShapeCountAtSimStart == mShapeIDTracker->getDeletedIDCount());
		const PxU32 removedShapeTestMask = (PxU32)(reducedTestForRemovedShapes ? ContactStreamManagerFlag::eTEST_FOR_REMOVED_SHAPES : (ContactStreamManagerFlag::eTEST_FOR_REMOVED_SHAPES | ContactStreamManagerFlag::eHAS_PAIRS_THAT_LOST_TOUCH));

		ActorPair*const* actorPairs = mNPhaseCore->getContactReportActorPairs();
		PxU32 nbActorPairs = mNPhaseCore->getNbContactReportActorPairs();
		for(PxU32 i=0; i < nbActorPairs; i++)
		{
			if (i < (nbActorPairs - 1))
				Ps::prefetchLine(actorPairs[i+1]);

			ActorPair* aPair = actorPairs[i];

			ContactStreamManager& cs = aPair->getContactStreamManager();

			PxU32 streamManagerFlag = cs.getFlags();
			if(streamManagerFlag & ContactStreamManagerFlag::eINVALID_STREAM)
				continue;

			const PxU16 nbShapePairs = cs.currentPairCount;
			ContactShapePair* contactPairs;

			PX_ASSERT(nbShapePairs > 0);
			
			PxU8* stream = mNPhaseCore->getContactReportPairData(cs.bufferIndex);
			if(!(streamManagerFlag & removedShapeTestMask))
				contactPairs = cs.getShapePairs(stream);  // better branch prediction
			else
			{
				contactPairs = cs.getShapePairs(stream);

				// At least one shape of this actor pair has been deleted. Need to traverse the contact buffer,
				// find the pairs which contain deleted shapes and set the flags accordingly.

				ContactStreamManager::convertDeletedShapesInContactStream(contactPairs, nbShapePairs, getShapeIDTracker());
			}

			if(i + 1 < nbActorPairs)
				Ps::prefetch(&(actorPairs[i+1]->getContactStreamManager()));

			PX_ASSERT(contactPairs);

			//multiclient support:
			PxClientID clientActor0 = aPair->getActorAClientID();
			PxClientID clientActor1 = aPair->getActorBClientID();

			PxU8 actor0ClientBehaviorFlags = aPair->getActorAClientBehavior();
			PxU8 actor1ClientBehaviorFlags = aPair->getActorBClientBehavior();

			ObjectIDTracker& RigidIDTracker = getRigidIDTracker();
			PxContactPairHeader pairHeader;
			pairHeader.actors[0] = static_cast<PxRigidActor*>(aPair->getPxActorA());
			pairHeader.actors[1] = static_cast<PxRigidActor*>(aPair->getPxActorB());
			PxU16 pairHeaderFlags = 0;
			if (RigidIDTracker.isDeletedID(aPair->getActorAID()))
				pairHeaderFlags |= PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
			if (RigidIDTracker.isDeletedID(aPair->getActorBID()))
				pairHeaderFlags |= PxContactPairHeaderFlag::eREMOVED_ACTOR_1;
			pairHeader.flags = PxContactPairHeaderFlags(pairHeaderFlags);
			PxU16 extraDataSize = cs.extraDataSize;
			if (!extraDataSize)
				pairHeader.extraDataStream = NULL;
			else
			{
				PX_ASSERT(extraDataSize >= sizeof(ContactStreamHeader));
				extraDataSize -= sizeof(ContactStreamHeader);
				pairHeader.extraDataStream = stream + sizeof(ContactStreamHeader);

				if (streamManagerFlag & ContactStreamManagerFlag::eNEEDS_POST_SOLVER_VELOCITY)
				{
					PX_ASSERT(!(pairHeaderFlags & Ps::to16(PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1)));
					cs.setContactReportPostSolverVelocity(stream, aPair->getActorA(), aPair->getActorB());
				}
			}
			pairHeader.extraDataStreamSize = extraDataSize;

			if (mClients[clientActor0]->simulationEventCallback &&
				(
				(clientActor0 == clientActor1)	//easy common case: the same client owns both shapes
				|| (	//else actor1 has a different owner -- see if we can still send this pair to the client of actor0:
					(mClients[clientActor0]->behaviorFlags & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_CONTACT_NOTIFY)//this client accepts foreign objects
					&& (actor1ClientBehaviorFlags & PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_CONTACT_NOTIFY)//this actor can be sent to foreign client
					)
				))
				mClients[clientActor0]->simulationEventCallback->onContact(pairHeader, reinterpret_cast<PxContactPair*>(contactPairs), nbShapePairs);

			if (
				(clientActor0 != clientActor1)	//don't call the same client twice
				&& mClients[clientActor1]->simulationEventCallback
				&& (mClients[clientActor1]->behaviorFlags & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_CONTACT_NOTIFY)//this client accepts foreign objects
				&& (actor0ClientBehaviorFlags & PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_CONTACT_NOTIFY)//this actor can be sent to foreign client
				)
				mClients[clientActor1]->simulationEventCallback->onContact(pairHeader, reinterpret_cast<PxContactPair*>(contactPairs), nbShapePairs);

			// estimates for next frame
			cs.maxPairCount = nbShapePairs;
			cs.setMaxExtraDataSize(cs.extraDataSize);
		}

		mNPhaseCore->clearContactReportStream();
		mNPhaseCore->clearContactReportActorPairs(asPartOfFlush);		
	}
}


PX_FORCE_INLINE void markDeletedShapes(Sc::ObjectIDTracker& idTracker, Sc::TriggerPairExtraData& tped, PxTriggerPair& pair)
{
	PxTriggerPairFlags::InternalType flags = 0;
	if (idTracker.isDeletedID(tped.shape0ID))
		flags |= PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER;
	if (idTracker.isDeletedID(tped.shape1ID))
		flags |= PxTriggerPairFlag::eREMOVED_SHAPE_OTHER;

	pair.flags = PxTriggerPairFlags(flags);
}


void Sc::Scene::fireTriggerCallbacks()
{
	// triggers
	const PxU32 nbTriggerPairs = mTriggerBufferAPI.size();
	PX_ASSERT(nbTriggerPairs == mTriggerBufferExtraData->size());
	if(nbTriggerPairs) 
	{
		// cases to take into account:
		// - no simulation shape has been removed -> no need to test shape references for removed shapes
		// - shapes have been removed but only while the simulation was not running -> only test the events that have 
		//   a marker for removed shapes set
		// - shapes have been removed while the simulation was running -> need to test all events (see explanation
		//   below)
		//
		// If buffered shape removals occured, then all trigger events need to be tested for removed shape pointers.
		// An optimization like in the contact report case is not applicable here because trigger interactions do not
		// have a reference to their reported events. It can happen that a trigger overlap found event is created but
		// a shape of that pair gets removed while the simulation is running. When processing the lost touch from the 
		// shape removal, no link to the overlap found event is possible and thus it can not be marked as dirty.
		const bool hasRemovedShapes = mShapeIDTracker->getDeletedIDCount() > 0;
		const bool forceTestsForRemovedShapes = (mRemovedShapeCountAtSimStart < mShapeIDTracker->getDeletedIDCount());

		if((mClients.size() == 1) && mClients[0]->simulationEventCallback)  // Simple and probably more common case
		{
			if (!(hasRemovedShapes || forceTestsForRemovedShapes))
				mClients[0]->simulationEventCallback->onTrigger(mTriggerBufferAPI.begin(), nbTriggerPairs);
			else
			{
				for(PxU32 i = 0; i < nbTriggerPairs; i++)
				{
					if (forceTestsForRemovedShapes || ((PxTriggerPairFlags::InternalType)mTriggerBufferAPI[i].flags & TriggerPairFlag::eTEST_FOR_REMOVED_SHAPES))
						markDeletedShapes(*mShapeIDTracker, (*mTriggerBufferExtraData)[i], mTriggerBufferAPI[i]);
				}

				mClients[0]->simulationEventCallback->onTrigger(mTriggerBufferAPI.begin(), nbTriggerPairs);
			}
		}
		else
		{
			PxU32 activeClients[(PX_MAX_CLIENTS+7)/8];
			PxMemSet(activeClients, 0, (PX_MAX_CLIENTS+7)/8);

			PxU16 activeClientLimit = 0;

			PxU32 nbValidPairs = 0;
			for(PxU32 i = 0; i < nbTriggerPairs; i++)
			{
				Sc::TriggerPairExtraData& tped = (*mTriggerBufferExtraData)[nbValidPairs];

				const PxU32 client0Broadcasting = PxU32(tped.actor0ClientBehavior & PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_TRIGGER_NOTIFY);
				const PxU32 client1Broadcasting = PxU32(tped.actor1ClientBehavior & PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_TRIGGER_NOTIFY);

				const PxU32 client0Listening = getClientBehaviorFlags(tped.client0ID) & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_TRIGGER_NOTIFY;
				const PxU32 client1Listening = getClientBehaviorFlags(tped.client1ID) & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_TRIGGER_NOTIFY;

				const bool reportTo0 = mClients[tped.client0ID]->simulationEventCallback && (tped.client0ID == tped.client1ID || (client0Listening && client1Broadcasting));
				const bool reportTo1 = mClients[tped.client1ID]->simulationEventCallback && (tped.client0ID != tped.client1ID && (client0Broadcasting && client1Listening));

				if(reportTo0 || reportTo1)
				{
					PxTriggerPair& triggerPair = mTriggerBufferAPI[nbValidPairs];

					if (forceTestsForRemovedShapes || ((PxTriggerPairFlags::InternalType)triggerPair.flags & TriggerPairFlag::eTEST_FOR_REMOVED_SHAPES))
						markDeletedShapes(*mShapeIDTracker, (*mTriggerBufferExtraData)[nbValidPairs], triggerPair);

					if(reportTo0)
					{
						activeClients[tped.client0ID>>3] |= 1<<(tped.client0ID&7);
						activeClientLimit = PxMax<PxU16>(PxU16(tped.client0ID+1), activeClientLimit);
					}
					else
						tped.client0ID = PX_MAX_CLIENTS;

					if(reportTo1)
					{
						activeClients[tped.client1ID>>3] |= 1<<(tped.client1ID&7);
						activeClientLimit = PxMax<PxU16>(PxU16(tped.client1ID+1), activeClientLimit);
					}
					else
						tped.client1ID = PX_MAX_CLIENTS;

					nbValidPairs++;
				}
				else
				{
					mTriggerBufferAPI.replaceWithLast(nbValidPairs);
					mTriggerBufferExtraData->replaceWithLast(nbValidPairs);
				}
			}

			Ps::InlineArray<PxTriggerPair, 32, Ps::TempAllocator> perClientArray;
			for(PxU32 i=0; i < activeClientLimit; i++)
			{
				if(!(activeClients[i>>3]&(1<<(i&7))))
					continue;
				perClientArray.clear();
				perClientArray.reserve(nbValidPairs);
				for(PxU32 j=0; j < nbValidPairs; j++)
				{
					if((*mTriggerBufferExtraData)[j].client0ID == i || (*mTriggerBufferExtraData)[j].client1ID == i)
						perClientArray.pushBack(mTriggerBufferAPI[j]);
				}

				mClients[i]->simulationEventCallback->onTrigger(perClientArray.begin(), perClientArray.size());
			}
		}
	}

	// PT: clear the buffer **even when there's no simulationEventCallback**.
	mTriggerBufferAPI.clear();
	mTriggerBufferExtraData->clear();
}


namespace
{
	struct BrokenConstraintReportData
	{
		BrokenConstraintReportData(PxConstraint* c, void* externalRef, PxU32 typeID, PxU16 i0, PxU16 i1): constraintInfo(c, externalRef, typeID), client0(i0), client1(i1) {}
		PxConstraintInfo constraintInfo;
		PxU16 client0;
		PxU16 client1;
	};
}

static const PxU16 NO_CLIENT = 0xffff;
static const PxActorClientBehaviorFlags ACTOR_BROADCASTING = PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_CONSTRAINT_BREAK_NOTIFY;
static const PxClientBehaviorFlags CLIENT_LISTENING = PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_CONSTRAINT_BREAK_NOTIFY;
void Sc::Scene::fireBrokenConstraintCallbacks()
{
	PxU32 count = mBrokenConstraints.size();
	Ps::InlineArray<BrokenConstraintReportData, 32, Ps::TempAllocator> notifyArray;
	notifyArray.reserve(count);

	PxU32 activeClients[(PX_MAX_CLIENTS+7)/8];
	PxMemSet(activeClients, 0, (PX_MAX_CLIENTS+7)/8);

	
	PxU16 activeClientLimit = 0;

	for(PxU32 i=0;i<count;i++)
	{
		Sc::ConstraintCore* c = mBrokenConstraints[i];

		Sc::RigidCore* mActors[2];
		if (c->getSim())  // the constraint might have been removed while the simulation was running
		{
			mActors[0] = (&c->getSim()->getRigid(0) != mStaticAnchor) ? &c->getSim()->getRigid(0).getRigidCore() : NULL;
			mActors[1] = (&c->getSim()->getRigid(1) != mStaticAnchor) ? &c->getSim()->getRigid(1).getRigidCore() : NULL;

			PxClientID clientID0 = mActors[0] ? mActors[0]->getOwnerClient() : PX_DEFAULT_CLIENT,
					   clientID1 = mActors[1] ? mActors[1]->getOwnerClient() : PX_DEFAULT_CLIENT;
			
			PxActorClientBehaviorFlags client0Broadcasting = mActors[0] ? mActors[0]->getClientBehaviorFlags() & ACTOR_BROADCASTING : PxActorClientBehaviorFlags(0);
			PxActorClientBehaviorFlags client1Broadcasting = mActors[1] ? mActors[1]->getClientBehaviorFlags() & ACTOR_BROADCASTING : PxActorClientBehaviorFlags(0);

			PxClientBehaviorFlags client0Listening = getClientBehaviorFlags(clientID0) & CLIENT_LISTENING;
			PxClientBehaviorFlags client1Listening = getClientBehaviorFlags(clientID1) & CLIENT_LISTENING;

			bool reportTo0 = mClients[clientID0]->simulationEventCallback && (clientID0 == clientID1 || (client0Listening && client1Broadcasting));
			bool reportTo1 = mClients[clientID1]->simulationEventCallback && (clientID0 != clientID1 && client0Broadcasting && client1Listening);

			if(reportTo0 || reportTo1)
			{
				PxU32 typeID = 0xffffffff;
				void* externalRef = c->getPxConnector()->getExternalReference(typeID);
				PX_CHECK_MSG(typeID != 0xffffffff, "onConstraintBreak: Invalid constraint type ID.");

				BrokenConstraintReportData d(c->getPxConstraint(), externalRef, typeID, reportTo0 ? PxU16(clientID0) : NO_CLIENT, PxU16(reportTo1) ? PxU16(clientID1) : NO_CLIENT);
				notifyArray.pushBack(d);
				if(reportTo0)
				{
					activeClients[clientID0>>3] |= 1<<(clientID0&7);
					activeClientLimit = PxMax<PxU16>(PxU16(clientID0+1), activeClientLimit);
				}

				if(reportTo1)
				{
					activeClients[clientID1>>3] |= 1<<(clientID1&7);
					activeClientLimit = PxMax<PxU16>(PxU16(clientID1+1), activeClientLimit);
				}
			}
		}
	}

	Ps::InlineArray<PxConstraintInfo, 32, Ps::TempAllocator> perClientArray;
	for(PxU32 i=0;i<activeClientLimit;i++)
	{
		if(!(activeClients[i>>3]&(1<<(i&7))))
			continue;
		perClientArray.clear();
		perClientArray.reserve(notifyArray.size());
		for(PxU32 j=0;j<notifyArray.size();j++)
		{
			if(notifyArray[j].client0 == i || notifyArray[j].client1 == i)
				perClientArray.pushBack(notifyArray[j].constraintInfo);
		}

		mClients[i]->simulationEventCallback->onConstraintBreak(perClientArray.begin(), perClientArray.size());
	}
}


/*
Threading: called in the context of the user thread, but only after the physics thread has finished its run
*/
void Sc::Scene::fireCallBacksPreSync()
{
	fireBrokenConstraintCallbacks();

	fireTriggerCallbacks();

	// Handle contact report logic
	fireQueuedContactCallbacks(false);
}


/*
Threading: called in the context of the user thread, but only after the physics thread has finished its run
*/
void Sc::Scene::fireCallBacksPostSync()
{
	//Temp-data
	PxActor** actors = NULL;

	//
	// Fire sleep & woken callbacks
	//
	if (
		//simulationEventCallback	//TODO: not sure if this is a key optimization, but to do something like this, we'd have to check if there are ANY contact reports set for any client.
		true)
	{
		// A body should be either in the sleep or the woken list. If it is in both, remove it from the list it was
		// least recently added to.

		if (!mSleepBodyListValid)
			cleanUpSleepBodies();

		if (!mWokeBodyListValid)
			cleanUpWokenBodies();

		// allocate temporary data
		PxU32 nbSleep = mSleepBodies.size();
		PxU32 nbWoken = mWokeBodies.size();
		PxU32 arrSize = PxMax(nbSleep, nbWoken);
		actors = arrSize ? (PxActor**) PX_ALLOC_TEMP(arrSize*sizeof(PxActor*), PX_DEBUG_EXP("PxActor*")) : NULL;

		if ((nbSleep > 0) && actors)
		{
			PxU32 destSlot = 0;
			PxClientID prevClient = (PxClientID)-1;
			for(PxU32 i=0; i < nbSleep; i++)
			{
				BodyCore* body = mSleepBodies[i];
				if (prevClient != body->getOwnerClient())
				{
					prevClient = body->getOwnerClient();
					//send off stuff buffered so far, then reset list:
					if(mClients[prevClient]->simulationEventCallback && destSlot)
						mClients[prevClient]->simulationEventCallback->onSleep(actors, destSlot);
					destSlot = 0;
				}
				if (body->getActorFlags() & PxActorFlag::eSEND_SLEEP_NOTIFIES)
					actors[destSlot++] = body->getPxActor();
			}

			if(mClients[prevClient]->simulationEventCallback && destSlot)
				mClients[prevClient]->simulationEventCallback->onSleep(actors, destSlot);

			//if (PX_DBG_IS_CONNECTED())
			//{
			//	for (PxU32 i = 0; i < nbSleep; ++i)
			//	{
			//		BodyCore* body = mSleepBodies[i];
			//		PX_ASSERT(body->getActorType() == PxActorType::eRIGID_DYNAMIC);
			//	}
			//}
		}

		// do the same thing for bodies that have just woken up

		if ((nbWoken > 0) && actors)
		{
			PxU32 destSlot = 0;
			PxClientID prevClient = (PxClientID)-1;
			for(PxU32 i=0; i < nbWoken; i++)
			{
				BodyCore* body = mWokeBodies[i];
				if (prevClient != body->getOwnerClient())
				{
					prevClient = body->getOwnerClient();
					//send off stuff buffered so far, then reset list:
					if(mClients[prevClient]->simulationEventCallback && destSlot)
						mClients[prevClient]->simulationEventCallback->onWake(actors, destSlot);
					destSlot = 0;
				}
				if (body->getActorFlags() & PxActorFlag::eSEND_SLEEP_NOTIFIES)
					actors[destSlot++] = body->getPxActor();
			}

			if(mClients[prevClient]->simulationEventCallback && destSlot)
				mClients[prevClient]->simulationEventCallback->onWake(actors, destSlot);

			//if (PX_DBG_IS_CONNECTED())
			//{
			//	for (PxU32 i = 0; i < nbWoken; ++i)
			//	{
			//		BodyCore* body = mWokeBodies[i];
			//		PX_ASSERT(actors[i]->getType() == PxActorType::eRIGID_DYNAMIC);
			//	}
			//}
		}

		clearSleepWakeBodies();
	}

	PX_FREE_AND_RESET(actors);
}

void Sc::Scene::prepareOutOfBoundsCallbacks()
{
	Ps::Array<void*>& outObjects = getOutOfBoundsObjects();
	const PxU32 nbOut0 = outObjects.size();

	mOutOfBoundsIDs.clear();
	for(PxU32 i=0;i<nbOut0;i++)
	{
		Element* volume = (Element*)outObjects[i];

		Sc::ShapeSim* sim = static_cast<Sc::ShapeSim*>(volume);
		PxU32 id = sim->getID();
		mOutOfBoundsIDs.pushBack(id);
	}
}


bool Sc::Scene::fireOutOfBoundsCallbacks()
{
	bool outputWarning = false;
	const Ps::Array<Client*>& clients = mClients;

	// Actors
	{
		Ps::Array<void*>& outObjects = getOutOfBoundsObjects();
		const PxU32 nbOut0 = outObjects.size();

		const ObjectIDTracker& tracker = getShapeIDTracker();

		for(PxU32 i=0;i<nbOut0;i++)
		{
			Element* volume = (Element*)outObjects[i];

			Sc::ShapeSim* sim = static_cast<Sc::ShapeSim*>(volume);
			if(tracker.isDeletedID(mOutOfBoundsIDs[i]))
				continue;

			Actor& actor = volume->getScActor();
			RigidSim& rigidSim = static_cast<RigidSim&>(actor);
			PxActor* pxActor = rigidSim.getPxActor();

			const PxClientID clientID = pxActor->getOwnerClient();
			PX_ASSERT(clients[clientID]);
			PxBroadPhaseCallback* cb = clients[clientID]->broadPhaseCallback;
			if(cb)
			{
				PxShape* px = sim->getPxShape();
				cb->onObjectOutOfBounds(*px, *pxActor);
			}
			else
			{
				outputWarning = true;
			}
		}
		outObjects.reset();
	}
	return outputWarning;
}


void Sc::Scene::postCallbacksPreSync()
{
	// Put/prepare kinematics to/for sleep and invalidate target pose
	// note: this needs to get done after the contact callbacks because
	//       the target might get read there.
	//
	InteractionScene& is = getInteractionScene();
	PxU32 nbKinematics = is.getActiveOneWayDominatorCount();
	Actor*const* kinematics = is.getActiveOneWayDominatorBodies();
	Actor*const* kineEnd = kinematics + nbKinematics;
	Actor*const* kinePrefetch = kinematics + 16;
	while(nbKinematics--)
	{
		if(kinePrefetch < kineEnd)
		{
			Ps::prefetchLine(static_cast<BodySim*>(*kinePrefetch));
			kinePrefetch++;
		}

		BodySim* b = static_cast<BodySim*>(kinematics[nbKinematics]);
		PX_ASSERT(b->isKinematic());
		PX_ASSERT(b->isActive());

		// Note: Can not assert for BF_KINEMATIC_MOVED being set because with invertes stepper you might call fetchResults() after collide() but before solve() has been kicked off
		b->deactivateKinematic();
		b->getBodyCore().invalidateKinematicTarget();
	}

	releaseConstraints(); //release constraint blocks at the end of the frame, so user can retrieve the blocks
}


PxU32 Sc::Scene::getErrorState()
{
	return mErrorState; // we only signal critical errors from the HW core
}


void Sc::Scene::setLimits(const PxSceneLimits & limits)
{
	mLimits = limits;
}


const PxSceneLimits& Sc::Scene::getLimits() const
{
	return mLimits;
}

void Sc::Scene::setNbContactDataBlocks(PxU32 numBlocks)
{
	getInteractionScene().getLowLevelContext()->getNpMemBlockPool().setBlockCount(numBlocks);
}

PxU32 Sc::Scene::getNbContactDataBlocksUsed() const
{
	return getInteractionScene().getLowLevelContext()->getNpMemBlockPool().getUsedBlockCount();
}

PxU32 Sc::Scene::getMaxNbContactDataBlocksUsed() const
{
	return getInteractionScene().getLowLevelContext()->getNpMemBlockPool().getMaxUsedBlockCount();
}

PxU32 Sc::Scene::getMaxNbConstraintDataBlocksUsed() const
{
	return getInteractionScene().getLowLevelContext()->getNpMemBlockPool().getPeakConstraintBlockCount();
}

void Sc::Scene::setScratchBlock(void* addr, PxU32 size)
{
	return getInteractionScene().getLowLevelContext()->setScratchBlock(addr, size);
}



void Sc::Scene::checkConstraintBreakage()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,checkConstraintBreakage);

	PxU32 count = mActiveBreakableConstraints.size();
	while(count)
	{
		count--;
		mActiveBreakableConstraints[count]->checkMaxForceExceeded();  // start from the back because broken constraints get removed from the list
	}
}

void Sc::Scene::getStats(PxSimulationStatistics& s) const
{
	mStats->readOut(s, getInteractionScene().getLowLevelContext()->getSimStats());
	s.nbStaticBodies = mNbRigidStatics;
	s.nbDynamicBodies = mNbRigidDynamics;
	for(PxU32 i=0; i<PxGeometryType::eGEOMETRY_COUNT; i++)
		s.nbShapes[i] = mNbGeometries[i];
}





void Sc::Scene::addShapes(void *const* shapes, PxU32 nbShapes, size_t ptrOffset, RigidSim& sim, PxsRigidBody* llBody)
{
	for(PxU32 i=0;i<nbShapes;i++)
	{
		ShapeCore& sc = *reinterpret_cast<ShapeCore*>(reinterpret_cast<size_t>(shapes[i])+ptrOffset);
		mShapeSimPool->construct(sim, sc, llBody);
		mNbGeometries[sc.getGeometryType()]++;
	}
}

void Sc::Scene::removeShapes(Sc::RigidSim& sim, Ps::InlineArray<Sc::ShapeSim*, 64>& shapesBuffer , Ps::InlineArray<const Sc::ShapeCore*,64>& removedShapes, bool wakeOnLostTouch)
{
	// DS: usual faff with deleting while iterating through an opaque iterator
	Sc::ShapeIterator iterator;
	iterator.init(sim);
	
	ShapeSim* s;
	while((s = iterator.getNext())!=NULL)
	{
		// can do two 2x the allocs in the worst case, but actors with >64 shapes are not common
		shapesBuffer.pushBack(s);
		removedShapes.pushBack(&s->getCore());
	}

	for(PxU32 i=0;i<shapesBuffer.size();i++)
		removeShape(*shapesBuffer[i], wakeOnLostTouch);
}


void Sc::Scene::addStatic(StaticCore& ro, void*const *shapes, PxU32 nbShapes, size_t shapePtrOffset)
{
	PX_ASSERT(ro.getActorCoreType() == PxActorType::eRIGID_STATIC);

	// sim objects do all the necessary work of adding themselves to broad phase,
	// activation, registering with the interaction framework, etc

	StaticSim* sim = mStaticSimPool->construct(*this, ro);
	mNbRigidStatics++;
	addShapes(shapes, nbShapes, shapePtrOffset, *sim, NULL);
}

void Sc::Scene::prefetchForRemove(const StaticCore&  core) const
{
	StaticSim* sim = core.getSim();
	if(sim)
	{
		Ps::prefetch(sim,sizeof(Sc::StaticSim));
		Ps::prefetch(sim->getElements_(),sizeof(Sc::Element));
	}
}

void Sc::Scene::prefetchForRemove(const BodyCore&  core) const
{
	BodySim *sim = core.getSim();	
	if(sim)
	{
		Ps::prefetch(sim,sizeof(Sc::BodySim));
		Ps::prefetch(sim->getElements_(),sizeof(Sc::Element));
	}
}

void Sc::Scene::removeStatic(StaticCore& ro, Ps::InlineArray<const Sc::ShapeCore*,64>& removedShapes, bool wakeOnLostTouch)
{
	PX_ASSERT(ro.getActorCoreType() == PxActorType::eRIGID_STATIC);

	StaticSim* sim = ro.getSim();
	if(sim)
	{
		if(mBatchRemoveState)
		{
			removeShapes(*sim, mBatchRemoveState->bufferedShapes ,removedShapes, wakeOnLostTouch);
		}
		else
		{
			Ps::InlineArray<Sc::ShapeSim*, 64>  shapesBuffer;
			removeShapes(*sim, shapesBuffer ,removedShapes, wakeOnLostTouch);
		}		
		mStaticSimPool->destroy(static_cast<Sc::StaticSim*>(ro.getSim()));
		mNbRigidStatics--;
	}
}


void Sc::Scene::addBody(BodyCore& body, void*const *shapes, PxU32 nbShapes, size_t shapePtrOffset)
{
	// sim objects do all the necessary work of adding themselves to broad phase,
	// activation, registering with the interaction framework, etc

	BodySim* sim = mBodySimPool->construct(*this, body);
	PX_UNUSED(sim);
	mNbRigidDynamics++;
	addShapes(shapes, nbShapes, shapePtrOffset, *sim, &sim->getLowLevelBody());
}

void Sc::Scene::removeBody(BodyCore& body, Ps::InlineArray<const Sc::ShapeCore*,64>& removedShapes, bool wakeOnLostTouch)
{
	BodySim *sim = body.getSim();	
	if(sim)
	{
		if(mBatchRemoveState)
		{
			removeShapes(*sim, mBatchRemoveState->bufferedShapes ,removedShapes, wakeOnLostTouch);
		}
		else
		{
			Ps::InlineArray<Sc::ShapeSim*, 64>  shapesBuffer;
			removeShapes(*sim,shapesBuffer, removedShapes, wakeOnLostTouch);
		}
		mBodySimPool->destroy(sim);
		mNbRigidDynamics--;
	}
}



void Sc::Scene::addShape(RigidSim& owner, ShapeCore& shapeCore)
{
	PxsRigidBody* atom = owner.getActorType() == PxActorType::eRIGID_DYNAMIC || owner.getActorType() == PxActorType::eARTICULATION_LINK ? &static_cast<BodySim&>(owner).getLowLevelBody() : NULL;

	mShapeSimPool->construct(owner, shapeCore, atom);
	mNbGeometries[shapeCore.getGeometryType()]++;
}

void Sc::Scene::removeShape(ShapeSim &shape, bool wakeOnLostTouch)
{
	mNbGeometries[shape.getCore().getGeometryType()]--;
	shape.removeFromBroadPhase(wakeOnLostTouch);
	mShapeSimPool->destroy(&shape);
}


void Sc::Scene::startBatchInsertion(BatchInsertionState&state)
{
	state.shapeSim = mShapeSimPool->allocateAndPrefetch();
	state.staticSim = mStaticSimPool->allocateAndPrefetch();
	state.bodySim = mBodySimPool->allocateAndPrefetch();														   
}


void Sc::Scene::addShapes(void *const* shapes, PxU32 nbShapes, size_t ptrOffset, RigidSim& rigidSim, PxsRigidBody* llBody, ShapeSim*& prefetchedShapeSim, PxBounds3* outBounds)
{
	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(i+1<nbShapes)
			Ps::prefetch(shapes[i+1], PxU32(ptrOffset+sizeof(Sc::ShapeCore)));
		ShapeSim* nextShapeSim = mShapeSimPool->allocateAndPrefetch();
		const ShapeCore& sc = *Ps::pointerOffset<const ShapeCore*>(shapes[i], (ptrdiff_t)ptrOffset);
		new(prefetchedShapeSim) ShapeSim(rigidSim, sc, llBody, (PxBounds3*)(&outBounds[i]));
		prefetchedShapeSim = nextShapeSim;
		mNbGeometries[sc.getGeometryType()]++;
	}	
}

void Sc::Scene::addStatic(PxActor* actor, BatchInsertionState& s, PxBounds3* outBounds)
{
	// static core has been prefetched by caller
	Sc::StaticSim* sim = s.staticSim;		// static core has been prefetched by the caller

	const Cm::PtrTable* shapeTable = Ps::pointerOffset<const Cm::PtrTable*>(actor, s.staticShapeTableOffset);
	void*const* shapes = shapeTable->getPtrs();
	if(shapeTable->getCount())
		Ps::prefetch(shapes[0],PxU32(s.shapeOffset+sizeof(Sc::ShapeCore)));

	mStaticSimPool->construct(sim, *this, *Ps::pointerOffset<Sc::StaticCore*>(actor, s.staticActorOffset));
	s.staticSim = mStaticSimPool->allocateAndPrefetch();

	addShapes(shapes, shapeTable->getCount(), (size_t)s.shapeOffset, *sim, NULL, s.shapeSim, outBounds);
	mNbRigidStatics++;
}

void Sc::Scene::addBody(PxActor* actor, BatchInsertionState& s, PxBounds3* outBounds)
{
	Sc::BodySim* sim = s.bodySim;		// body core has been prefetched by the caller

	const Cm::PtrTable* shapeTable = Ps::pointerOffset<const Cm::PtrTable*>(actor, s.dynamicShapeTableOffset);
	void*const* shapes = shapeTable->getPtrs();
	if(shapeTable->getCount())
		Ps::prefetch(shapes[0], PxU32(s.shapeOffset+sizeof(Sc::ShapeCore)));

	mBodySimPool->construct(sim, *this, *Ps::pointerOffset<Sc::BodyCore*>(actor, s.dynamicActorOffset));
	s.bodySim = mBodySimPool->allocateAndPrefetch();

	addShapes(shapes, shapeTable->getCount(), (size_t)s.shapeOffset, *sim, &sim->getLowLevelBody(), s.shapeSim, outBounds);
	mNbRigidDynamics++;
}

void Sc::Scene::finishBatchInsertion(BatchInsertionState& state)
{
	// a little bit lazy - we could deal with the last one in the batch specially to avoid overallocating by one.
	
	mStaticSimPool->releasePreallocated(static_cast<Sc::StaticSim*>(state.staticSim));	
	mBodySimPool->releasePreallocated(static_cast<Sc::BodySim*>(state.bodySim));
	mShapeSimPool->releasePreallocated(state.shapeSim);
}



// PT: TODO: get rid of those iterators eventually.

void Sc::Scene::initActiveBodiesIterator(BodyIterator& activeBodiesIterator)
{
	activeBodiesIterator = BodyIterator(getInteractionScene().getActiveBodies());
}

void Sc::Scene::initContactsIterator(ContactIterator& contactIterator)
{
	contactIterator = ContactIterator(getInteractionScene().getActiveInteractions(Sc::PX_INTERACTION_TYPE_OVERLAP));
}



void Sc::Scene::setDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2, const PxDominanceGroupPair& dominance)
{
	struct {
		void operator()(PxU32& bits, PxDominanceGroup shift, PxReal weight)
		{
			if(weight != PxReal(0))
				bits |=  (PxU32(1) << shift);
			else 
				bits &= ~(PxU32(1) << shift);
		}
	} bitsetter;

	bitsetter(mDominanceBitMatrix[group1], group2, dominance.dominance0);
	bitsetter(mDominanceBitMatrix[group2], group1, dominance.dominance1);

	mInternalFlags |= SCENE_SIP_STATES_DIRTY_DOMINANCE;		//force an update on all interactions on matrix change -- very expensive but we have no choice!!
}


PxDominanceGroupPair Sc::Scene::getDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2) const
{
	PxReal dom0 = (mDominanceBitMatrix[group1]>>group2) & 0x1 ? 1.0f : 0.0f;
	PxReal dom1 = (mDominanceBitMatrix[group2]>>group1) & 0x1 ? 1.0f : 0.0f;
	return PxDominanceGroupPair(dom0, dom1);
}

void Sc::Scene::setCreateContactReports(bool s)		
{ 
	PxsContext* context = mInteractionScene->getLowLevelContext();
	if(context)
		context->setCreateContactStream(s); 
}

void Sc::Scene::setSolverBatchSize(PxU32 solverBatchSize)
{
	getInteractionScene().getLowLevelContext()->setSolverBatchSize(solverBatchSize);
}


PxU32 Sc::Scene::getSolverBatchSize() const
{
	return getInteractionScene().getLowLevelContext()->getSolverBatchSize();
}


void Sc::Scene::setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value)
{
	mVisualizationParameterChanged = true;

	PX_ASSERT(getInteractionScene().getLowLevelContext()->getVisualizationParameter(PxVisualizationParameter::eSCALE) == mVisualizationScale); // Safety check because the scale is duplicated for performance reasons

	getInteractionScene().getLowLevelContext()->setVisualizationParameter(param, value);

	if (param == PxVisualizationParameter::eSCALE)
		mVisualizationScale = value;
}


PxReal Sc::Scene::getVisualizationParameter(PxVisualizationParameter::Enum param) const
{
	PX_ASSERT(getInteractionScene().getLowLevelContext()->getVisualizationParameter(PxVisualizationParameter::eSCALE) == mVisualizationScale); // Safety check because the scale is duplicated for performance reasons

	return getInteractionScene().getLowLevelContext()->getVisualizationParameter(param);
}


void Sc::Scene::setVisualizationCullingBox(const PxBounds3& box)
{
	getInteractionScene().getLowLevelContext()->setVisualizationCullingBox(box);
}

const PxBounds3& Sc::Scene::getVisualizationCullingBox() const
{
	return getInteractionScene().getLowLevelContext()->getVisualizationCullingBox();
}

void Sc::Scene::setMeshContactMargin(PxReal contactMargin)
{
	getInteractionScene().getLowLevelContext()->setMeshContactMargin(contactMargin);
}

PxReal Sc::Scene::getMeshContactMargin() const
{
	return getInteractionScene().getLowLevelContext()->getMeshContactMargin();
}

PxReal Sc::Scene::getContactCorrelationDistance() const
{
	return getInteractionScene().getLowLevelContext()->getCorrelationDistance();
}

PxReal Sc::Scene::getFrictionOffsetThreshold() const
{
	const PxsContext* llContext = getInteractionScene().getLowLevelContext();
	const PxsDynamicsContext* dynamicContext = llContext->getDynamicsContext();
	return dynamicContext->getFrictionOffsetThreshold();
}

PxU32 Sc::Scene::getDefaultContactReportStreamBufferSize() const
{
	return mNPhaseCore->getDefaultContactReportStreamBufferSize();
}


PX_FORCE_INLINE void buildActiveTransform(Sc::Actor*const PX_RESTRICT activeActor, Sc::Client** PX_RESTRICT clients, const PxU32 numClients)
{
	PX_ASSERT(activeActor->isDynamicRigid());
	
	Sc::BodySim* body = static_cast<Sc::BodySim*>(activeActor);
	if(!body->getBodyCore().getCore().mInternalFlags & PxsRigidCore::eFROZEN)
	{
		PxRigidActor* ra = static_cast<PxRigidActor*>(body->getPxActor());
		PX_ASSERT(ra != NULL);

		PxActiveTransform activeTransform;

		activeTransform.actor = ra;
		activeTransform.userData = ra->userData;
		activeTransform.actor2World = ra->getGlobalPose();

		PxClientID client = body->getActorCore().getOwnerClient();
		PX_ASSERT(client < numClients);
		PX_UNUSED(numClients);

		const PxU32 clientActiveTransformsSize=clients[client]->activeTransforms.size();
		clients[client]->activeTransforms.pushBack(activeTransform);
		Ps::prefetchLine((void*)((((size_t)(clients[client]->activeTransforms.begin() + clientActiveTransformsSize + 1)) + 128) & ~127)); 
	}
}

void Sc::Scene::buildActiveTransforms()
{
	Client** PX_RESTRICT clients=mClients.begin();
	const PxU32 numClients=mClients.size();

	InteractionScene& interactionScene=getInteractionScene();
	const PxU32 numActiveBodies=interactionScene.getNumActiveBodies();
	Actor*const* PX_RESTRICT activeBodies=interactionScene.getActiveBodiesArray();

	Ps::prefetchLine(activeBodies);

	for (PxU32 i = 0; i < numClients; i++)
	{
		clients[i]->activeTransforms.clear();
		Ps::prefetchLine((void*)(((size_t)mClients[i]->activeTransforms.begin() + 0)  & ~127)); 
		Ps::prefetchLine((void*)(((size_t)mClients[i]->activeTransforms.begin() + 128) & ~127)); 
	}

	const PxU32 numActiveBodies32=(numActiveBodies & ~31);

	for(PxU32 i=0;i<numActiveBodies32;i+=32)
	{
		Ps::prefetchLine(activeBodies+32);

		for(PxU32 j=0;j<32;j++)
		{
			// handle case where numActiveBodies is a multiple of 32
			// and we need to avoid reading one past end of the array
			if (i+j < numActiveBodies-1)
				Ps::prefetchLine(activeBodies[i+j+1]);

			buildActiveTransform(activeBodies[i+j],clients,numClients);
		}
	}

	for(PxU32 i=numActiveBodies32;i<numActiveBodies;i++)
	{
		if (i < numActiveBodies-1)
			Ps::prefetchLine(activeBodies[i+1]);

		buildActiveTransform(activeBodies[i],clients,numClients);
	}
}


PxActiveTransform* Sc::Scene::getActiveTransforms(PxU32& nbTransformsOut, PxClientID client)
{
	PX_ASSERT(client < mClients.size());

	nbTransformsOut = mClients[client]->activeTransforms.size();
	
	if(nbTransformsOut == 0)
	{
		return NULL;
	}
	return mClients[client]->activeTransforms.begin();
}


PxClientID Sc::Scene::createClient()
{
	mClients.pushBack(PX_NEW(Client)());
	return PxClientID(mClients.size()-1);
}

void Sc::Scene::setClientBehaviorFlags(PxClientID client, PxClientBehaviorFlags clientBehaviorFlags)
{
	PX_ASSERT(client < mClients.size());
	mClients[client]->behaviorFlags = clientBehaviorFlags;
}

PxClientBehaviorFlags Sc::Scene::getClientBehaviorFlags(PxClientID client) const
{
	PX_ASSERT(client < mClients.size());
	return mClients[client]->behaviorFlags;
}

#if PX_USE_CLOTH_API

void Sc::Scene::setClothInterCollisionDistance(PxF32 distance)
{
	PX_ASSERT(*mClothSolvers);
	for(PxU32 i=0; i<mNumClothSolvers; ++i)
	{
		if(mClothSolvers[i])
			mClothSolvers[i]->setInterCollisionDistance(distance);
	}
}

PxF32 Sc::Scene::getClothInterCollisionDistance() const
{
	PX_ASSERT(*mClothSolvers);
	return mClothSolvers[0]->getInterCollisionDistance();
}

void Sc::Scene::setClothInterCollisionStiffness(PxF32 stiffness)
{
	PX_ASSERT(*mClothSolvers);
	for(PxU32 i=0; i<mNumClothSolvers; ++i)
	{
		if(mClothSolvers[i])
			mClothSolvers[i]->setInterCollisionStiffness(stiffness);
	}
}

PxF32 Sc::Scene::getClothInterCollisionStiffness() const
{
	PX_ASSERT(*mClothSolvers);
	return mClothSolvers[0]->getInterCollisionStiffness();
}

void Sc::Scene::setClothInterCollisionNbIterations(PxU32 nbIterations)
{
	PX_ASSERT(*mClothSolvers);
	for(PxU32 i=0; i<mNumClothSolvers; ++i)
	{
		if(mClothSolvers[i])
			mClothSolvers[i]->setInterCollisionNbIterations(nbIterations);
	}
}

PxU32 Sc::Scene::getClothInterCollisionNbIterations() const
{
	PX_ASSERT(*mClothSolvers);
	return mClothSolvers[0]->getInterCollisionNbIterations();
}

#endif // #if PX_USE_CLOTH_API

void Sc::Scene::clearSleepWakeBodies(void)
{
	// Clear sleep/woken marker flags
	for(PxU32 i=0; i < mSleepBodies.size(); i++)
	{
		BodySim* body = mSleepBodies[i]->getSim();

		PX_ASSERT(!body->readInternalFlag(BodySim::BF_WAKEUP_NOTIFY));
		body->clearInternalFlag(BodySim::BF_SLEEP_NOTIFY);

		// A body can be in both lists depending on the sequence of events
		body->clearInternalFlag(BodySim::BF_IS_IN_SLEEP_LIST);
        body->clearInternalFlag(BodySim::BF_IS_IN_WAKEUP_LIST);
	}

	for(PxU32 i=0; i < mWokeBodies.size(); i++)
	{
		BodySim* body = mWokeBodies[i]->getSim();

		PX_ASSERT(!body->readInternalFlag(BodySim::BF_SLEEP_NOTIFY));
		body->clearInternalFlag(BodySim::BF_WAKEUP_NOTIFY);

		// A body can be in both lists depending on the sequence of events
		body->clearInternalFlag(BodySim::BF_IS_IN_SLEEP_LIST);
        body->clearInternalFlag(BodySim::BF_IS_IN_WAKEUP_LIST);
	}

	mSleepBodies.clear();
	mWokeBodies.clear();
	mWokeBodyListValid = true;
	mSleepBodyListValid = true;
}


void Sc::Scene::onBodySleep(BodySim* body)
{
	//temp: TODO: Add support for other clients
	PxSimulationEventCallback* simulationEventCallback = mClients[PX_DEFAULT_CLIENT]->simulationEventCallback;

	if (simulationEventCallback)
	{
		if (body->readInternalFlag(BodySim::BF_WAKEUP_NOTIFY))
		{
			PX_ASSERT(!body->readInternalFlag(BodySim::BF_SLEEP_NOTIFY));

			// Body is in the list of woken bodies, hence, mark this list as dirty such that it gets cleaned up before
			// being sent to the user
			body->clearInternalFlag(BodySim::BF_WAKEUP_NOTIFY);
			mWokeBodyListValid = false;
		}

		body->raiseInternalFlag(BodySim::BF_SLEEP_NOTIFY);

		// Avoid multiple insertion (the user can do multiple transitions between asleep and awake)
		if (!body->readInternalFlag(BodySim::BF_IS_IN_SLEEP_LIST))
		{
			PX_ASSERT(mSleepBodies.find(&body->getBodyCore()) == mSleepBodies.end());
			mSleepBodies.pushBack(&body->getBodyCore());
			body->raiseInternalFlag(BodySim::BF_IS_IN_SLEEP_LIST);
		}
	}
	else
	{
		// even if no sleep events are requested, we still need to track the objects which were put to sleep because
		// for those we need to sync the buffered state (strictly speaking this only applies to sleep changes that are 
		// triggered by the simulation and not the user but we do not distinguish here)
		if (!body->readInternalFlag(BodySim::BF_IS_IN_SLEEP_LIST))
			mSleepBodies.pushBack(&body->getBodyCore());
		body->raiseInternalFlag(BodySim::BF_IS_IN_SLEEP_LIST);
	}
}


void Sc::Scene::onBodyWakeUp(BodySim* body)
{
	//temp: TODO: Add support for other clients
	PxSimulationEventCallback* simulationEventCallback = mClients[PX_DEFAULT_CLIENT]->simulationEventCallback;

	if (!simulationEventCallback)
		return;

	if (body->readInternalFlag(BodySim::BF_SLEEP_NOTIFY))
	{
		PX_ASSERT(!body->readInternalFlag(BodySim::BF_WAKEUP_NOTIFY));

		// Body is in the list of sleeping bodies, hence, mark this list as dirty such it gets cleaned up before
		// being sent to the user
		body->clearInternalFlag(BodySim::BF_SLEEP_NOTIFY);
		mSleepBodyListValid = false;
	}

	body->raiseInternalFlag(BodySim::BF_WAKEUP_NOTIFY);

	// Avoid multiple insertion (the user can do multiple transitions between asleep and awake)
	if (!body->readInternalFlag(BodySim::BF_IS_IN_WAKEUP_LIST))
	{
		PX_ASSERT(mWokeBodies.find(&body->getBodyCore()) == mWokeBodies.end());
		mWokeBodies.pushBack(&body->getBodyCore());
		body->raiseInternalFlag(BodySim::BF_IS_IN_WAKEUP_LIST);
	}
}


PX_INLINE void Sc::Scene::cleanUpSleepBodies()
{
	cleanUpSleepOrWokenBodies(mSleepBodies, BodySim::BF_WAKEUP_NOTIFY, mSleepBodyListValid);
}


PX_INLINE void Sc::Scene::cleanUpWokenBodies()
{
	cleanUpSleepOrWokenBodies(mWokeBodies, BodySim::BF_SLEEP_NOTIFY, mWokeBodyListValid);
}


PX_INLINE void Sc::Scene::cleanUpSleepOrWokenBodies(Ps::Array<BodyCore*>& bodyList, PxU32 removeFlag, bool& validMarker)
{
	// With our current logic it can happen that a body is added to the sleep as well as the woken body list in the
	// same frame.
	//
	// Examples:
	// - Kinematic is created (added to woken list) but has not target (-> deactivation -> added to sleep list)
	// - Dynamic is created (added to woken list) but is forced to sleep by user (-> deactivation -> added to sleep list)
	//
	// This code traverses the sleep/woken body list and removes bodies which have been initially added to the given
	// list but do not belong to it anymore.

	PxU32 i = 0;
	while (i < bodyList.size())
	{
		BodySim* body = bodyList[i]->getSim();

		if (body->readInternalFlag(static_cast<BodySim::InternalFlags>(removeFlag)))
			bodyList.replaceWithLast(i);
		else
			i++;
	}

	validMarker = true;
}


void Sc::Scene::releaseConstraints()
{
	mInteractionScene->releaseConstraints(true);
}


PX_INLINE void Sc::Scene::clearBrokenConstraintBuffer()
{
	mBrokenConstraints.clear();
}


void Sc::Scene::updateFromVisualizationParameters()
{
	if (!mVisualizationParameterChanged) 		// All up to date
		return;

	// Update SIPs if visualization is enabled
	if (getVisualizationParameter(PxVisualizationParameter::eCONTACT_POINT) || getVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL) || 
		getVisualizationParameter(PxVisualizationParameter::eCONTACT_ERROR) || getVisualizationParameter(PxVisualizationParameter::eCONTACT_FORCE))
		mInternalFlags |= SCENE_SIP_STATES_DIRTY_VISUALIZATION;

	mVisualizationParameterChanged = false;
}


bool Sc::Scene::isValid() const
{ 
	return getInteractionScene().isValid(); 
}


void Sc::Scene::addToLostTouchList(BodySim* body1, BodySim* body2)
{
	PX_ASSERT(body1 != 0);
	PX_ASSERT(body2 != 0);
	SimpleBodyPair p = { body1, body2, body1->getID(), body2->getID() };
	mLostTouchPairs.pushBack(p);
}


void Sc::Scene::initDominanceMatrix()
{
	//init all dominance pairs such that:
	//if g1 == g2, then (1.0f, 1.0f) is returned
	//if g1 <  g2, then (0.0f, 1.0f) is returned
	//if g1 >  g2, then (1.0f, 0.0f) is returned

	PxU32 mask = ~PxU32(1);
	for (unsigned i = 0; i < PX_MAX_DOMINANCE_GROUP; ++i, mask <<= 1)
		mDominanceBitMatrix[i] = ~mask;
}

#ifdef PX_PS3
void Sc::Scene::setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value)
{
	getInteractionScene().getLowLevelContext()->setSceneParamInt(param,value);
}

PxU32 Sc::Scene::getSceneParamInt(PxPS3ConfigParam::Enum param)
{
	return getInteractionScene().getLowLevelContext()->getSceneParamInt(param);
}

void Sc::Scene::getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks)
{
	return getInteractionScene().getLowLevelContext()->getSpuMemBlockCounters(numNpContactStreamBlocks, numNpCacheBlocks, numDyFrictionBlocks, numDyConstraintBlocks);
}
#endif

#if PX_USE_PARTICLE_SYSTEM_API

void Sc::Scene::addParticleSystem(ParticleSystemCore& ps)
{
	// sim objects do all the necessary work of adding themselves to broad phase,
	// activation, registering with the interaction framework, etc

	ParticleSystemSim* psSim = PX_NEW(ParticleSystemSim)(*this, ps);

	if (!psSim)
	{
		getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Scene::addParticleSystem() failed.");
		return;
	}

	PX_ASSERT(ps.getSim());
	
	mParticleSystems.pushBack(&ps);
}


void Sc::Scene::removeParticleSystem(ParticleSystemCore& ps, bool isRelease)
{
	PxU32 i;
	for(i=0; i < mParticleSystems.size() && mParticleSystems[i] != &ps; i++)
	PX_ASSERT(i < mParticleSystems.size());

	mParticleSystems.replaceWithLast(i);
	ps.getSim()->release(isRelease);
}


PxU32 Sc::Scene::getNbParticleSystems() const
{
	return mParticleSystems.size();
}


Sc::ParticleSystemCore** Sc::Scene::getParticleSystems()
{
	return mParticleSystems.begin();
}
#endif	// PX_USE_PARTICLE_SYSTEM_API

bool Sc::Scene::hasParticleSystems() const
{
#if PX_USE_PARTICLE_SYSTEM_API
	return !mEnabledParticleSystems.empty();
#else
	return false;
#endif
}

PxSceneGpu* Sc::Scene::getSceneGpu(bool createIfNeeded)
{
#if PX_SUPPORT_GPU_PHYSX
	return getInteractionScene().getLowLevelContext()->getSceneGpu(createIfNeeded);
#else
	PX_UNUSED(createIfNeeded);
	return NULL;
#endif
}

#if PX_USE_CLOTH_API

bool Sc::Scene::addCloth(ClothCore& clothCore)
{
	// sim objects do all the necessary work of adding themselves to broad phase,
	// activation, registering with the interaction framework, etc

	cloth::Cloth* cloth = clothCore.getLowLevelCloth();
	PxU32 type = clothCore.getClothFlags() & PxClothFlag::eGPU;

	if(type)
	{
		if(cloth::Cloth* clone = mClothSolvers[type] ? mClothFactories[type]->clone(*cloth) : NULL)
		{
			clothCore.switchCloth(cloth = clone);
		}
		else
		{
			// clone failed, warn and fallback to CPU
			getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
				"GPU cloth creation failed. Falling back to CPU implementation.");
			clothCore.setClothFlag(PxClothFlag::eGPU, false);
			type = 0;
		}
	}

	mClothSolvers[type]->addCloth(cloth);
	mCloths.pushBack(&clothCore);

	PX_NEW(ClothSim)(*this, clothCore);

	return true; // always returns true, remove return value
}

void Sc::Scene::removeCloth(ClothCore& clothCore)
{
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,API,findAndReplaceWithLast);
		bool deleted = mCloths.findAndReplaceWithLast(&clothCore);
		PX_ASSERT(deleted); PX_UNUSED(deleted);
	}

	clothCore.getSim()->clearCollisionShapes();

	cloth::Cloth* cloth = clothCore.getLowLevelCloth();
	PxU32 type = cloth->getFactory().getPlatform();

	// platform and cloth flag need to match for the above to work
	PX_COMPILE_TIME_ASSERT(PxU32(cloth::Factory::CUDA) == PxU32(PxClothFlag::eGPU));

	mClothSolvers[type]->removeCloth(cloth);

	if(type)
		clothCore.switchCloth(mClothFactories[0]->clone(*cloth));

	PX_DELETE(clothCore.getSim());
}

void Sc::Scene::createClothSolver()
{
	if(*mClothSolvers) 
		return; // already called before

	if(Sc::Physics::getInstance().hasLowLevelClothFactory())
		mClothFactories[0] = &Sc::Physics::getInstance().getLowLevelClothFactory();

	if (mTaskManager && mTaskManager->getGpuDispatcher())
	{
#if PX_SUPPORT_GPU_PHYSX
		mClothFactories[PxClothFlag::eGPU] = PxvGetPhysXGpu(true)->createClothFactory(
			cloth::Factory::CUDA, mTaskManager->getGpuDispatcher()->getCudaContextManager());
#endif
	}

	PxProfileZone* profileZone = static_cast<PxProfileZone*>(mEventProfiler.getProfileEventSender());
	for(PxU32 i=0; i<mNumClothSolvers; ++i)
	{
		if(mClothFactories[i])
			mClothSolvers[i] = mClothFactories[i]->createSolver(profileZone, mTaskManager);
		if(mClothSolvers[i])
			mClothSolvers[i]->setInterCollisionFilter(Sc::DefaultClothInterCollisionFilter);
	}
}

#endif // PX_USE_CLOTH_API

bool Sc::Scene::hasCloths() const
{
#if PX_USE_CLOTH_API
	return !mCloths.empty();
#else
	return false;
#endif
}

PxU32 Sc::Scene::getNbArticulations() const
{
	return mArticulations.size();
}

Sc::ArticulationCore** Sc::Scene::getArticulations() 
{
	return mArticulations.begin();
}

PxU32 Sc::Scene::getNbConstraints() const
{
	return mConstraintArray.size();
}

Sc::ConstraintCore** Sc::Scene::getConstraints() 
{
	return mConstraintArray.begin();
}




// PX_AGGREGATE
PxU32 Sc::Scene::createAggregate(void* userData, bool selfCollisions)
{
	PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	return aabbMgr->createAggregate(userData, selfCollisions);
}

void Sc::Scene::deleteAggregate(PxU32 id)
{
	PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	aabbMgr->deleteAggregate(id);
}

//~PX_AGGREGATE
void Sc::Scene::shiftOrigin(const PxVec3& shift)
{
	//
	// adjust low level context
	//
	PxsContext* llContext = mInteractionScene->getLowLevelContext();
	llContext->shiftOrigin(shift);

	//
	// adjust broadphase
	//
	PxsAABBManager* aabbManager = llContext->getAABBManager();
	aabbManager->shiftOrigin(shift);

	//
	// adjust active transforms
	//
	Client** PX_RESTRICT clients = mClients.begin();
	const PxU32 numClients = mClients.size();
	const PxU32 prefetchLookAhead = 6;  // fits more or less into 2x128 byte prefetches

	for (PxU32 c = 0; c < numClients; c++)
	{
		PxActiveTransform* activeTransform = clients[c]->activeTransforms.begin();
		const PxU32 activeTransformCount = clients[c]->activeTransforms.size();

		PxU32 batchIterCount = activeTransformCount / prefetchLookAhead;
		
		PxU32 idx = 0;
		PxU8* prefetchPtr = ((PxU8*)activeTransform) + 256;
		for(PxU32 i=0; i < batchIterCount; i++)
		{
			Ps::prefetchLine(prefetchPtr);
			Ps::prefetchLine(prefetchPtr + 128);

			for(PxU32 j=idx; j < (idx + prefetchLookAhead); j++)
			{
				activeTransform[j].actor2World.p -= shift;
			}

			idx += prefetchLookAhead;
			prefetchPtr += 256;
		}
		// process remaining objects
		for(PxU32 i=idx; i < activeTransformCount; i++)
		{
			activeTransform[i].actor2World.p -= shift;
		}
	}

	//
	// adjust constraints
	//
	for(PxU32 i=0; i < mConstraintArray.size(); i++)
	{
		mConstraintArray[i]->getPxConnector()->onOriginShift(shift);
	}

	//
	// adjust cloth
	//
#if PX_USE_CLOTH_API
	for(PxU32 i=0; i < mCloths.size(); i++)
	{
		mCloths[i]->onOriginShift(shift);
	}
#endif

	//
	// adjust particles
	//
#if PX_USE_PARTICLE_SYSTEM_API
	PxU32 count = mParticleSystems.size();
	for(PxU32 i=0; i < count; i++)
	{
		ParticleSystemCore* ps = mParticleSystems[i];
		ps->getSim()->release(false);
		ps->onOriginShift(shift);
		ParticleSystemSim* psSim = PX_NEW(ParticleSystemSim)(*this, *ps);
		if (!psSim)
		{
			getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Scene::shiftOrigin() failed for particle system.");
		}
		PX_ASSERT(ps->getSim());
	}
#endif
}

//BROADPHASE
void Sc::Scene::addBroadPhaseVolume(const PxBounds3& bounds, const PxU32 actorGroup, const AABBMgrId& aabbMgrId, ShapeSim& shapeSim)
{
	ActorCore& actorCore = shapeSim.getRbSim().getActorCore();
	shapeSim.createLowLevelVolume(actorGroup, bounds, actorCore.mAggregateID, aabbMgrId);

#if PX_ENABLE_SIM_STATS
	mNPhaseCore->getScene().getStatsInternal().incBroadphaseAdds(PxSimulationStatistics::eRIGID_BODY);
#endif
}

void Sc::Scene::addBroadPhaseVolume(const PxBounds3& bounds, ShapeSim& shapeSim)
{
	Sc::BodySim* bodySim = shapeSim.getBodySim();
	const PxU32 group = shapeSim.getRbSim().getBroadphaseGroupId();
	const AABBMgrId aabbMgrId = bodySim ? bodySim->getLowLevelBody().getAABBMgrId() : AABBMgrId();
	return addBroadPhaseVolume(bounds, group, aabbMgrId, shapeSim);
}

#if PX_USE_PARTICLE_SYSTEM_API

void Sc::Scene::addBroadPhaseVolume(ParticlePacketShape& particleShape)
{
	PxBounds3 bounds = particleShape.getBounds();
	PX_ASSERT(bounds.minimum.isFinite());
	PX_ASSERT(bounds.maximum.isFinite());

	// Volumes of the same group do not trigger interactions.
	const PxU32 group = BP_GROUP_PARTICLES;

	ActorCore& actorCore = particleShape.getParticleSystem().getCore();
	particleShape.createLowLevelVolume(group, bounds, actorCore.mAggregateID); 

#if PX_ENABLE_SIM_STATS
	mNPhaseCore->getScene().getStatsInternal().incBroadphaseAdds(PxSimulationStatistics::ePARTICLE_SYSTEM);
#endif
}

#endif	// PX_USE_PARTICLE_SYSTEM_API

#if PX_USE_CLOTH_API

void Sc::Scene::addBroadPhaseVolume(ClothShape& clothShape)
{
	PxBounds3 bounds = clothShape.getWorldBounds();
	PX_ASSERT(bounds.minimum.isFinite());
	PX_ASSERT(bounds.maximum.isFinite());

	// Volumes of the same group do not trigger interactions.
	// TODO: Ok, these are not particles, but don't want to touch BPGroup just yet
	const PxU32 group = BP_GROUP_PARTICLES; 

    ActorCore& actorCore = clothShape.getClothSim().getActorCore();
	clothShape.createLowLevelVolume(group, bounds, actorCore.mAggregateID); 

#if PX_ENABLE_SIM_STATS
	mNPhaseCore->getScene().getStatsInternal().incBroadphaseAdds(PxSimulationStatistics::eCLOTH);
#endif
}

#endif // PX_USE_CLOTH_API

/**
  Removes a shape from the broadphase. This is called when a shape gets deleted.
 */

void Sc::Scene::removeBroadPhaseVolume(const PxU32 flags, ShapeSim& shape)
{
	mNPhaseCore->onVolumeRemoved(&shape, flags);

	shape.destroyLowLevelVolume();

#if PX_ENABLE_SIM_STATS
	mNPhaseCore->getScene().getStatsInternal().incBroadphaseRemoves(PxSimulationStatistics::eRIGID_BODY);
#endif
}


#if PX_USE_PARTICLE_SYSTEM_API
void Sc::Scene::removeBroadPhaseVolume(ParticlePacketShape& particleShape)
{
	mNPhaseCore->onVolumeRemoved(&particleShape, 0);

	particleShape.destroyLowLevelVolume();

#if PX_ENABLE_SIM_STATS
	mNPhaseCore->getScene().getStatsInternal().incBroadphaseRemoves(PxSimulationStatistics::ePARTICLE_SYSTEM);
#endif
}
#endif	// PX_USE_PARTICLE_SYSTEM_API

#if PX_USE_CLOTH_API
void Sc::Scene::removeBroadPhaseVolume(ClothShape& clothShape)
{
	mNPhaseCore->onVolumeRemoved(&clothShape, 0);

	clothShape.destroyLowLevelVolume();

#if PX_ENABLE_SIM_STATS
	mNPhaseCore->getScene().getStatsInternal().incBroadphaseRemoves(PxSimulationStatistics::eCLOTH);
#endif
}
#endif	// PX_USE_CLOTH_API

void Sc::Scene::finishBroadPhase(const PxU32 ccdPass)
{
#ifdef PX_PS3
	startTimerMarker(eFINISH_BROADPHASE);
#endif

	PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	PxU32 destroyedOverlapCount = aabbMgr->getDestroyedOverlapsCount();
	PxU32 createdOverlapCount = aabbMgr->getCreatedOverlapsCount();

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,processNewOverlaps)

		const PxvBroadPhaseOverlap* PX_RESTRICT p = aabbMgr->getCreatedOverlaps();
		mNPhaseCore->onOverlapCreated(p, createdOverlapCount, ccdPass);
		aabbMgr->freeCreatedOverlaps();
	}

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,processLostOverlaps);

		const PxvBroadPhaseOverlap* PX_RESTRICT p = aabbMgr->getDestroyedOverlaps();
		while(destroyedOverlapCount--)
		{
			Element* volume0 = (Element*)p->userdata0;
			Element* volume1 = (Element*)p->userdata1;
			mNPhaseCore->onOverlapRemoved(volume0, volume1, ccdPass);
			p++;
		}
		aabbMgr->freeDestroyedOverlaps();
	}

#ifdef PX_PS3
	stopTimerMarker(eFINISH_BROADPHASE);
#endif

}

//~BROADPHASE

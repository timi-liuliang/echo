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


#include "PxString.h"
#include "PxSimulationEventCallback.h"

#include "NpScene.h"
#include "NpRigidStatic.h"
#include "NpRigidDynamic.h"
#include "NpArticulation.h"
#include "NpArticulationLink.h"
#include "NpArticulationJoint.h"
// PX_AGGREGATE
#include "NpAggregate.h"
//~PX_AGGREGATE
#include "NpVolumeCache.h"
#include "NpBatchQuery.h"

#if PX_USE_PARTICLE_SYSTEM_API
#include "particles/NpParticleSystem.h"
#include "particles/NpParticleFluid.h"
#include "ScbParticleSystem.h"
#endif

#if PX_USE_CLOTH_API
#include "NpCloth.h"
#endif

#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PvdVisualDebugger.h"
#endif

#include "ScbNpDeps.h"
#include "CmCollection.h"

#if PX_SUPPORT_GPU_PHYSX
#include "PxGpuDispatcher.h"
#endif

using namespace physx;

#if (USE_GRB_INTEROP == 1)
#include "GrbStackAllocator.h"
#include "GrbSceneEventDescs.h"
using GrbInterop3::StackAllocator;
#endif

// enable thread checks in all debug builds
#if defined(PX_DEBUG) || defined(PX_CHECKED)
#define NP_ENABLE_THREAD_CHECKS 1
#else
#define NP_ENABLE_THREAD_CHECKS 0
#endif

///////////////////////////////////////////////////////////////////////////////


static PX_FORCE_INLINE bool removeFromSceneCheck(NpScene* npScene, PxScene* scene, const char* name)
{
	if (scene == static_cast<PxScene*>(npScene))
	{
		return true;
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "%s not assigned to scene or assigned to another scene. Call will be ignored!", name);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
NpSceneQueries::NpSceneQueries(const PxSceneDesc& desc) : 
	mScene					(desc, Cm::EventProfiler( &NpPhysics::getInstance().getProfileZone(), getPvdId() ) ),
	mSceneQueryManager		(mScene, desc)
#if PX_SUPPORT_VISUAL_DEBUGGER
	, mSingleSqCollector		(mScene, false),
	mBatchedSqCollector		(mScene, true)
#endif
{
}

NpScene::NpScene(const PxSceneDesc& desc) :
	NpSceneQueries			(desc),
	mConstraints			(PX_DEBUG_EXP("sceneConstraints")),
	mRigidActorArray		(PX_DEBUG_EXP("sceneRigidActorArray")),
	mArticulations			(PX_DEBUG_EXP("sceneArticulations")),
	mAggregates				(PX_DEBUG_EXP("sceneAggregates")),
#if PX_USE_PARTICLE_SYSTEM_API
	mPxParticleBaseArray	(PX_DEBUG_EXP("sceneParticles")),
#endif
#if PX_USE_CLOTH_API
	mPxClothArray			(PX_DEBUG_EXP("sceneCloths")),
#endif
	mSanityBounds			(desc.sanityBounds),
	mPhysicsRunning			(false),
	mIsBuffering			(desc.simulationOrder == PxSimulationOrder::eSOLVE_COLLIDE),
	mCollisionRunning		(false),	
	mNbClients				(1),			//we always have the default client.
	mClientBehaviorFlags	(PX_DEBUG_EXP("sceneBehaviorFlags")),
	mSceneCompletion		(mPhysicsDone),
	mCollisionCompletion	(mCollisionDone),
	mSceneExecution			(0, "NpScene.execution"),
	mSceneCollide			(0, "NpScene.collide"),
	mSceneSolve				(0, "NpScene.solve"),
	mControllingSimulation	(false),
	mSimThreadStackSize		(0),
	mConcurrentWriteCount	(0),
	mConcurrentReadCount	(0),
	mConcurrentErrorCount	(0),	
	mCurrentWriter			(0),
	mHasSimulated			(false)
{
	mSceneExecution.setObject(this);
	mSceneCollide.setObject(this);
	mSceneSolve.setObject(this);

	mTaskManager = mScene.getScScene().getTaskManagerPtr();
#if PX_SUPPORT_VISUAL_DEBUGGER
	PxProfileZoneManager* pzm = NpPhysics::getInstance().getProfileZoneManager();
	if (pzm)
		mTaskManager->initializeProfiling(*pzm);
#endif

#if USE_GRB_INTEROP
	mGrbEventPools = PX_NEW(GrbInterop3::PoolSet);
#endif
	mThreadReadWriteDepth = TlsAlloc();
}

NpSceneQueries::~NpSceneQueries()
{
}

NpScene::~NpScene()
{

#if USE_GRB_INTEROP
	PX_DELETE(mGrbEventPools);
#endif

	// PT: we need to do that one first, now that we don't release the objects anymore. Otherwise we end up with a sequence like:
	// - actor is part of an aggregate, and part of a scene
	// - actor gets removed from the scene. This does *not* remove it from the aggregate.
	// - aggregate gets removed from the scene, sees that one contained actor ain't in the scene => we get a warning message
	while(!mAggregates.empty())
		removeAggregate(*mAggregates[0], false);

#if PX_USE_PARTICLE_SYSTEM_API
	while(!mPxParticleBaseArray.empty())
		removeActor(*mPxParticleBaseArray[0], false);
#endif

#if PX_USE_CLOTH_API
	while(!mPxClothArray.empty())
		removeActor(*mPxClothArray[0], false);
#endif

	while(!mRigidActorArray.empty())
		removeActor(*mRigidActorArray[0], false);

	while(!mArticulations.empty())
		removeArticulation(*mArticulations[0], false);

	// release volume caches
	Array<NpVolumeCache*> caches; caches.reserve(mVolumeCaches.size());
	for(HashSet<NpVolumeCache*>::Iterator iter = mVolumeCaches.getIterator(); !iter.done(); ++iter)
		caches.pushBack(*iter);
	for(PxU32 i = 0; i < caches.size(); i++)
		releaseVolumeCache(caches[i]);

	bool unlock = mScene.getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK;

#if PX_SUPPORT_VISUAL_DEBUGGER
	getSingleSqCollector().release();
	getBatchedSqCollector().release();
#endif

	// release batch queries
	PxU32 numSq = mBatchQueries.size();
	while(numSq--)
		PX_DELETE(mBatchQueries[numSq]);
	mBatchQueries.clear();

	mScene.release();

	// unlock the lock taken in release(), must unlock before 
	// mRWLock is destroyed otherwise behavior is undefined
	if (unlock)
		unlockWrite();

	TlsFree(mThreadReadWriteDepth);
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::release()
{
	// need to acquire lock for release, note this is unlocked in the destructor
	if (mScene.getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK)
		lockWrite(__FILE__, __LINE__);

	// It will be hard to do a write check here since all object release calls in the scene destructor do it and would mess
	// up the test. If we really want it on scene destruction as well, we need to either have internal and external release
	// calls or come up with a different approach (for example using thread ID as detector variable).

	if (isPhysicsRunning() || mIsBuffering)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::release(): Scene is still being simulated! PxScene::fetchResults() is called implicitly.");
		fetchResults(true, NULL);
	}
	NpPhysics::getInstance().releaseSceneInternal(*this);
}

///////////////////////////////////////////////////////////////////////////////

PxScene* physx::NpGetPxScene(Scb::Scene& scene)
{
	char* p = reinterpret_cast<char*>(&scene);
	size_t scbOffset = reinterpret_cast<size_t>(&(reinterpret_cast<NpScene*>(0)->getScene()));
	return reinterpret_cast<NpScene*>(p - scbOffset);
}

///////////////////////////////////////////////////////////////////////////////

bool NpScene::loadFromDesc(const PxSceneDesc& desc)
{
	{
		if(desc.limits.maxNbActors)
			mRigidActorArray.reserve(desc.limits.maxNbActors);

		//const PxU32 totalNbShapes = desc.limits.maxNbStaticShapes + desc.limits.maxNbDynamicShapes;
		mScene.getScScene().preAllocate(desc.limits.maxNbActors, desc.limits.maxNbBodies, desc.limits.maxNbStaticShapes, desc.limits.maxNbDynamicShapes, desc.limits.maxNbAggregates);
	}

	userData = desc.userData;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::setGravity(const PxVec3& g)
{
	NP_WRITE_CHECK(this);
	mScene.setGravity(g);

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneSetGravity, g);
}

PxVec3 NpScene::getGravity() const
{
	NP_READ_CHECK(this);
	return mScene.getGravity();
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::setBounceThresholdVelocity(const PxReal t)
{
	NP_WRITE_CHECK(this);
	mScene.setBounceThresholdVelocity(t);
}

PxReal NpScene::getBounceThresholdVelocity() const
{
	NP_READ_CHECK(this)
	return mScene.getBounceThresholdVelocity();
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::setLimits(const PxSceneLimits& limits)
{
	NP_WRITE_CHECK(this);

	if(limits.maxNbActors)
		mRigidActorArray.reserve(limits.maxNbActors);	
	mScene.getScScene().preAllocate(limits.maxNbActors, limits.maxNbBodies, limits.maxNbStaticShapes, limits.maxNbDynamicShapes, limits.maxNbAggregates);
	mScene.setLimits(limits);

	mSceneQueryManager.preallocate(limits.maxNbStaticShapes, limits.maxNbDynamicShapes);
}

//////////////////////////////////////////////////////////////////////////

PxSceneLimits NpScene::getLimits() const
{
	NP_READ_CHECK(this);

	return mScene.getLimits();
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::setFlag(PxSceneFlag::Enum flag, bool value)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_AND_RETURN(flag & (PxU16)(	PxSceneFlag::eENABLE_CCD|PxSceneFlag::eDISABLE_CCD_RESWEEP|PxSceneFlag::eADAPTIVE_FORCE|
										PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS|PxSceneFlag::eENABLE_KINEMATIC_PAIRS|
										PxSceneFlag::eREQUIRE_RW_LOCK|PxSceneFlag::eDISABLE_CONTACT_REPORT_BUFFER_RESIZE|
										PxSceneFlag::eENABLE_PCM),"NpScene::setFlag: This flag is not mutable - you can only set it once in PxSceneDesc at startup!");

	PxSceneFlags currentFlags = mScene.getFlags();

	if(value)
		currentFlags |= flag;
	else
		currentFlags &= ~PxSceneFlags(flag);

	mScene.setFlags(currentFlags);
}

PxSceneFlags NpScene::getFlags() const
{
	NP_READ_CHECK(this);
	return mScene.getFlags();
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::addActor(PxActor& actor)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addActor);
	NP_WRITE_CHECK(this);

#ifdef PX_CHECKED
	PxRigidStatic* a = actor.is<PxRigidStatic>();
	if(a && !static_cast<NpRigidStatic*>(a)->checkConstraintValidity())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addActor(): actor has invalid constraint and may not be added to scene");
		return;
	}
#endif

	Scb::ControlState::Enum cs = NpActor::getScbFromPxActor(actor).getControlState();
	if ((cs == Scb::ControlState::eNOT_IN_SCENE) || ((cs == Scb::ControlState::eREMOVE_PENDING) && (NpActor::getOwnerScene(actor) == this)))
		addActorInternal(actor);
	else
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addActor(): Actor already assigned to a scene. Call will be ignored!");
}

void NpScene::addActorInternal(PxActor& actor)
{
	switch(actor.getConcreteType())
	{
		case PxConcreteType::eRIGID_STATIC:
		{
			NpRigidStatic& npStatic = static_cast<NpRigidStatic&>(actor);
			if(!npStatic.getShapeManager().getNbShapes())
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addActor(): Static actor with no shapes added to scene");
#ifdef PX_CHECKED
			checkPositionSanity(npStatic, npStatic.getGlobalPose(), "PxScene::addActor or PxScene::addAggregate");
#endif
			addRigidStatic(npStatic);

			GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneAddRigidStatic, &npStatic);
		}
		break;

		case PxConcreteType::eRIGID_DYNAMIC:
		{
			NpRigidDynamic& npDynamic = static_cast<NpRigidDynamic&>(actor);
#ifdef PX_CHECKED
			checkPositionSanity(npDynamic, npDynamic.getGlobalPose(), "PxScene::addActor or PxScene::addAggregate");
#endif
			addRigidDynamic(npDynamic);

			GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneAddRigidDynamic, &npDynamic);
		}
		break;

#if PX_USE_PARTICLE_SYSTEM_API
		case PxConcreteType::ePARTICLE_SYSTEM:
		{
			NpParticleSystem& npSystem = static_cast<NpParticleSystem&>(actor);
			addParticleSystem(npSystem);
		}
		break;

		case PxConcreteType::ePARTICLE_FLUID:
		{
			NpParticleFluid& npFluid = static_cast<NpParticleFluid&>(actor);
			addParticleFluid(npFluid);
		}
		break;
#endif

#if PX_USE_CLOTH_API
		case PxConcreteType::eCLOTH:
		{
			NpCloth& npCloth = static_cast<NpCloth&>(actor);
			addCloth(npCloth);
		}
		break;
#endif
		case PxConcreteType::eARTICULATION_LINK:
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addActor(): Individual articulation links can not be added to the scene");
		}
		break;

		default:
			PX_ASSERT(0);
	}
}


void NpScene::updateScbStateAndSetupSq(const PxRigidActor& rigidActor, Scb::Actor& scbActor, NpShapeManager& shapeManager, bool actorDynamic, PxBounds3* bounds)
{
	// all the things Scb does in non-buffered insertion
	Sq::SceneQueryManager& sqManager = getSceneQueryManagerFast();

	scbActor.setScbScene(&getScene());
	scbActor.setControlState(Scb::ControlState::eIN_SCENE);
	NpShape*const * shapes = shapeManager.getShapes();
	PxU32 nbShapes = shapeManager.getNbShapes();

	for(PxU32 i=0;i<nbShapes;i++)
	{
		NpShape& shape = *shapes[i];
		const PxShapeFlags& shapeFlags = shape.getFlagsUnbuffered();

		shape.incRefCount();
		if(shape.NpShape::isExclusive())
		{
			shape.getScbShape().setScbScene(&getScene());
			shape.getScbShape().setControlState(Scb::ControlState::eIN_SCENE);
		}

		if(shapeFlags & PxShapeFlag::eSCENE_QUERY_SHAPE)
		{
			Sq::ActorShape* data = sqManager.addShape(*shapes[i], rigidActor, actorDynamic, shapeFlags&(PxShapeFlag::eSIMULATION_SHAPE|PxShapeFlag::eTRIGGER_SHAPE)? bounds+i : NULL);
			shapeManager.setSceneQueryData(i, data);
		}
	}			
}


PX_FORCE_INLINE	void NpScene::updateScbStateAndSetupSq(const PxRigidActor& rigidActor, Scb::Body& body, NpShapeManager& shapeManager, bool actorDynamic, PxBounds3* bounds)
{
	body.initBufferedState();
	updateScbStateAndSetupSq(rigidActor, static_cast<Scb::Actor&>(body), shapeManager, actorDynamic, bounds);
}


void NpScene::addActors(PxActor*const* PX_RESTRICT actors, PxU32 nbActors)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addActors);
	NP_WRITE_CHECK(this);	

	if (mPhysicsRunning) 
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxScene::addActors() not allowed while simulation is running.");
		return;
	}


	Sc::Scene& scScene = mScene.getScScene();

	PxU32 actorsDone;

	Sc::BatchInsertionState scState;
	scScene.startBatchInsertion(scState);

	scState.staticActorOffset		= ptrdiff_t(size_t(&(reinterpret_cast<NpRigidStatic*>(0)->getScbRigidStaticFast().getScStatic())));
	scState.staticShapeTableOffset	= ptrdiff_t(size_t(&(reinterpret_cast<NpRigidStatic*>(0)->getShapeManager().getShapeTable())));
	scState.dynamicActorOffset		= ptrdiff_t(size_t(&(reinterpret_cast<NpRigidDynamic*>(0)->getScbBodyFast().getScBody())));
	scState.dynamicShapeTableOffset = ptrdiff_t(size_t(&(reinterpret_cast<NpRigidDynamic*>(0)->getShapeManager().getShapeTable())));
	scState.shapeOffset				= (ptrdiff_t)NpShapeGetScPtrOffset();

	Ps::InlineArray<PxBounds3, 8> shapeBounds;
	for(actorsDone=0; actorsDone<nbActors; actorsDone++)
	{
		if(actorsDone+1<nbActors)
			Ps::prefetch(actors[actorsDone+1], sizeof(NpRigidDynamic));	// worst case: PxRigidStatic is smaller

		PxType type = actors[actorsDone]->getConcreteType();
		Scb::ControlState::Enum cs = NpActor::getScbFromPxActor(*actors[actorsDone]).getControlState();
		if (!((cs == Scb::ControlState::eNOT_IN_SCENE) || ((cs == Scb::ControlState::eREMOVE_PENDING) && (NpActor::getOwnerScene(*actors[actorsDone]) == this))))
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addActors(): Actor already assigned to a scene. Call will be ignored!");
			break;
		}

		if(type == PxConcreteType::eRIGID_STATIC)
		{
			NpRigidStatic& a = *static_cast<NpRigidStatic*>(actors[actorsDone]);
#ifdef PX_CHECKED
			if(!a.checkConstraintValidity())
			{
				Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addActors(): actor has invalid constraint and may not be added to scene");
				break;
			}

			checkPositionSanity(a, a.getGlobalPose(), "PxScene::addActors");
#endif
			if (!(a.getScbRigidStaticFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION))
			{
				shapeBounds.resizeUninitialized(a.NpRigidStatic::getNbShapes());
				scScene.addStatic(&a, scState, shapeBounds.begin());
				updateScbStateAndSetupSq(a, a.getScbActorFast(), a.getShapeManager(), false, shapeBounds.begin());
				a.setRigidActorArrayIndex(mRigidActorArray.size());
				mRigidActorArray.pushBack(&a);
				a.addConstraintsToScene();
			}
			else
				addRigidStatic(a);

		}
		else if(type == PxConcreteType::eRIGID_DYNAMIC)
		{
			NpRigidDynamic& a = *static_cast<NpRigidDynamic*>(actors[actorsDone]);
#ifdef PX_CHECKED
			checkPositionSanity(a, a.getGlobalPose(), "PxScene::addActors");
#endif
			if (!(a.getScbBodyFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION))
			{
				shapeBounds.resizeUninitialized(a.NpRigidDynamic::getNbShapes());
				scScene.addBody(&a, scState, shapeBounds.begin());
				updateScbStateAndSetupSq(a, a.getScbBodyFast(), a.getShapeManager(), true, shapeBounds.begin());
				a.setRigidActorArrayIndex(mRigidActorArray.size());
				mRigidActorArray.pushBack(&a);
				a.addConstraintsToScene();
			}
			else
				addRigidDynamic(a);
		}
		else if(type == PxConcreteType::eCLOTH || type == PxConcreteType::ePARTICLE_SYSTEM || type == PxConcreteType::ePARTICLE_FLUID)
		{
			addActorInternal(*actors[actorsDone]);
		}
		else
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addRigidActors(): articulation link not permitted");
			break;
		}

	}
	scScene.finishBatchInsertion(scState);

	// if we failed, still complete everything for the successful inserted actors before backing out	
#if PX_SUPPORT_VISUAL_DEBUGGER
	for(PxU32 i=0;i<actorsDone;i++)
	{
		if ((actors[i]->getConcreteType()==PxConcreteType::eRIGID_STATIC) && (!(static_cast<NpRigidStatic*>(actors[i])->getScbRigidStaticFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION)))
			mScene.addStaticAndShapesToPvd(static_cast<NpRigidStatic*>(actors[i])->getScbRigidStaticFast());
		else if ((actors[i]->getConcreteType() == PxConcreteType::eRIGID_DYNAMIC) && (!(static_cast<NpRigidDynamic*>(actors[i])->getScbBodyFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION)))
			mScene.addBodyAndShapesToPvd(static_cast<NpRigidDynamic*>(actors[i])->getScbBodyFast());
	}
#endif

	if(actorsDone<nbActors)	// Everything is consistent up to the failure point, so just use removeActor to back out gracefully if necessary
	{
		for(PxU32 j=0;j<actorsDone;j++)
			removeActorInternal(*actors[j], false, true);
		return;
	}

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneAddRigidActors, actors, nbActors);	
}


///////////////////////////////////////////////////////////////////////////////

void NpScene::removeActors(PxActor*const* PX_RESTRICT actors, PxU32 nbActors, bool wakeOnLostTouch)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeActors);
	NP_WRITE_CHECK(this);	
	
	Sc::Scene& scScene = mScene.getScScene();
	// resize the bitmap so it does not allocate each remove actor call
	scScene.resizeReleasedBodyIDMaps(mRigidActorArray.size(),nbActors);
	Sc::BatchRemoveState removeState;
	scScene.setBatchRemove(&removeState);
	 
	for(PxU32 actorsDone=0; actorsDone<nbActors; actorsDone++)
	{
		if(actorsDone+1<nbActors)
			Ps::prefetch(actors[actorsDone+1], sizeof(NpRigidDynamic));	// worst case: PxRigidStatic is smaller

		PxType type = actors[actorsDone]->getConcreteType();
		if (!removeFromSceneCheck(this, actors[actorsDone]->getScene(), "PxScene::removeActors(): Actor"))
		{			
			break;
		}
					
		removeState.bufferedShapes.clear();
		removeState.removedShapes.clear();		

		if(type == PxConcreteType::eRIGID_STATIC)
		{			
			NpRigidStatic& actor = *static_cast<NpRigidStatic*>(actors[actorsDone]);
			const PxActorFlags actorFlags = actor.getActorFlags();
			if(actor.getShapeManager().getNbShapes())
				Ps::prefetch(actor.getShapeManager().getShapes()[0],sizeof(NpShape));
			scScene.prefetchForRemove(actor.getScbRigidStaticFast().getScStatic());
			Ps::prefetch(mRigidActorArray[mRigidActorArray.size()-1],sizeof(NpRigidDynamic));

			bool noSimBuffered = actorFlags.isSet(PxActorFlag::eDISABLE_SIMULATION);
			if (!noSimBuffered)
				actor.removeConstraintsFromScene();

			actor.getShapeManager().teardownAllSceneQuery(getSceneQueryManagerFast());

			Scb::RigidStatic& rs = actor.getScbRigidStaticFast();
			mScene.removeRigidStatic(rs, wakeOnLostTouch, rs.isSimDisabledInternally());

			removeFromRigidActorList(actor.getRigidActorArrayIndex());
		}
		else if(type == PxConcreteType::eRIGID_DYNAMIC)
		{			
			NpRigidDynamic& actor = *static_cast<NpRigidDynamic*>(actors[actorsDone]);	
			const PxActorFlags actorFlags = actor.getActorFlags();
			if(actor.getShapeManager().getNbShapes())
				Ps::prefetch(actor.getShapeManager().getShapes()[0],sizeof(NpShape));
			scScene.prefetchForRemove(actor.getScbBodyFast().getScBody());	
			Ps::prefetch(mRigidActorArray[mRigidActorArray.size()-1],sizeof(NpRigidDynamic));

			bool noSimBuffered = actorFlags.isSet(PxActorFlag::eDISABLE_SIMULATION);			
			if (!noSimBuffered)
				actor.removeConstraintsFromScene();

			actor.getShapeManager().teardownAllSceneQuery(getSceneQueryManagerFast());

			Scb::Body& b = actor.getScbBodyFast();
			mScene.removeRigidBody(b, wakeOnLostTouch, b.isSimDisabledInternally());

			removeFromRigidActorList(actor.getRigidActorArrayIndex());
		}
		else if(type == PxConcreteType::eCLOTH || type == PxConcreteType::ePARTICLE_SYSTEM || type == PxConcreteType::ePARTICLE_FLUID)
		{
			removeActorInternal(*actors[actorsDone],wakeOnLostTouch, true);
		}
		else
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::removeActor(): Individual articulation links can not be removed from the scene");
			break;
		}
	}	

	scScene.setBatchRemove(NULL);
}

void NpScene::removeActor(PxActor& actor, bool wakeOnLostTouch)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeActor);
	NP_WRITE_CHECK(this);	
	if (removeFromSceneCheck(this, actor.getScene(), "PxScene::removeActor(): Actor"))
	{
		removeActorInternal(actor, wakeOnLostTouch, true);
	}
}

void NpScene::removeActorInternal(PxActor& actor, bool wakeOnLostTouch, bool removeFromAggregate)
{
	switch(actor.getType())
	{
		case PxActorType::eRIGID_STATIC:
		{
			NpRigidStatic& npStatic = static_cast<NpRigidStatic&>(actor);
			removeRigidStatic(npStatic, wakeOnLostTouch, removeFromAggregate);
		}
		break;

		case PxActorType::eRIGID_DYNAMIC:
		{
			NpRigidDynamic& npDynamic = static_cast<NpRigidDynamic&>(actor);
			removeRigidDynamic(npDynamic, wakeOnLostTouch, removeFromAggregate);
		}
		break;
#if PX_USE_PARTICLE_SYSTEM_API
		case PxActorType::ePARTICLE_SYSTEM:
		{
			NpParticleSystem& npSystem = static_cast<NpParticleSystem&>(actor);
			removeParticleSystem(npSystem);
		}
		break;

		case PxActorType::ePARTICLE_FLUID:
		{
			NpParticleFluid& npFluid = static_cast<NpParticleFluid&>(actor);
			removeParticleFluid(npFluid);
		}
		break;
#endif

#if PX_USE_CLOTH_API
		case PxActorType::eCLOTH:
		{
			NpCloth& npCloth = static_cast<NpCloth&>(actor);
			removeCloth(npCloth);
		}
		break;
#endif

		case PxActorType::eARTICULATION_LINK:
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::removeActor(): Individual articulation links can not be removed from the scene");
		}
		break;
		
		case PxActorType::eACTOR_COUNT:
		case PxActorType::eACTOR_FORCE_DWORD:
		default:
			PX_ASSERT(0);
	}
}

///////////////////////////////////////////////////////////////////////////////

// PT: TODO: inline this one in the header for consistency
void NpScene::removeFromRigidActorList(const PxU32& index)
{
	PX_ASSERT(index != 0xFFFFFFFF);
	PX_ASSERT(index < mRigidActorArray.size());

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneRemoveActor, mRigidActorArray[index]);

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene, API, findAndReplaceWithLast);		
		PxU32 size = mRigidActorArray.size() - 1;
		mRigidActorArray.replaceWithLast(index);
		if(size && size != index)
		{
			PxRigidActor& rigidActor = *mRigidActorArray[index];
			switch(rigidActor.getType())
			{
			case PxActorType::eRIGID_STATIC:
				{
					NpRigidStatic& npStatic = static_cast<NpRigidStatic&>(rigidActor);
					npStatic.setRigidActorArrayIndex(index);
				}
				break;
			case PxActorType::eRIGID_DYNAMIC:
				{
					NpRigidDynamic& npDynamic = static_cast<NpRigidDynamic&>(rigidActor);
					npDynamic.setRigidActorArrayIndex(index);			}
				break;

#if PX_USE_CLOTH_API
			case PxActorType::eCLOTH:
#endif
#if PX_USE_PARTICLE_SYSTEM_API
			case PxActorType::ePARTICLE_FLUID:
			case PxActorType::ePARTICLE_SYSTEM:
#endif
			case PxActorType::eARTICULATION_LINK:
			case PxActorType::eACTOR_COUNT:
			case PxActorType::eACTOR_FORCE_DWORD:
			default:
				PX_ASSERT(0);
				break;
			}
		}
	}
}

void NpScene::addRigidStatic(NpRigidStatic& actor)
{
	bool noSimBuffered = actor.getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION);

	mScene.addRigidStatic(actor.getScbRigidStaticFast(), noSimBuffered);

	actor.getShapeManager().setupAllSceneQuery(actor);
	if (!noSimBuffered)
		actor.addConstraintsToScene();
	actor.setRigidActorArrayIndex(mRigidActorArray.size());
	mRigidActorArray.pushBack(&actor);	
}

void NpScene::removeRigidStatic(NpRigidStatic& actor, bool wakeOnLostTouch, bool removeFromAggregate)
{
	PX_ASSERT(NpActor::getAPIScene(actor) == this);
	bool noSimBuffered = actor.getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION);

	if(removeFromAggregate && actor.getAggregate())
	{
		((NpAggregate*)actor.getAggregate())->removeActorAndReinsert(actor, false);
		PX_ASSERT(!actor.getAggregate());
	}

	actor.getShapeManager().teardownAllSceneQuery(getSceneQueryManagerFast());
	if (!noSimBuffered)
		actor.removeConstraintsFromScene();

	Scb::RigidStatic& rs = actor.getScbRigidStaticFast();
	mScene.removeRigidStatic(rs, wakeOnLostTouch, rs.isSimDisabledInternally());

	removeFromRigidActorList(actor.getRigidActorArrayIndex());
}

void NpScene::addRigidDynamic(NpRigidDynamic& body)
{
	bool noSimBuffered = body.getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION);

	mScene.addRigidBody(body.getScbBodyFast(), noSimBuffered);

	body.getShapeManager().setupAllSceneQuery(body);
	if (!noSimBuffered)
		body.addConstraintsToScene();
	body.setRigidActorArrayIndex(mRigidActorArray.size());
	mRigidActorArray.pushBack(&body);
}

void NpScene::removeRigidDynamic(NpRigidDynamic& body, bool wakeOnLostTouch, bool removeFromAggregate)
{
	PX_ASSERT(NpActor::getAPIScene(body) == this);
	bool noSimBuffered = body.getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION);

	if(removeFromAggregate && body.getAggregate())
	{
		((NpAggregate*)body.getAggregate())->removeActorAndReinsert(body, false);
		PX_ASSERT(!body.getAggregate());
	}

	body.getShapeManager().teardownAllSceneQuery(getSceneQueryManagerFast());

	if (!noSimBuffered)
		body.removeConstraintsFromScene();

	Scb::Body& b = body.getScbBodyFast();
	mScene.removeRigidBody(b, wakeOnLostTouch, b.isSimDisabledInternally());

	removeFromRigidActorList(body.getRigidActorArrayIndex());
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::addArticulation(PxArticulation& articulation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addArticulation);
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN(articulation.getNbLinks()>0, "PxScene::addArticulation: empty articulations may not be added to simulation.");

	Scb::Articulation& art = static_cast<NpArticulation&>(articulation).getArticulation(); 
	Scb::ControlState::Enum cs = art.getControlState();
	if ((cs == Scb::ControlState::eNOT_IN_SCENE) || ((cs == Scb::ControlState::eREMOVE_PENDING) && (art.getScbScene()->getPxScene() == this)))
		addArticulationInternal(articulation);
	else
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addArticulation(): Articulation already assigned to a scene. Call will be ignored!");
}

void NpScene::addArticulationInternal(PxArticulation& articulation)
{
	NpArticulation& npa = static_cast<NpArticulation&>(articulation);

	// Add root link first
	PxU32 nbLinks = npa.getNbLinks();
	PX_ASSERT(nbLinks > 0);
	NpArticulationLink* rootLink = npa.getLinks()[0];

#ifdef PX_CHECKED
	checkPositionSanity(*rootLink, rootLink->getGlobalPose(), "PxScene::addArticulation or PxScene::addAggregate");
#endif
	if(rootLink->getMass()==0)
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addArticulation(): Articulation link with zero mass added to scene; defaulting mass to 1");
		rootLink->setMass(1.0f);
	}

	PxVec3 inertia0 = rootLink->getMassSpaceInertiaTensor();		
	if(inertia0.x == 0.0f || inertia0.y == 0.0f || inertia0.z == 0.0f)
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addArticulation(): Articulation link with zero moment of inertia added to scene; defaulting inertia to (1,1,1)");
		rootLink->setMassSpaceInertiaTensor(PxVec3(1.0f, 1.0f, 1.0f));
	}

	bool linkTriggersWakeUp = !rootLink->getScbBodyFast().checkSleepReadinessBesidesWakeCounter();


	addArticulationLinkBody(*rootLink);

	// Add articulation
	Scb::Articulation& scbArt = npa.getArticulation();
	mScene.addArticulation(scbArt);

	addArticulationLinkConstraint(*rootLink);
	
	// Add links & joints
	PX_ALLOCA(linkStack, NpArticulationLink*, nbLinks);
	linkStack[0] = rootLink;
	PxU32 curLink = 0;
	PxU32 stackSize = 1;
	while(curLink < (nbLinks-1))
	{
		PX_ASSERT(curLink < stackSize);
		NpArticulationLink* l = linkStack[curLink];
		NpArticulationLink*const* children = l->getChildren();

		for(PxU32 i=0; i < l->getNbChildren(); i++)
		{
			NpArticulationLink* child = children[i];

#ifdef PX_CHECKED
			checkPositionSanity(*rootLink, rootLink->getGlobalPose(), "PxScene::addArticulation");
#endif
			if(child->getMass()==0)
			{
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addArticulation(): Articulation link with zero mass added to scene; defaulting mass to 1");
				child->setMass(1.0f);
			}

			PxVec3 inertia = child->getMassSpaceInertiaTensor();		
			if(inertia.x == 0.0f || inertia.y == 0.0f || inertia.z == 0.0f)
			{
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addArticulation(): Articulation link with zero moment of inertia added to scene; defaulting inertia to (1,1,1)");
				child->setMassSpaceInertiaTensor(PxVec3(1.0f, 1.0f, 1.0f));
			}

			linkTriggersWakeUp = linkTriggersWakeUp || (!child->getScbBodyFast().checkSleepReadinessBesidesWakeCounter());

			addArticulationLink(*child);  // Adds joint too

			linkStack[stackSize] = child;
			stackSize++;
		}

		curLink++;
	}

	if ((scbArt.getWakeCounter() == 0.0f) && linkTriggersWakeUp)
	{
		// this is for the buffered insert case, where the articulation needs to wake up, if one of the links triggers activation.
		npa.wakeUpInternal(true, false);
	}

	mArticulations.pushBack(&npa);
}

void NpScene::removeArticulation(PxArticulation& articulation, bool wakeOnLostTouch)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeArticulation);
	NP_WRITE_CHECK(this);

	if (removeFromSceneCheck(this, articulation.getScene(), "PxScene::removeArticulation(): Articulation"))
	{
		removeArticulationInternal(articulation, wakeOnLostTouch, true);
	}
}

void NpScene::removeArticulationInternal(PxArticulation& articulation, bool wakeOnLostTouch,  bool removeFromAggregate)
{

	NpArticulation& npa = static_cast<NpArticulation&>(articulation);

	PxU32 nbLinks = npa.getNbLinks();
	PX_ASSERT(nbLinks > 0);

	if(removeFromAggregate && articulation.getAggregate())
	{
		((NpAggregate*)articulation.getAggregate())->removeArticulationAndReinsert(articulation, false);
		PX_ASSERT(!articulation.getAggregate());
	}

	//!!!AL
	// Inefficient. We might want to introduce a LL method to kill the whole LL articulation together with all joints in one go, then
	// the order of removing the links/joints does not matter anymore.

	// Remove links & joints
	PX_ALLOCA(linkStack, NpArticulationLink*, nbLinks);
	linkStack[0] = npa.getLinks()[0];
	PxU32 curLink = 0, stackSize = 1;

	while(curLink < (nbLinks-1))
	{
		PX_ASSERT(curLink < stackSize);
		NpArticulationLink* l = linkStack[curLink];
		NpArticulationLink*const* children = l->getChildren();

		for(PxU32 i=0; i < l->getNbChildren(); i++)
		{
			linkStack[stackSize] = children[i];
			stackSize++;
		}

		curLink++;
	}

	for(PxI32 j=(PxI32)nbLinks; j-- > 0; )
	{
		removeArticulationLink(*linkStack[j], wakeOnLostTouch);
	}

	// Remove articulation
	mScene.removeArticulation(npa.getArticulation());


	removeFromArticulationList(articulation);
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::addArticulationLinkBody(NpArticulationLink& link)
{
	mScene.addRigidBody(link.getScbBodyFast(), false);
	link.getShapeManager().setupAllSceneQuery(link);
}

void NpScene::addArticulationLinkConstraint(NpArticulationLink& link)
{
	NpArticulationJoint* j = static_cast<NpArticulationJoint*>(link.getInboundJoint());
	if (j)
		mScene.addArticulationJoint(j->getScbArticulationJoint());

	link.addConstraintsToScene();
}

void NpScene::addArticulationLink(NpArticulationLink& link)
{
	addArticulationLinkBody(link);
	addArticulationLinkConstraint(link);
}

void NpScene::removeArticulationLink(NpArticulationLink& link, bool wakeOnLostTouch)
{
	NpArticulationJoint* j = static_cast<NpArticulationJoint*>(link.getInboundJoint());

	link.removeConstraintsFromScene();
	link.getShapeManager().teardownAllSceneQuery(getSceneQueryManagerFast());

	if (j)
		mScene.removeArticulationJoint(j->getScbArticulationJoint());

	mScene.removeRigidBody(link.getScbBodyFast(), wakeOnLostTouch, false);
}

///////////////////////////////////////////////////////////////////////////////

// PX_AGGREGATE

void NpScene::addAggregate(PxAggregate& aggregate)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addAggregate);
	NP_WRITE_CHECK(this);

	NpAggregate& np = static_cast<NpAggregate&>(aggregate);

	const PxU32 nb = np.getCurrentSizeFast();
#ifdef PX_CHECKED
	for(PxU32 i=0;i<nb;i++)
	{
		PxRigidStatic* a = np.getActorFast(i)->is<PxRigidStatic>();
		if(a && !static_cast<NpRigidStatic*>(a)->checkConstraintValidity())
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addAggregate(): Aggregate contains an actor with an invalid constraint!");
			return;
		}
	}	
#endif

	Scb::Aggregate& agg = np.getScbAggregate(); 
	Scb::ControlState::Enum cs = agg.getControlState();
	if ((cs == Scb::ControlState::eNOT_IN_SCENE) || ((cs == Scb::ControlState::eREMOVE_PENDING) && (agg.getScbScene()->getPxScene() == this)))
	{
		mScene.addAggregate(agg);

		for(PxU32 i=0;i<nb;i++)
		{
			PX_ASSERT(np.getActorFast(i));
			np.addActorInternal(*np.getActorFast(i), *this);
		}

		mAggregates.pushBack(&aggregate);

		GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneAddAggregate, &aggregate);
	}
	else
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::addAggregate(): Aggregate already assigned to a scene. Call will be ignored!");
}

void NpScene::removeAggregate(PxAggregate& aggregate, bool wakeOnLostTouch)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeAggregate);
	NP_WRITE_CHECK(this);	
	if(!removeFromSceneCheck(this, aggregate.getScene(), "PxScene::removeAggregate(): Aggregate"))
		return;

	NpAggregate& np = static_cast<NpAggregate&>(aggregate);
	if(np.getScene()!=this)
		return;

	const PxU32 nb = np.getCurrentSizeFast();
	for(PxU32 j=0;j<nb;j++)
	{
		PxActor* a = np.getActorFast(j);
		PX_ASSERT(a);

		if (a->getType() != PxActorType::eARTICULATION_LINK)
		{
			Scb::Actor& scb = NpActor::getScbFromPxActor(*a);

			np.getScbAggregate().removeActor(scb, false);  // This is only here to make sure the aggregateID gets set to invalid on sync

			removeActorInternal(*a, wakeOnLostTouch, false);
		}
		else if (a->getScene())
		{
			NpArticulationLink& al = static_cast<NpArticulationLink&>(*a);
			NpArticulation& npArt = al.getRoot();
			NpArticulationLink* const* links = npArt.getLinks();
			for(PxU32 i=0; i < npArt.getNbLinks(); i++)
			{
				np.getScbAggregate().removeActor(links[i]->getScbActorFast(), false);  // This is only here to make sure the aggregateID gets set to invalid on sync
			}

			removeArticulationInternal(npArt, wakeOnLostTouch, false);
		}
	}

	mScene.removeAggregate(np.getScbAggregate());

	removeFromAggregateList(aggregate);

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneRemoveAggregate, &aggregate, 1);
}

PxU32 NpScene::getNbAggregates() const
{
	NP_READ_CHECK(this);
	return mAggregates.size();
}

PxU32 NpScene::getAggregates(PxAggregate** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(this);
	const PxU32 size = mAggregates.size();

	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = PxMin(remainder, bufferSize);
	for(PxU32 i=0; i<writeCount; i++)
		buffer[i] = mAggregates[i+startIndex];

	return writeCount;
}

//~PX_AGGREGATE

void NpScene::addCollection(const PxCollection& collection)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addCollection);
	const Cm::Collection& col = static_cast<const Cm::Collection&>(collection);

	PxU32 nb = col.internalGetNbObjects();
#ifdef PX_CHECKED
	for(PxU32 i=0;i<nb;i++)
	{
		PxRigidStatic* a = col.internalGetObject(i)->is<PxRigidStatic>();
		if(a && !static_cast<NpRigidStatic*>(a)->checkConstraintValidity())
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "NpScene::addCollection(): collection contains an actor with an invalid constraint!");
			return;
		}
	}	
#endif



	struct Local
	{
		static void addActorIfNeeded(NpScene& scene, PxActor* actor)
		{
			PxAggregate* aggregate = actor->getAggregate();
			if(aggregate)
				return;	// The actor will be added when the aggregate is added

			scene.addActor(*actor);
		}
	};

	for(PxU32 i=0;i<nb;i++)
	{
		PxBase* s = col.internalGetObject(i);
		const PxType serialType = s->getConcreteType();

		//NpArticulationLink, NpArticulationJoint are added with the NpArticulation
		//Actors and Articulations that are members of an Aggregate are added with the NpAggregate

		if(serialType==PxConcreteType::eRIGID_DYNAMIC)
		{
			NpRigidDynamic* np = static_cast<NpRigidDynamic*>(s);
			Local::addActorIfNeeded(*this, np);
		}
		else if(serialType==PxConcreteType::eRIGID_STATIC)
		{
			NpRigidStatic* np = static_cast<NpRigidStatic*>(s);
			Local::addActorIfNeeded(*this, np);
		}
		else if(serialType==PxConcreteType::eSHAPE)
		{			
		}
#if PX_USE_CLOTH_API
		else if (serialType==PxConcreteType::eCLOTH)
		{
			NpCloth* np = static_cast<NpCloth*>(s);
			Local::addActorIfNeeded(*this, np);
		}
#endif
#if PX_USE_PARTICLE_SYSTEM_API
		else if(serialType==PxConcreteType::ePARTICLE_SYSTEM)
		{
			NpParticleSystem* np = static_cast<NpParticleSystem*>(s);
			Local::addActorIfNeeded(*this, np);
		}
		else if(serialType==PxConcreteType::ePARTICLE_FLUID)
		{
			NpParticleFluid* np = static_cast<NpParticleFluid*>(s);
			Local::addActorIfNeeded(*this, np);
		}
#endif
		else if(serialType==PxConcreteType::eARTICULATION)
		{
			NpArticulation* np = static_cast<NpArticulation*>(s);
			if(!np->getAggregate()) // The actor will be added when the aggregate is added
				addArticulation(*np);
		}
		else if(serialType==PxConcreteType::eAGGREGATE)
		{
			NpAggregate* np = static_cast<NpAggregate*>(s);
			addAggregate(*np);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

PxU32 NpScene::getNbActors(PxActorTypeFlags types) const
{
	NP_READ_CHECK(this);
	PxU32 nbActors = 0;

	if (types & PxActorTypeFlag::eRIGID_STATIC)
	{
		for(PxU32 i=mRigidActorArray.size(); i--;)
		{
			if (mRigidActorArray[i]->is<PxRigidStatic>())
				nbActors++;
		}
	}

	if (types & PxActorTypeFlag::eRIGID_DYNAMIC)
	{
		for(PxU32 i=mRigidActorArray.size(); i--;)
		{
			if (mRigidActorArray[i]->is<PxRigidDynamic>())
				nbActors++;
		}
	}
#if PX_USE_PARTICLE_SYSTEM_API
	if (types & PxActorTypeFlag::ePARTICLE_SYSTEM)
	{
		for(PxU32 i=0; i < mPxParticleBaseArray.size(); i++)
		{
			if (mPxParticleBaseArray[i]->is<PxParticleSystem>())
				nbActors++;
		}
	}

	if (types & PxActorTypeFlag::ePARTICLE_FLUID)
	{
		for(PxU32 i=0; i < mPxParticleBaseArray.size(); i++)
		{
			if (mPxParticleBaseArray[i]->is<PxParticleFluid>())
				nbActors++;
		}
	}
#endif

#if PX_USE_CLOTH_API
	if (types & PxActorTypeFlag::eCLOTH)
	{
		nbActors += mPxClothArray.size();
	}
#endif

	return nbActors;
}

PxU32 NpScene::getActors(PxActorTypeFlags types, PxActor** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(this);

	PxU32 writeCount = 0;
	PxU32 virtualIndex = 0;	// PT: virtual index of actor, continuous across different actor containers.

	if(types & (PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC))
	{
		const PxU32 size = mRigidActorArray.size();
		for(PxU32 i=0; (i < size) && (writeCount < bufferSize); i++)
		{
			if ((types & PxActorTypeFlag::eRIGID_STATIC ) && mRigidActorArray[i]->is<PxRigidStatic>())
			{
				if (virtualIndex >= startIndex)
					buffer[writeCount++] = mRigidActorArray[i];
				virtualIndex++;
			}
			else if ((types & PxActorTypeFlag::eRIGID_DYNAMIC) && mRigidActorArray[i]->is<PxRigidDynamic>())
			{
				if (virtualIndex >= startIndex)
					buffer[writeCount++] = mRigidActorArray[i];
				virtualIndex++;
			}
		}
	}

#if PX_USE_PARTICLE_SYSTEM_API
	if (types & (PxActorTypeFlag::ePARTICLE_SYSTEM | PxActorTypeFlag::ePARTICLE_FLUID))
	{
		const PxU32 size = mPxParticleBaseArray.size();
		for(PxU32 i=0; (i < size) && (writeCount < bufferSize); i++)
		{
			if ((types & PxActorTypeFlag::ePARTICLE_SYSTEM ) && mPxParticleBaseArray[i]->is<PxParticleSystem>())
			{
				if (virtualIndex >= startIndex)
					buffer[writeCount++] = mPxParticleBaseArray[i];
				virtualIndex++;
			}
			else if ((types & PxActorTypeFlag::ePARTICLE_FLUID) && mPxParticleBaseArray[i]->is<PxParticleFluid>())
			{
				if (virtualIndex >= startIndex)
					buffer[writeCount++] = mPxParticleBaseArray[i];
				virtualIndex++;
			}
		}
	}
	
#endif

#if PX_USE_CLOTH_API
	if (types & PxActorTypeFlag::eCLOTH)
	{
		const PxU32 size = mPxClothArray.size();
		for(PxU32 i=0; (i < size) && (writeCount < bufferSize); i++)
		{
			if(virtualIndex>=startIndex)
				buffer[writeCount++] = mPxClothArray[i];
			virtualIndex++;
		}
	}
#endif

	return writeCount;
}

///////////////////////////////////////////////////////////////////////////////

const PxActiveTransform* NpScene::getActiveTransforms(PxU32& nbTransformsOut, PxClientID client)
{
	NP_READ_CHECK(this);
	return mScene.getActiveTransforms(nbTransformsOut, client);
}

///////////////////////////////////////////////////////////////////////////////

PxU32 NpScene::getNbArticulations() const
{
	NP_READ_CHECK(this);
	return mArticulations.size();
}

PxU32 NpScene::getArticulations(PxArticulation** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(this);
	const PxU32 size = mArticulations.size();

	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = PxMin(remainder, bufferSize);
	for(PxU32 i=0; i<writeCount; i++)
		buffer[i] = mArticulations[i+startIndex];

	return writeCount;
}

///////////////////////////////////////////////////////////////////////////////

PxU32 NpScene::getNbConstraints() const
{
	NP_READ_CHECK(this);
	return mConstraints.size();
}

PxU32 NpScene::getConstraints(PxConstraint** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(this);
	const PxU32 size = mConstraints.size();

	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = PxMin(remainder, bufferSize);
	for(PxU32 i=0; i<writeCount; i++)
		buffer[i] = mConstraints[i+startIndex];

	return writeCount;
}

///////////////////////////////////////////////////////////////////////////////

const PxRenderBuffer& NpScene::getRenderBuffer()
{
	if (mPhysicsRunning) 
	{
		// will be reading the Sc::Scene renderable which is getting written 
		// during the sim, hence, avoid call while simulation is running.
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxScene::getRenderBuffer() not allowed while simulation is running.");
	}

	return mRenderBuffer;
}

void NpScene::visualize()
{
	NP_READ_CHECK(this);

	mRenderBuffer.clear(); // clear last frame visualizations 

#if PX_ENABLE_DEBUG_VISUALIZATION
	if(getVisualizationParameter(PxVisualizationParameter::eSCALE) == 0.0f)
		return;

	Cm::RenderOutput out(mRenderBuffer);

	// Visualize scene axis
	const PxReal worldAxes = getVisualizationParameter(PxVisualizationParameter::eWORLD_AXES);
	if (worldAxes != 0)
		out << Cm::DebugBasis(PxVec3(worldAxes));

	// Visualize articulations
	for(PxU32 i=0;i<mArticulations.size();i++)
		static_cast<NpArticulation *>(mArticulations[i])->visualize(out, this);

	// Visualize rigid actors and rigid bodies
	PxRigidActor** actorIt = mRigidActorArray.begin();
	PxRigidActor** actorEnd = mRigidActorArray.end();

#if PX_USE_CLOTH_API
	// Visualize cloths
	for(PxU32 i=0;i<mPxClothArray.size();i++)
		static_cast<NpCloth*>(mPxClothArray[i])->visualize(out, this);
#endif

	for(; actorIt != actorEnd; ++actorIt)
	{
		if ((*actorIt)->getType() == PxActorType::eRIGID_DYNAMIC)
			static_cast<NpRigidDynamic*>(*actorIt)->visualize(out, this);
		else
			static_cast<NpRigidStatic*>(*actorIt)->visualize(out, this);
	}

	// Visualize pruning structures
	const bool visStatic = getVisualizationParameter(PxVisualizationParameter::eCOLLISION_STATIC) != 0.0f;
	const bool visDynamic = getVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC) != 0.0f;
	//flushQueryUpdates(); // DE7834
	if(visStatic && mSceneQueryManager.getStaticPruner())
		mSceneQueryManager.getStaticPruner()->visualize(out, PxU32(PxDebugColor::eARGB_BLUE));
	if(visDynamic && mSceneQueryManager.getDynamicPruner())
		mSceneQueryManager.getDynamicPruner()->visualize(out, PxU32(PxDebugColor::eARGB_RED));

	if(getVisualizationParameter(PxVisualizationParameter::eMBP_REGIONS) != 0.0f)
	{
		out << PxTransform(PxIdentity);

		const PxU32 nbRegions = mScene.getNbBroadPhaseRegions();
		for(PxU32 i=0;i<nbRegions;i++)
		{
			PxBroadPhaseRegionInfo info;
			mScene.getBroadPhaseRegions(&info, 1, i);

			if(info.active)
				out << PxU32(PxDebugColor::eARGB_YELLOW);
			else
				out << PxU32(PxDebugColor::eARGB_BLACK);
			out << DebugBox(info.region.bounds);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::getSimulationStatistics(PxSimulationStatistics& s) const
{
	NP_READ_CHECK(this);

	if (!mPhysicsRunning)
	{
#if PX_ENABLE_SIM_STATS
		mScene.getStats(s);
#endif
	}
	else
	{
		//will be reading data that is getting written during the sim, hence, avoid call while simulation is running.
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::getSimulationStatistics() not allowed while simulation is running. Call will be ignored.");
	}
}

///////////////////////////////////////////////////////////////////////////////

//Multiclient 

PxClientID NpScene::createClient()
{
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN_NULL(mNbClients < PX_MAX_CLIENTS, "Scene::createClient: Maximum number of clients reached! No new client created.");
	mNbClients++;		//track this just for error checking 
	return mScene.createClient();
}

void NpScene::setClientBehaviorFlags(PxClientID client, PxClientBehaviorFlags clientBehaviorFlags)
{
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN(client < mNbClients, "Scene::setClientBehaviorFlags: bad clientID! Please create clientIDs with PxScene::createClient().");
	mScene.setClientBehaviorFlags(client, clientBehaviorFlags);
}

PxClientBehaviorFlags NpScene::getClientBehaviorFlags(PxClientID client) const
{
	NP_READ_CHECK(this);
	PX_CHECK_AND_RETURN_VAL(client < mNbClients, "Scene::getClientBehaviorFlags: bad clientID! Please create clientIDs with PxScene::createClient().", PxClientBehaviorFlags());
	return mScene.getClientBehaviorFlags(client);
}

///////////////////////////////////////////////////////////////////////////////

//FrictionModel 

void NpScene::setFrictionType(PxFrictionType::Enum frictionType)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_AND_RETURN(!mHasSimulated, "NpScene::setFrictionType: This flag can only be set before calling Simulate() or Solve() or Collide() for the first time");
	mScene.setFrictionType(frictionType);
}

PxFrictionType::Enum NpScene::getFrictionType() const
{
	NP_READ_CHECK(this);
	return mScene.getFrictionType();
}

#if PX_USE_CLOTH_API

///////////////////////////////////////////////////////////////////////////////

//Cloth

void NpScene::setClothInterCollisionDistance(PxF32 distance)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_AND_RETURN(distance >= 0.0f, "Scene::setClothInterCollisionDistance: distance must be non-negative.");
	mScene.setClothInterCollisionDistance(distance);
}

PxF32 NpScene::getClothInterCollisionDistance() const
{
	NP_READ_CHECK(this);
	return mScene.getClothInterCollisionDistance();
}

void NpScene::setClothInterCollisionStiffness(PxF32 stiffness)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_AND_RETURN(stiffness >= 0.0f, "Scene::setClothInterCollisionStiffness: stiffness must be non-negative.");
	return mScene.setClothInterCollisionStiffness(stiffness);
}

PxF32 NpScene::getClothInterCollisionStiffness() const
{
	NP_READ_CHECK(this);
	return mScene.getClothInterCollisionStiffness();
}

void NpScene::setClothInterCollisionNbIterations(PxU32 nbIterations)
{
	NP_WRITE_CHECK(this);
	mScene.setClothInterCollisionNbIterations(nbIterations);
}

PxU32 NpScene::getClothInterCollisionNbIterations() const
{
	NP_READ_CHECK(this);
	return mScene.getClothInterCollisionNbIterations();
}

#endif

///////////////////////////////////////////////////////////////////////////////

// Callbacks

void NpScene::setSimulationEventCallback(PxSimulationEventCallback* callback, PxClientID client)
{
	NP_WRITE_CHECK(this);
	mScene.setSimulationEventCallback(callback, client);
}

PxSimulationEventCallback* NpScene::getSimulationEventCallback(PxClientID client) const
{
	NP_READ_CHECK(this);
	return mScene.getSimulationEventCallback(client);
}

void NpScene::setContactModifyCallback(PxContactModifyCallback* callback)
{
	NP_WRITE_CHECK(this);
	mScene.setContactModifyCallback(callback);
}

PxContactModifyCallback* NpScene::getContactModifyCallback() const
{
	NP_READ_CHECK(this);
	return mScene.getContactModifyCallback();
}

void NpScene::setCCDContactModifyCallback(PxCCDContactModifyCallback* callback)
{
	NP_WRITE_CHECK(this);
	mScene.setCCDContactModifyCallback(callback);
}

PxCCDContactModifyCallback* NpScene::getCCDContactModifyCallback() const
{
	NP_READ_CHECK(this);
	return mScene.getCCDContactModifyCallback();
}


void NpScene::setBroadPhaseCallback(PxBroadPhaseCallback* callback, PxClientID client)
{
	NP_WRITE_CHECK(this);
	mScene.setBroadPhaseCallback(callback, client);
}

PxBroadPhaseCallback* NpScene::getBroadPhaseCallback(PxClientID client) const
{
	NP_READ_CHECK(this);
	return mScene.getBroadPhaseCallback(client);
}

void NpScene::setCCDMaxPasses(PxU32 ccdMaxPasses)
{
	NP_WRITE_CHECK(this);
	mScene.setCCDMaxPasses(ccdMaxPasses);
}

PxU32 NpScene::getCCDMaxPasses() const
{
	NP_READ_CHECK(this);
	return mScene.getCCDMaxPasses();
}

PxBroadPhaseType::Enum NpScene::getBroadPhaseType() const
{
	NP_READ_CHECK(this);
	return mScene.getBroadPhaseType();
}

bool NpScene::getBroadPhaseCaps(PxBroadPhaseCaps& caps) const
{
	NP_READ_CHECK(this);
	return mScene.getBroadPhaseCaps(caps);
}

PxU32 NpScene::getNbBroadPhaseRegions() const
{
	NP_READ_CHECK(this);
	return mScene.getNbBroadPhaseRegions();
}

PxU32 NpScene::getBroadPhaseRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(this);
	return mScene.getBroadPhaseRegions(userBuffer, bufferSize, startIndex);
}

PxU32 NpScene::addBroadPhaseRegion(const PxBroadPhaseRegion& region, bool populateRegion)
{
	NP_WRITE_CHECK(this);

	PX_CHECK_MSG(region.bounds.isValid(), "PxScene::addBroadPhaseRegion(): invalid bounds provided!");
	if(region.bounds.isEmpty())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxScene::addBroadPhaseRegion(): region bounds are empty. Call will be ignored.");
		return 0xffffffff;
	}

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneAddBroadphaseRegion, &region, populateRegion);

	return mScene.addBroadPhaseRegion(region, populateRegion);
}

bool NpScene::removeBroadPhaseRegion(PxU32 handle)
{
	NP_WRITE_CHECK(this);
	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneRemoveBroadphaseRegion, handle);
	return mScene.removeBroadPhaseRegion(handle);
}

///////////////////////////////////////////////////////////////////////////////

// Filtering
const void*	NpScene::getFilterShaderData() const
{
	NP_READ_CHECK(this);
	return mScene.getFilterShaderData();
}

PxU32 NpScene::getFilterShaderDataSize() const
{
	NP_READ_CHECK(this);
	return mScene.getFilterShaderDataSize();
}

PxSimulationFilterShader NpScene::getFilterShader() const
{
	NP_READ_CHECK(this);
	return mScene.getFilterShader();
}

PxSimulationFilterCallback*	NpScene::getFilterCallback() const
{
	NP_READ_CHECK(this);
	return mScene.getFilterCallback();
}

void NpScene::resetFiltering(PxActor& actor)
{
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN(NpActor::getAPIScene(actor) && (NpActor::getAPIScene(actor) == this), "Scene::resetFiltering(): actor not in scene!");

	switch(actor.getConcreteType())
	{
		case PxConcreteType::eRIGID_STATIC:
		{
			NpRigidStatic& npStatic = static_cast<NpRigidStatic&>(actor);
			npStatic.resetFiltering(npStatic.getScbRigidStaticFast(), NULL, 0);
		}
		break;

		case PxConcreteType::eRIGID_DYNAMIC:
		{
			NpRigidDynamic& npDynamic = static_cast<NpRigidDynamic&>(actor);
			if (npDynamic.resetFiltering(npDynamic.getScbBodyFast(), NULL, 0))
				npDynamic.wakeUpInternal();
		}
		break;

		case PxConcreteType::eARTICULATION_LINK:
		{
			NpArticulationLink& npLink = static_cast<NpArticulationLink&>(actor);
			if (npLink.resetFiltering(npLink.getScbBodyFast(), NULL, 0))
				npLink.getRoot().wakeUpInternal(false, true);
		}
		break;

#if PX_USE_PARTICLE_SYSTEM_API
		case PxConcreteType::ePARTICLE_SYSTEM:
		{
			NpParticleSystem& npSystem = static_cast<NpParticleSystem&>(actor);
			npSystem.getScbParticleSystem().resetFiltering();
		}
		break;

		case PxConcreteType::ePARTICLE_FLUID:
		{
			NpParticleFluid& npFluid = static_cast<NpParticleFluid&>(actor);
			npFluid.getScbParticleSystem().resetFiltering();
		}
		break;
#endif

		default:
			Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "Scene::resetFiltering(): only PxParticleBase and PxRigidActor support this operation!");
	}
}

void NpScene::resetFiltering(PxRigidActor& actor, PxShape*const* shapes, PxU32 shapeCount)
{
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN(NpActor::getAPIScene(actor) && (NpActor::getAPIScene(actor) == this), "Scene::resetFiltering(): actor not in scene!");

	switch(actor.getConcreteType())
	{
		case PxConcreteType::eRIGID_STATIC:
		{
			NpRigidStatic& npStatic = static_cast<NpRigidStatic&>(actor);
			npStatic.resetFiltering(npStatic.getScbRigidStaticFast(), shapes, shapeCount);
		}
		break;

		case PxConcreteType::eRIGID_DYNAMIC:
		{
			NpRigidDynamic& npDynamic = static_cast<NpRigidDynamic&>(actor);
			if (npDynamic.resetFiltering(npDynamic.getScbBodyFast(), shapes, shapeCount))
				npDynamic.wakeUpInternal();
		}
		break;

		case PxConcreteType::eARTICULATION_LINK:
		{
			NpArticulationLink& npLink = static_cast<NpArticulationLink&>(actor);
			if (npLink.resetFiltering(npLink.getScbBodyFast(), shapes, shapeCount))
				npLink.getRoot().wakeUpInternal(false, true);
		}
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

PxPhysics& NpScene::getPhysics()
{
	return NpPhysics::getInstance();
}

void NpScene::updateDirtyShaders()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,updateDirtyShaders);
	// this should continue to be done in the Np layer even after SC has taken over
	// all vital simulation functions, because it needs to complete before simulate()
	// returns to the application

	// However, the implementation needs fixing so that it does work proportional to
	// the number of dirty shaders

	for(PxU32 i=0;i<mConstraints.size();i++)
	{
		static_cast<NpConstraint*>(mConstraints[i])->updateConstants();
	}
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::simulate(PxReal _elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation)
{
	{
	CM_PROFILE_START_CROSSTHREAD(mScene.getEventProfiler(), Cm::ProfileEventId::Basic::Getsimulate());
	// write guard must end before simulation kicks off worker threads
	// otherwise the simulation callbacks could overlap with this function
	// and peform API reads,triggering an error
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN(_elapsedTime > 0, "Scene::simulate: The elapsed time must be positive!");
	PX_CHECK_AND_RETURN(!isPhysicsRunning(), "Scene::simulate: Simulation is still processing last simulate call, you should call fetchResults()!");

	PX_CHECK_AND_RETURN((reinterpret_cast<size_t>(scratchBlock)&15) == 0, "Scene::simulate: scratch block must be 16-byte aligned!");
	PX_CHECK_AND_RETURN((scratchBlockSize&16383) == 0, "Scene::simulate: scratch block size must be a multiple of 16K");

	PX_SIMD_GUARD;	

#if PX_SUPPORT_VISUAL_DEBUGGER
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene, Basic, pvdFrameStart);

	//This call pauses us if PVD sent a pause command
	PxVisualDebugger* theDebugger = NpPhysics::getInstance().getVisualDebugger();
	if ( theDebugger != NULL )
		theDebugger->checkConnection();
	
	//Flush the pvd commands before the simulate()
	mScene.getSceneVisualDebugger().flushPendingCommands();

	//signal the frame is starting.	
	mScene.getSceneVisualDebugger().frameStart(_elapsedTime);
	}
#endif

#ifdef PHYSX_STATS
	PhysX_Simulate(this, _elapsedTime);
#endif

#if PX_ENABLE_DEBUG_VISUALIZATION
	visualize();
#endif

	// signal thread
	mPhysicsRunning = true;
	mIsBuffering	= true;
	mCollisionRunning = true;

	elapsedTime = _elapsedTime;

	mHasSimulated = true;

	//mScene.setSceneMaterialTableBuffered();

	updateDirtyShaders();
#if PX_SUPPORT_VISUAL_DEBUGGER
	{
		mScene.getSceneVisualDebugger().updateJoints();
	}
#endif

	NpPhysics& physics = (NpPhysics&)this->getPhysics();
	NpMaterialManager& manager = physics.getMaterialManager();
	NpMaterial** materials = manager.getMaterials();

	mScene.updateLowLevelMaterial(materials);
	mScene.preSimulateUpdateAppThread(_elapsedTime);

	mScene.setPhysicsRunning(true);
	mScene.setPhysicsBuffering(true);  // Clear the buffering flag to allow buffered writes to execute immediately. Once collision detection is running, buffering is automatically forced on
	mScene.getScScene().setScratchBlock(scratchBlock, scratchBlockSize);

	}

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,taskFrameworkSetup);
		if (controlSimulation)
		{
			{
				CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,resetDependencies);
				// Only reset dependencies, etc if we own the TaskManager.  Will be false
				// when an NpScene is controlled by an APEX scene.
				mTaskManager->resetDependencies();
			}
			mTaskManager->startSimulation();
		}

		mControllingSimulation = controlSimulation;
		mSceneCompletion.setContinuation(*mTaskManager, completionTask);
		mSceneExecution.setContinuation(*mTaskManager, &mSceneCompletion);

#if PX_SUPPORT_GPU_PHYSX
		//workaround to prevent premature launching of gpu launch task
		if (PxGpuDispatcher* gpuDispatcher = getGpuDispatcher())
		{
			gpuDispatcher->addPreLaunchDependent(mSceneCompletion);
		}
#endif

		mSceneCompletion.removeReference();
		mSceneExecution.removeReference();
	}
}


void NpScene::prepareSolve(PxReal _elapsedTime, void* scratchBlock, PxU32 scratchBlockSize)
{
	CM_PROFILE_START_CROSSTHREAD(mScene.getEventProfiler(), Cm::ProfileEventId::Basic::Getsimulate());
	// write guard must end before simulation kicks off worker threads
	// otherwise the simulation callbacks could overlap with this function
	// and peform API reads,triggering an error
	NP_WRITE_CHECK(this);

	PX_CHECK_AND_RETURN(_elapsedTime > 0, "Scene::simulate: The elapsed time must be non-negative!");
	PX_CHECK_AND_RETURN(!isPhysicsRunning(), "Scene::simulate: Simulation is still processing last simulate call, you should call fetchResults()!");

	PX_CHECK_AND_RETURN((reinterpret_cast<size_t>(scratchBlock)&15) == 0, "Scene::simulate: scratch block must be 16-byte aligned!");
	PX_CHECK_AND_RETURN((scratchBlockSize&16383) == 0, "Scene::simulate: scratch block size must be a multiple of 16K");	

#if PX_SUPPORT_VISUAL_DEBUGGER
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene, Basic, pvdFrameStart);

	//This call pauses us if PVD sent a pause command
	PxVisualDebugger* theDebugger = NpPhysics::getInstance().getVisualDebugger();
	if ( theDebugger != NULL )
		theDebugger->checkConnection();
	//signal the frame is starting.
	mScene.getSceneVisualDebugger().frameStart(_elapsedTime);
	}
#endif

#ifdef PHYSX_STATS
	PhysX_Simulate(this, _elapsedTime);
#endif

	// signal thread
	mPhysicsRunning = true;

	elapsedTime = _elapsedTime;

#if PX_ENABLE_DEBUG_VISUALIZATION
	visualize();
#endif

	//mScene.setSceneMaterialTableBuffered();

	updateDirtyShaders();
#if PX_SUPPORT_VISUAL_DEBUGGER
	{
		mScene.getSceneVisualDebugger().updateJoints();
	}
#endif


	//mScene.preSimulateUpdateAppThread(_elapsedTime);

	mScene.setPhysicsRunning(true);
	
	if(!mCollisionRunning)
	{
		mScene.getScScene().setScratchBlock(scratchBlock, scratchBlockSize);
	}

#ifdef SERIALIZE_SCENE_EXECUTION
	NpPhysics::getInstance().lockScene();
#endif


}

void NpScene::prepareCollide(PxReal _elapsedTime)
{
	NP_WRITE_CHECK(this);

	mScene.preSimulateUpdateAppThread(_elapsedTime);

	elapsedTime = _elapsedTime;
	NpPhysics& physics = (NpPhysics&)this->getPhysics();
	NpMaterialManager& manager = physics.getMaterialManager();
	NpMaterial** materials = manager.getMaterials();
	mScene.getScScene().setElapsedTime(_elapsedTime);

	//sync all the material events
	mScene.updateLowLevelMaterial(materials);
	
}

void NpScene::solve(PxReal _elapsedTime, physx::PxBaseTask* completionTask,  void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation)
{
#if !PX_ENABLE_INVERTED_STEPPER_FEATURE
	PX_UNUSED(_elapsedTime);
	PX_UNUSED(completionTask);
	PX_UNUSED(scratchBlock);
	PX_UNUSED(scratchBlockSize);
	PX_UNUSED(controlSimulation);
	Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Not implemented in this release!");
	return;
#else
	//ML:we need to wait for the collision finish before we can solve the contacts
	if( !mCollisionRunning || checkCollisionInternal(true) )
	{
#ifdef AG_PERFMON
	Ps::Foundation::getInstance().getPAUtils().startEvent( gPerfMonSimulate, 0 );
#endif // AG_PERFMON

	prepareSolve(_elapsedTime, scratchBlock, scratchBlockSize);

	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,taskFrameworkSetup);
		//if (controlSimulation)
		//{
		//	// Only reset dependencies, etc if we own the TaskManager.  Will be false
		//	// when an NpScene is controlled by an APEX scene.
		//	mTaskManager->resetDependencies();
		//	mTaskManager->startSimulation();
		//}

		mControllingSimulation = controlSimulation;
		mSceneCompletion.setContinuation(*mTaskManager, completionTask);
		mSceneSolve.setContinuation(*mTaskManager, &mSceneCompletion);
		mSceneCompletion.removeReference();
		mSceneSolve.removeReference();
	}

#ifdef AG_PERFMON
	getFoundation().getPAUtils().stopEvent( gPerfMonSimulate, 0 );
#endif // AG_PERFMON
	}

	mHasSimulated = true;
#endif
}

void NpScene::collide(PxReal _elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize)
{
#if !PX_ENABLE_INVERTED_STEPPER_FEATURE
	PX_UNUSED(_elapsedTime);
	PX_UNUSED(completionTask);
	PX_UNUSED(scratchBlock);
	PX_UNUSED(scratchBlockSize);
	Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Not implemented in this release!");
	return;
#else
	mScene.getScScene().setScratchBlock(scratchBlock, scratchBlockSize);
	prepareCollide(_elapsedTime);
	mIsBuffering	= true;
	mCollisionRunning = true;
	mScene.setPhysicsBuffering(true);

	mHasSimulated = true;

	if (1)//controlSimulation)
	{
		// Only reset dependencies, etc if we own the TaskManager.  Will be false
		// when an NpScene is controlled by an APEX scene.
		mTaskManager->resetDependencies();
		mTaskManager->startSimulation();
	}

	mScene.stepSetupCollide();
	mCollisionCompletion.setContinuation(*mTaskManager, completionTask);
	mSceneCollide.setContinuation(&mCollisionCompletion);
	mCollisionCompletion.removeReference();
	mSceneCollide.removeReference();
#endif
}

bool NpScene::checkResultsInternal(bool block)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Basic,checkResults);
	return mPhysicsDone.wait(block ? Ps::Sync::waitForever : 0);
}

bool NpScene::checkCollisionInternal(bool block)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Basic,checkCollision);
	return mCollisionDone.wait(block ? Ps::Sync::waitForever : 0);
}

bool NpScene::checkResults(bool block)
{
	return checkResultsInternal(block);
}

bool NpScene::checkCollision(bool block)
{
	return checkCollisionInternal(block);
}


struct NpSceneFetchResultsScope
{
	Scb::Scene& mScene;
	NpSceneFetchResultsScope(Scb::Scene& inScene)
		: mScene( inScene )
	{
	}
	~NpSceneFetchResultsScope()
	{
		#if PX_SUPPORT_VISUAL_DEBUGGER
			mScene.getSceneVisualDebugger().frameEnd();
		#endif
	}

private:
	NpSceneFetchResultsScope& operator=(const NpSceneFetchResultsScope&);
};

void NpScene::fireCallBacksPreSync()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,fireCallBacksPreSync);

	// Fire broad-phase callbacks
	{
		Sc::Scene& scene = mScene.getScScene();
		using namespace physx::Sc;

		bool outputWarning = scene.fireOutOfBoundsCallbacks();

		// Aggregates
		{
			Ps::Array<void*>& outAgg = scene.getOutOfBoundsAggregates();
			const PxU32 nbOut1 = outAgg.size();

			for(PxU32 i=0;i<nbOut1;i++)
			{
				PxAggregate* px = (PxAggregate*)outAgg[i];
				NpAggregate* np = static_cast<NpAggregate*>(px);
				if(np->getScbAggregate().getControlState()==Scb::ControlState::eREMOVE_PENDING)
					continue;

				// PT: used to avoid calling the callback twice for the same client
				bool flags[PX_MAX_CLIENTS];
				PxMemZero(flags, PX_MAX_CLIENTS*sizeof(bool));

				PxU32 nbActors = np->getCurrentSizeFast();
				for(PxU32 j=0;j<nbActors;j++)
				{
					PxActor* pxActor = np->getActorFast(j);
					const PxClientID clientID = pxActor->getOwnerClient();
					if(!flags[clientID])
					{
						flags[clientID] = true;
						PxBroadPhaseCallback* cb = scene.getBroadPhaseCallback(clientID);
						if(cb)
						{
							cb->onObjectOutOfBounds(*px);
						}
						else
						{
							outputWarning = true;
						}
					}
				}
			}
			outAgg.reset();
		}

		if(outputWarning)
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "At least one object is out of the broadphase bounds. To manage those objects, define a PxBroadPhaseCallback for each used client.");
	}

	mScene.fireCallBacksPreSync();		// fire all callbacks which need the data in a state which is not synchronized with the latest simulation results
}

//extern PxU32 gContactCache_NbCalls;
//extern PxU32 gContactCache_NbHits;

bool NpScene::fetchResults(bool block, PxU32* errorState)
{
	if(!mPhysicsRunning && mCollisionRunning)
	{
		if(!checkCollisionInternal(block))
			return false;
	}

	PX_SIMD_GUARD;

	if (!mPhysicsRunning && !mCollisionRunning && !mIsBuffering)
	{
		return false;
	}
	else if (mPhysicsRunning && !checkResultsInternal(block))
	{
		return false;
	}

	// take write check *after* simulation has finished, otherwise 
	// we will block simulation callbacks from using the API
	// disallow re-entry to detect callbacks making write calls
	NP_WRITE_CHECK_NOREENTRY(this);

	//We always need to indicate a frame end and we always need to flush the profile events.
	//these things need to happen *after* everything else has happened.
	NpSceneFetchResultsScope fetchResultsScope(mScene);

	// we use cross thread profile here, to show the event in cross thread view
	CM_PROFILE_START_CROSSTHREAD(mScene.getEventProfiler(), Cm::ProfileEventId::Basic::GetfetchResults());
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,fetchResults);

	// The order of the following operations is important!
	// 1. Process object deletions which were carried out while the simulation was running (since these effect contact and trigger reports)
	// 2. Write contact reports to global stream (taking pending deletions into account), clear some simulation buffers (deleted objects etc.), ...
	// 3. Send reports which have to be done before the data is synced (contact & trigger reports etc.) such that the user gets the old state.
	// 4. Mark the simulation as not running internally to allow reading data which should not be read otherwise
	// 5. Synchronize the simulation and user state
	// 6. Fire callbacks which need to reflect the synchronized object state

#if PX_SUPPORT_VISUAL_DEBUGGER	
	mScene.getSceneVisualDebugger().updateContacts();
#endif

	mScene.prepareOutOfBoundsCallbacks();
	mScene.processPendingRemove();
	mScene.endSimulation();

	fireCallBacksPreSync();		// fire all callbacks which need the data in a state which is not synchronized with the latest simulation results

	mScene.postCallbacksPreSync();

	mScene.setPhysicsRunning(false);	// This is ok since fetchResults() is blocking, so no user changes will come in at that point
	mScene.setPhysicsBuffering(false);  // Clear the buffering flag to allow buffered writes to execute immediately. Once collision detection is running, buffering is automatically forced on

	mScene.syncEntireScene(errorState);	// double buffering

	mSceneQueryManager.processSimUpdates();	

#if PX_SUPPORT_VISUAL_DEBUGGER	
	mScene.getSceneVisualDebugger().updateSceneQueries();

	getSingleSqCollector().clear();
	getBatchedSqCollector().clear();
#endif

	// fire sleep and wake-up events
	// we do this after buffer-swapping so that the events have the new state
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,fireCallBacksPostSync);
		mScene.fireCallBacksPostSync();
	}

	mScene.postReportsCleanup();

	// build the list of active transforms
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,buildActiveTransforms);
		if(mScene.getFlags() & PxSceneFlag::eENABLE_ACTIVETRANSFORMS)
			mScene.buildActiveTransforms();
	}

	mRenderBuffer.append(mScene.getScScene().getRenderBuffer());

	if (mPhysicsRunning && mControllingSimulation)
	{
		mTaskManager->stopSimulation();
	}

	mPhysicsRunning = false;			// allows reading mPhysicsRunning in a threadsafer way
	mIsBuffering = false;
	mCollisionRunning = false;

	mPhysicsDone.reset();				// allow Physics to run again
	mCollisionDone.reset();

	CM_PROFILE_STOP_CROSSTHREAD(mScene.getEventProfiler(), Cm::ProfileEventId::Basic::GetfetchResults());
	CM_PROFILE_STOP_CROSSTHREAD(mScene.getEventProfiler(), Cm::ProfileEventId::Basic::Getsimulate());

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneFetchResults, *mGrbEventPools, *this);

//	if(gContactCache_NbCalls)
//		printf("%d | %d | %f\n", gContactCache_NbCalls, gContactCache_NbHits, float(gContactCache_NbHits)/float(gContactCache_NbCalls));
//	gContactCache_NbCalls = 0;
//	gContactCache_NbHits = 0;

	return true;
}



void NpScene::flushSimulation(bool sendPendingReports)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,flushSimulation);
	NP_WRITE_CHECK(this);
	PX_SIMD_GUARD;

	if (mPhysicsRunning)
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"Scene::flushSimulation(): This call is not allowed while the simulation is running. Call will be ignored");
		return;
	}

	//!!! TODO: send joint break events?

	mScene.flush(sendPendingReports);

	//!!! TODO: Shrink all NpObject lists?
}

void NpScene::flushQueryUpdates()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,flushQueryUpdates);
	NP_WRITE_CHECK(this);
	PX_SIMD_GUARD;

	if (mPhysicsRunning)
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"Scene::flushQueryUpdates(): This call is not allowed while the simulation is running. Call will be ignored");
		return;
	}

	mSceneQueryManager.flushUpdates();
}


/*
Replaces finishRun() with the addition of appropriate thread sync(pulled out of PhysicsThread())

Note: this function can be called from the application thread or the physics thread, depending on the
scene flags.
*/
void NpScene::executeScene(PxBaseTask* continuation)
{
	mScene.simulate(elapsedTime, continuation);
}

void NpScene::executeCollide(PxBaseTask* continuation)
{
	mScene.collide(elapsedTime, continuation);
}

void NpScene::executeSolve(PxBaseTask* continuation)
{
	mScene.solve(elapsedTime, continuation);
}

///////////////////////////////////////////////////////////////////////////////

bool NpScene::addMaterial(NpMaterial& mat)
{
	return mScene.addMaterial(mat.getScMaterial());
}

void NpScene::updateMaterial(NpMaterial& mat)
{
	//PxU32 index = mat.getTableIndex();
	mScene.updateMaterial(mat.getScMaterial());

	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneUpdateMaterial, &mat);
}

void NpScene::removeMaterial(NpMaterial& mat)
{
	GRB_EVENT(this, GrbInteropEvent3, GrbInteropEvent3::PxSceneRemoveMaterial, &mat, 1);

	//PxU32 index = mat.getTableIndex();
	mScene.removeMaterial(mat.getScMaterial());
}

///////////////////////////////////////////////////////////////////////////////

void NpScene::setDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2, const PxDominanceGroupPair& dominance)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_AND_RETURN((group1 < PX_MAX_DOMINANCE_GROUP && group2 < PX_MAX_DOMINANCE_GROUP), 
		"Scene::setDominanceGroupPair: invalid params! Groups must be <= 31!");
	//can't change matrix diagonal 
	PX_CHECK_AND_RETURN(group1 != group2, "Scene::setDominanceGroupPair: invalid params! Groups must be unequal! Can't change matrix diagonal!");
	PX_CHECK_AND_RETURN(
		((dominance.dominance0) == 1.0f && (dominance.dominance1 == 1.0f))
		||	((dominance.dominance0) == 1.0f && (dominance.dominance1 == 0.0f))
		||	((dominance.dominance0) == 0.0f && (dominance.dominance1 == 1.0f))
		, "Scene::setDominanceGroupPair: invalid params! dominance must be one of (1,1), (1,0), or (0,1)!");

	mScene.setDominanceGroupPair(group1, group2, dominance);
}


PxDominanceGroupPair NpScene::getDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2) const
{
	NP_READ_CHECK(this);
	PX_CHECK_AND_RETURN_VAL((group1 < PX_MAX_DOMINANCE_GROUP && group2 < PX_MAX_DOMINANCE_GROUP), 
		"Scene::getDominanceGroupPair: invalid params! Groups must be <= 31!", PxDominanceGroupPair(1.0f, 1.0f));
	return mScene.getDominanceGroupPair(group1, group2);
}

///////////////////////////////////////////////////////////////////////////////

#if PX_USE_PARTICLE_SYSTEM_API

void NpScene::addParticleSystem(NpParticleSystem& system)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addParticleSystem);
	mScene.addParticleSystem(system.getScbParticleSystem());
	mPxParticleBaseArray.pushBack(&system);

	updatePhysXIndicator();
}

void NpScene::removeParticleSystem(NpParticleSystem& system)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeParticleSystem);
	PX_ASSERT(system.getNpScene() == this);

	mScene.removeParticleSystem(system.getScbParticleSystem(), false);
	removeFromParticleBaseList(system);

	updatePhysXIndicator();
}

void NpScene::addParticleFluid(NpParticleFluid& fluid)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addParticleFluid);
	mScene.addParticleSystem(fluid.getScbParticleSystem());
	mPxParticleBaseArray.pushBack(&fluid);

	updatePhysXIndicator();
}

void NpScene::removeParticleFluid(NpParticleFluid& fluid)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeParticleFluid);
	PX_ASSERT(fluid.getNpScene() == this);

	mScene.removeParticleSystem(fluid.getScbParticleSystem(), false);
	removeFromParticleBaseList(fluid);

	updatePhysXIndicator();
}

// PT: TODO: inline this one in the header for consistency
void NpScene::removeFromParticleBaseList(PxParticleBase& particleBase)
{
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,findAndReplaceWithLast);
		bool status = mPxParticleBaseArray.findAndReplaceWithLast(&particleBase);
		PX_ASSERT(status);
		PX_UNUSED(status);
	}
}

#endif // PX_USE_PARTICLE_SYSTEM_API

///////////////////////////////////////////////////////////////////////////////

#if PX_USE_CLOTH_API

void NpScene::addCloth(NpCloth& cloth)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addCloth);
	mScene.addCloth(cloth.getScbCloth());
	mPxClothArray.pushBack(&cloth);

	updatePhysXIndicator();
}

void NpScene::removeCloth(NpCloth& cloth)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeCloth);
	PX_ASSERT(NpActor::getAPIScene(cloth) == this);

	mScene.removeCloth(cloth.getScbCloth());
	removeFromClothList(cloth);

	updatePhysXIndicator();
}

#endif  // PX_USE_CLOTH_API

///////////////////////////////////////////////////////////////////////////////

#if PX_SUPPORT_GPU_PHYSX

void NpScene::updatePhysXIndicator()
{
	Ps::IntBool isGpu = 0;

#if PX_USE_PARTICLE_SYSTEM_API
	for (PxU32 i = 0; !isGpu && i < mPxParticleBaseArray.size(); i++)
	{
		NpParticleSystem* particles = (NpParticleSystem*)mPxParticleBaseArray[i]->is<PxParticleSystem>();
		NpParticleFluid* fluid = (NpParticleFluid*)mPxParticleBaseArray[i]->is<PxParticleFluid>();

		isGpu |= particles && particles->getScbParticleSystem().getScParticleSystem().isGpu();
		isGpu |= fluid && fluid->getScbParticleSystem().getScParticleSystem().isGpu();
	}
#endif

#if PX_USE_CLOTH_API
	for (PxU32 i = 0; !isGpu && i < mPxClothArray.size(); i++)
	{
		NpCloth* pCloth = (NpCloth*)mPxClothArray[i]->is<PxCloth>();
		isGpu = pCloth->getScbCloth().getScCloth().isGpu();
	}
#endif

	mPhysXIndicator.setIsGpu(isGpu != 0);
}
#endif	//PX_SUPPORT_GPU_PHYSX

///////////////////////////////////////////////////////////////////////////////

PxVolumeCache* NpScene::createVolumeCache(PxU32 maxStaticShapes, PxU32 maxDynamicShapes)
{
	NpVolumeCache* cache = PX_NEW(NpVolumeCache)(&mSceneQueryManager, maxStaticShapes, maxDynamicShapes);
	mVolumeCaches.insert(cache);
	return cache;
}

void NpScene::releaseVolumeCache(NpVolumeCache* volumeCache)
{
	bool found = mVolumeCaches.erase(volumeCache); PX_UNUSED(found);
	PX_ASSERT_WITH_MESSAGE(found, "volume cache not found in releaseVolumeCache");
	PX_DELETE(static_cast<NpVolumeCache*>(volumeCache));
}

void NpScene::setDynamicTreeRebuildRateHint(PxU32 dynamicTreeRebuildRateHint)
{
	PX_CHECK_AND_RETURN((dynamicTreeRebuildRateHint >= 4), "Scene::setDynamicTreeRebuildRateHint(): Param has to be >= 4!");
	mSceneQueryManager.setDynamicTreeRebuildRateHint(dynamicTreeRebuildRateHint);
}

PxU32 NpScene::getDynamicTreeRebuildRateHint() const
{
	NP_READ_CHECK(this);
	return mSceneQueryManager.getDynamicTreeRebuildRateHint();
}

void NpScene::forceDynamicTreeRebuild(bool rebuildStaticStructure, bool rebuildDynamicStructure)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,forceDynamicTreeRebuild);
	NP_WRITE_CHECK(this);
	PX_SIMD_GUARD;
	mSceneQueryManager.forceDynamicTreeRebuild(rebuildStaticStructure, rebuildDynamicStructure);
}

void NpScene::setSolverBatchSize(PxU32 solverBatchSize)
{
	NP_WRITE_CHECK(this);
	mScene.setSolverBatchSize(solverBatchSize);
}

PxU32 NpScene::getSolverBatchSize(void) const
{
	NP_READ_CHECK(this);
	// get from our local copy
	return mScene.getSolverBatchSize();
}

///////////////////////////////////////////////////////////////////////////////

bool NpScene::setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_AND_RETURN_VAL(PxIsFinite(value), "NpScene::setVisualizationParameter: value is not valid.", false);

	if (param >= PxVisualizationParameter::eNUM_VALUES)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "setVisualizationParameter: parameter out of range.");
		return false;
	}
	else if (value < 0.0f)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "setVisualizationParameter: value must be larger or equal to 0.");
		return false;
	}
	else
	{
		mScene.setVisualizationParameter(param, value);
		return true;
	}
}

PxReal NpScene::getVisualizationParameter(PxVisualizationParameter::Enum param) const
{
	if (param < PxVisualizationParameter::eNUM_VALUES)
		return mScene.getVisualizationParameter(param);
	else
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "getVisualizationParameter: param is not an enum.");

	return 0.0f;
}

void NpScene::setVisualizationCullingBox(const PxBounds3& box)
{
	NP_WRITE_CHECK(this);
	PX_CHECK_MSG(box.isValid(), "PxScene::setVisualizationCullingBox(): invalid bounds provided!");
	mScene.setVisualizationCullingBox(box);
}

const PxBounds3& NpScene::getVisualizationCullingBox() const
{
	NP_READ_CHECK(this);
	const PxBounds3& bounds = mScene.getVisualizationCullingBox();
	PX_ASSERT(bounds.isValid());
	return bounds;
}

#ifdef PX_PS3
void NpScene::setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value)
{
	NP_WRITE_CHECK(this);
	mScene.setSceneParamInt(param,value);
}

PxU32 NpScene::getSceneParamInt(PxPS3ConfigParam::Enum param)
{
	NP_READ_CHECK(this);
	return mScene.getSceneParamInt(param);
}

void NpScene::getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks)
{
	NP_READ_CHECK(this);
	return mScene.getScScene().getSpuMemBlockCounters(numNpContactStreamBlocks, numNpCacheBlocks, numDyFrictionBlocks, numDyConstraintBlocks);
}
#endif

void NpScene::setNbContactDataBlocks(PxU32 numBlocks)
{
	PX_CHECK_AND_RETURN((mPhysicsRunning == false), 
		"Scene::setNbContactDataBlock: This call is not allowed while the simulation is running. Call will be ignored!");
	
	mScene.getScScene().setNbContactDataBlocks(numBlocks);
}

PxU32 NpScene::getNbContactDataBlocksUsed() const
{
	PX_CHECK_AND_RETURN_VAL((mPhysicsRunning == false), 
		"Scene::getNbContactDataBlocksUsed: This call is not allowed while the simulation is running. Returning 0.", 0);
	
	return mScene.getScScene().getNbContactDataBlocksUsed();
}

PxU32 NpScene::getMaxNbContactDataBlocksUsed() const
{
	PX_CHECK_AND_RETURN_VAL((mPhysicsRunning == false), 
		"Scene::getMaxNbContactDataBlocksUsed: This call is not allowed while the simulation is running. Returning 0.", 0);
	
	return mScene.getScScene().getMaxNbContactDataBlocksUsed();
}

PxU32 NpScene::getTimestamp() const
{
	return mScene.getScScene().getTimeStamp();
}

PxU32 NpScene::getSceneQueryStaticTimestamp() const
{
	return mSceneQueryManager.getStaticTimestamp();
}

PxReal NpScene::getMeshContactMargin() const
{
	return mScene.getScScene().getMeshContactMargin();
}

PxCpuDispatcher*	NpScene::getCpuDispatcher() const
{
	return getTaskManager()->getCpuDispatcher();
}

PxGpuDispatcher*	NpScene::getGpuDispatcher() const
{
	return getTaskManager()->getGpuDispatcher();
}

PxSpuDispatcher* NpScene::getSpuDispatcher() const
{
	return getTaskManager()->getSpuDispatcher();
}

PxPruningStructure::Enum NpScene::getStaticStructure() const
{
	return mSceneQueryManager.getStaticStructure();
}

PxPruningStructure::Enum NpScene::getDynamicStructure() const
{
	return mSceneQueryManager.getDynamicStructure();
}

PxF32 NpScene::getContactCorrelationDistance() const 
{
	return mScene.getScScene().getContactCorrelationDistance();
}

PxReal NpScene::getFrictionOffsetThreshold() const
{
	return mScene.getScScene().getFrictionOffsetThreshold();
}

PxU32 NpScene::getContactReportStreamBufferSize() const
{
	return mScene.getScScene().getDefaultContactReportStreamBufferSize();
}

#ifdef PX_CHECKED
void NpScene::checkPositionSanity(const PxRigidActor& a, const PxTransform& pose, const char* fnName) const
{
	if(!mSanityBounds.contains(pose.p))
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__,
			"%s: actor pose for %lp is outside sanity bounds\n", fnName, &a);
}
#endif

namespace 
{
	struct ThreadReadWriteCount
	{
		PxU8 readDepth;			// depth of re-entrant reads
		PxU8 writeDepth;		// depth of re-entrant writes 

		PxU8 readLockDepth;		// depth of read-locks
		PxU8 writeLockDepth;	// depth of write-locks
	};
}

#if NP_ENABLE_THREAD_CHECKS

bool NpScene::startWrite(bool allowReentry)
{ 
	PX_COMPILE_TIME_ASSERT(sizeof(ThreadReadWriteCount) == 4);

	if (mScene.getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK)
	{
		ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));

		// ensure we already have the write lock
		return localCounts.writeLockDepth > 0;
	}
	else
	{
		ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
		bool error = false;

		// check that we are the only thread reading (this allows read->write order on a single thread)
		error |= mConcurrentReadCount != localCounts.readDepth;

		// check no other threads are writing 
		error |= mConcurrentWriteCount != localCounts.writeDepth;

		// increment shared write counter
		Ps::atomicIncrement(&mConcurrentWriteCount);

		// in the normal case (re-entry is allowed) then we simply increment
		// the writeDepth by 1, otherwise (re-entry is not allowed) increment
		// by 2 to force subsequent writes to fail by creating a mismatch between
		// the concurrent write counter and the local counter, any value > 1 will do
		if (allowReentry)
			localCounts.writeDepth++;
		else
			localCounts.writeDepth+=2;

		TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));

		if (error)
			Ps::atomicIncrement(&mConcurrentErrorCount);

		return !error;
	}
}

void NpScene::stopWrite(bool allowReentry) 
{ 
	if (!(mScene.getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK))
	{
		Ps::atomicDecrement(&mConcurrentWriteCount);

		// decrement depth of writes for this thread
		ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));

		// see comment in startWrite()
		if (allowReentry)
			localCounts.writeDepth--;
		else
			localCounts.writeDepth-=2;

		TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));
	}
}

bool NpScene::startRead() const 
{ 
	if (mScene.getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK)
	{
		ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));

		// ensure we already have the write or read lock
		return localCounts.writeLockDepth > 0 || localCounts.readLockDepth > 0;
	}
	else
	{
		Ps::atomicIncrement(&mConcurrentReadCount);

		// update current threads read depth
		ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
		localCounts.readDepth++;
		TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));

		// success if the current thread is already performing a write (API re-entry) or no writes are in progress
		bool success = (localCounts.writeDepth > 0 || mConcurrentWriteCount == 0); 

		if (!success)
			Ps::atomicIncrement(&mConcurrentErrorCount);

		return success;
	}
} 

void NpScene::stopRead() const 
{
	if (!(mScene.getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK))
	{
		Ps::atomicDecrement(&mConcurrentReadCount); 

		// update local threads read depth
		ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
		localCounts.readDepth--;
		TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));
	}
}

#else 

bool NpScene::startWrite(bool) { PX_ASSERT(0); return false; }
void NpScene::stopWrite(bool) {}

bool NpScene::startRead() const { PX_ASSERT(0); return false; }
void NpScene::stopRead() const {}

#endif // NP_ENABLE_THREAD_CHECKS


void NpScene::lockRead(const char* /*file*/, PxU32 /*line*/)
{
	// increment this threads read depth
	ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
	localCounts.readLockDepth++;
	TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));

	// if we are the current writer then do nothing (allow reading from threads with write ownership)
	if (mCurrentWriter == Thread::getId())
		return;

	// only lock on first read
	if (localCounts.readLockDepth == 1)
		mRWLock.lockReader();
}

void NpScene::unlockRead()
{
	// increment this threads read depth
	ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
	if (localCounts.readLockDepth < 1)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::unlockRead() called without matching call to PxScene::lockRead(), behaviour will be undefined.");
		return;
	}
	localCounts.readLockDepth--;
	TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));

	// if we are the current writer then do nothing (allow reading from threads with write ownership)
	if (mCurrentWriter == Thread::getId())
		return;

	// only unlock on last read
	if (localCounts.readLockDepth == 0)
		mRWLock.unlockReader();
}

void NpScene::lockWrite(const char* file, PxU32 line)
{
	// increment this threads write depth
	ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
	if (localCounts.writeLockDepth == 0 && localCounts.readLockDepth > 0)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, file?file:__FILE__, file?int(line):__LINE__, "PxScene::lockWrite() detected after a PxScene::lockRead(), lock upgrading is not supported, behaviour will be undefined.");
		return;
	}
	localCounts.writeLockDepth++;
	TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));

	// only lock on first call
	if (localCounts.writeLockDepth == 1)
		mRWLock.lockWriter();

	PX_ASSERT(mCurrentWriter == 0 || mCurrentWriter == Thread::getId());

	// set ourselves as the current writer
	mCurrentWriter = Thread::getId();
}

void NpScene::unlockWrite()
{
	// increment this thread's write depth
	ThreadReadWriteCount localCounts = PxUnionCast<ThreadReadWriteCount>(TlsGet(mThreadReadWriteDepth));
	if (localCounts.writeLockDepth < 1)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxScene::unlockWrite() called without matching call to PxScene::lockWrite(), behaviour will be undefined.");
		return;
	}
	localCounts.writeLockDepth--;
	TlsSet(mThreadReadWriteDepth, PxUnionCast<void*>(localCounts));

	PX_ASSERT(mCurrentWriter == Thread::getId());

	if (localCounts.writeLockDepth == 0)
	{
		mCurrentWriter = 0;	
		mRWLock.unlockWriter();
	}
}


PxReal NpScene::getWakeCounterResetValue() const
{
	NP_READ_CHECK(this);

	return getWakeCounterResetValueInteral();
}


static PX_FORCE_INLINE void shiftRigidActor(PxRigidActor* a, const PxVec3& shift)
{
	PxActorType::Enum t = a->getType();
	if (t == PxActorType::eRIGID_DYNAMIC)
	{
		NpRigidDynamic* rd = static_cast<NpRigidDynamic*>(a);
		rd->getScbBodyFast().onOriginShift(shift);
	}
	else if (t == PxActorType::eRIGID_STATIC)
	{
		NpRigidStatic* rs = static_cast<NpRigidStatic*>(a);
		rs->getScbRigidStaticFast().onOriginShift(shift);
	}
	else
	{
		PX_ASSERT(t == PxActorType::eARTICULATION_LINK);
		NpArticulationLink* al = static_cast<NpArticulationLink*>(a);
		al->getScbBodyFast().onOriginShift(shift);
	}
}


void NpScene::shiftOrigin(const PxVec3& shift)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,shiftOrigin);
	NP_WRITE_CHECK(this);

	if(mScene.isPhysicsBuffering())
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::shiftOrigin() not allowed while simulation is running. Call will be ignored.");
		return;
	}
	
	PX_SIMD_GUARD;

	const PxU32 prefetchLookAhead = 4;
	PxU32 rigidCount = mRigidActorArray.size();
	PxU32 batchIterCount = rigidCount / prefetchLookAhead;
	
	PxU32 idx = 0;
	for(PxU32 i=0; i < batchIterCount; i++)
	{
		// prefetch elements for next batch
		if (i < (batchIterCount-1))
		{
			Ps::prefetchLine(mRigidActorArray[idx + prefetchLookAhead]);
			Ps::prefetchLine(((PxU8*)mRigidActorArray[idx + prefetchLookAhead]) + 128);  // for the buffered pose
			Ps::prefetchLine(mRigidActorArray[idx + prefetchLookAhead + 1]);
			Ps::prefetchLine(((PxU8*)mRigidActorArray[idx + prefetchLookAhead + 1]) + 128);
			Ps::prefetchLine(mRigidActorArray[idx + prefetchLookAhead + 2]);
			Ps::prefetchLine(((PxU8*)mRigidActorArray[idx + prefetchLookAhead + 2]) + 128);
			Ps::prefetchLine(mRigidActorArray[idx + prefetchLookAhead + 3]);
			Ps::prefetchLine(((PxU8*)mRigidActorArray[idx + prefetchLookAhead + 3]) + 128);
		}
		else
		{
			for(PxU32 k=(idx + prefetchLookAhead); k < rigidCount; k++)
			{
				Ps::prefetchLine(mRigidActorArray[k]);
				Ps::prefetchLine(((PxU8*)mRigidActorArray[k]) + 128);
			}
		}

		for(PxU32 j=idx; j < (idx + prefetchLookAhead); j++)
		{
			shiftRigidActor(mRigidActorArray[j], shift);
		}

		idx += prefetchLookAhead;
	}
	// process remaining objects
	for(PxU32 i=idx; i < rigidCount; i++)
	{
		shiftRigidActor(mRigidActorArray[i], shift);
	}


	for(PxU32 i=0; i < mArticulations.size(); i++)
	{
		NpArticulation* np = static_cast<NpArticulation*>(mArticulations[i]);
		NpArticulationLink*const* links = np->getLinks();

		for(PxU32 j=0; j < np->getNbLinks(); j++)
		{
			shiftRigidActor(links[j], shift);
		}
	}


	mScene.shiftOrigin(shift);


	//
	// shift scene query related data structures
	//
	mSceneQueryManager.shiftOrigin(shift);

	Ps::HashSet<NpVolumeCache*>::Iterator it = mVolumeCaches.getIterator();
	while (!it.done())
	{
		NpVolumeCache* cache = (*it);
		cache->onOriginShift(shift);
		++it;
	}


#if PX_ENABLE_DEBUG_VISUALIZATION
	//
	// debug visualization
	//
	mRenderBuffer.shift(-shift);
#endif
}


///////////////////////////////////////////////////////////////////////////////
#if (USE_GRB_INTEROP == 1)

#include "NpScene.h"
#include "GrbEventStream.h"

GrbInteropEventStream3 * NpScene::createSceneEventStream()
{
	Array<GrbInteropEvent3> * eventStream = &mGrbEventStreams.insert();
	new (eventStream) Array<GrbInteropEvent3>;

	mGrbEventStreamAllocs.pushBack(new StackAllocator(mGrbEventStreamStackAllocatorPageSize));

	GrbInteropEventStream3 * retval = new GrbInteropEventStream3(this, eventStream);
	
	return retval;
}

void NpScene::releaseSceneEventStream(GrbInteropEventStream3 * sceneEventStream)
{
	PxU32 index = PxU32(sceneEventStream->getEventStream() - &mGrbEventStreams[0]);

	mGrbEventStreams.replaceWithLast(index);
	delete mGrbEventStreamAllocs[index];
	mGrbEventStreamAllocs.replaceWithLast(index);

}

PxU32 NpScene::getNumEventStreams()
{
	return mGrbEventStreams.size();
}

void NpScene::eventStreamSend(const GrbInteropEvent3 & sceneEvent, PxU32 stream)
{
	mGrbEventStreams[stream].pushBack(sceneEvent);
}

StackAllocator & NpScene::getEventStreamStackAlloc(PxU32 eventStream)
{
	return *(mGrbEventStreamAllocs[eventStream]);
}

void NpScene::clearEventStream(Array<GrbInteropEvent3> * eventStream)
{
	int index = (int)(eventStream - &mGrbEventStreams[0]);

	for(PxU32 i=0;i!=eventStream->size();i++)
	{
		//Free any memory allocated by the event
		(*eventStream)[i].release();
	}

	eventStream->clear();

	mGrbEventStreamAllocs[(PxU32)index]->reset(); //reset the stack
}
#endif
///////////////////////////////////////////////////////////////////////////////

PxBatchQuery* NpScene::createBatchQuery(const PxBatchQueryDesc& desc)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,createBatchQuery);
	PX_CHECK_AND_RETURN_NULL(desc.isValid(),"Supplied PxBatchQueryDesc is not valid. createBatchQuery returns NULL.");

	NpBatchQuery* bq = PX_NEW(NpBatchQuery)(*this, desc);
	mBatchQueries.pushBack(bq);
	return bq;
}

void NpScene::releaseBatchQuery(PxBatchQuery* sq)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,releaseBatchQuery);
	NpBatchQuery* npsq = static_cast<NpBatchQuery*>(sq);
	bool found = mBatchQueries.findAndReplaceWithLast(npsq);
	PX_UNUSED(found); PX_ASSERT(found);
	PX_DELETE_AND_RESET(npsq);
}

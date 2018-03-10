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


#ifndef PX_PHYSICS_NP_SCENE
#define PX_PHYSICS_NP_SCENE

#include "PsUserAllocated.h"
#include "PsSync.h"
#include "PsArray.h"
#include "PsThread.h"
#include "CmEventProfiler.h"
#ifdef PX_PS3
#include "ps3/PxPS3Config.h"
#endif

#include "GrbEvents.h"

#if PX_SUPPORT_GPU_PHYSX
#include "device/PhysXIndicator.h"
#endif

#include "NpSceneQueries.h"

namespace physx
{

class PhysicsThread;
class PxBatchQueryDesc;
class NpMaterial;
class NpVolumeCache;

#if (USE_GRB_INTEROP == 1)
class GrbInteropEventStream3;
#endif

namespace Sc
{
	class Joint;
	class ConstraintBreakEvent;
}

namespace Sq
{
	class SceneQueryManager;
}


class PxTask;
class NpObjectFactory;
class NpRigidStatic;
class NpRigidDynamic;
class NpParticleSystem;
class NpParticleFluid;
class NpConstraint;
class NpArticulationLink;
class NpCloth;
class NpShapeManager;
class NpBatchQuery;

class PxBatchQuery;

enum NpProfileZones
{
	NpScene_checkResults,
	NpScene_reportContacts,
	NpScene_reportProfiling,
	NpScene_reportTriggers,
	NpScene_stats,

	NpPrNumZones
};

class NpScene : public NpSceneQueries, public Ps::UserAllocated
{
	//virtual interfaces:

	//An enum for states. The state of the scene is either complete, running or waiting. 
	//When the state is "waiting", a fetch is waiting on another thread. In this case, the 
	enum
	{
		eSIMULATE_COMPLETE,
		eSIMULATE_RUNNING,
		eFETCH_WAITING
	};
	
	PX_NOCOPY(NpScene)
	public:

	virtual			void							release();

	virtual			void							setFlag(PxSceneFlag::Enum flag, bool value);
	virtual			PxSceneFlags					getFlags() const;

	// implement PxScene:

	virtual			void							setGravity(const PxVec3&);
	virtual			PxVec3							getGravity() const;

	virtual			void							setBounceThresholdVelocity(const PxReal t);
	virtual			PxReal							getBounceThresholdVelocity() const;

	virtual			PxReal							getContactCorrelationDistance() const;
	virtual			PxReal							getFrictionOffsetThreshold() const;

	virtual			void							setLimits(const PxSceneLimits& limits);
	virtual			PxSceneLimits					getLimits() const;

	virtual			void							addActor(PxActor& actor);
	virtual			void							removeActor(PxActor& actor, bool wakeOnLostTouch);

	virtual			PxU32							getNbConstraints() const;
	virtual			PxU32							getConstraints(PxConstraint** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	virtual			void							addArticulation(PxArticulation&);
	virtual			void							removeArticulation(PxArticulation&, bool wakeOnLostTouch);
	virtual			PxU32							getNbArticulations() const;
	virtual			PxU32							getArticulations(PxArticulation** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

// PX_AGGREGATE
    virtual			void							addAggregate(PxAggregate&);
	virtual			void							removeAggregate(PxAggregate&, bool wakeOnLostTouch);
	virtual			PxU32							getNbAggregates()	const;
	virtual			PxU32							getAggregates(PxAggregate** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;
//~PX_AGGREGATE
	
	virtual			void							addCollection(const PxCollection& collection);

	// Groups
	virtual			void							setDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2, const PxDominanceGroupPair& dominance);
	virtual			PxDominanceGroupPair			getDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2) const;

	// Actors
	virtual			PxU32							getNbActors(PxActorTypeFlags types) const;
	virtual			PxU32							getActors(PxActorTypeFlags types, PxActor** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;
	virtual			const PxActiveTransform*		getActiveTransforms(PxU32& nbTransformsOut, PxClientID client);

	// Run
	virtual			void							getSimulationStatistics(PxSimulationStatistics& s) const;

	// Multiclient 
	virtual			PxClientID						createClient();
	virtual			void							setClientBehaviorFlags(PxClientID client, PxClientBehaviorFlags clientBehaviorFlags); 
	virtual			PxClientBehaviorFlags			getClientBehaviorFlags(PxClientID client) const;

	// FrictionModel
	virtual			void							setFrictionType(PxFrictionType::Enum frictionType);
	virtual			PxFrictionType::Enum			getFrictionType() const;

#if PX_USE_CLOTH_API
	// Cloth
	virtual			void							setClothInterCollisionDistance(PxF32 distance);
	virtual			PxF32							getClothInterCollisionDistance() const;
	virtual			void							setClothInterCollisionStiffness(PxF32 stiffness); 
	virtual			PxF32							getClothInterCollisionStiffness() const;
	virtual			void							setClothInterCollisionNbIterations(PxU32 nbIterations);
	virtual			PxU32							getClothInterCollisionNbIterations() const;
#endif

	// Callbacks
	virtual			void							setSimulationEventCallback(PxSimulationEventCallback* callback, PxClientID client);
	virtual			PxSimulationEventCallback*		getSimulationEventCallback(PxClientID client)	const;
	virtual			void							setContactModifyCallback(PxContactModifyCallback* callback);
	virtual			PxContactModifyCallback*		getContactModifyCallback()	const;
	virtual			void							setCCDContactModifyCallback(PxCCDContactModifyCallback* callback);
	virtual			PxCCDContactModifyCallback*		getCCDContactModifyCallback()	const;
	virtual			void							setBroadPhaseCallback(PxBroadPhaseCallback* callback, PxClientID client);
	virtual			PxBroadPhaseCallback*			getBroadPhaseCallback(PxClientID client)		const;

	//CCD passes
	virtual			void							setCCDMaxPasses(PxU32 ccdMaxPasses);
	virtual			PxU32							getCCDMaxPasses()	const;

	// Collision filtering
	virtual			const void*						getFilterShaderData() const;
	virtual			PxU32							getFilterShaderDataSize() const;
	virtual			PxSimulationFilterShader		getFilterShader() const;
	virtual			PxSimulationFilterCallback*		getFilterCallback() const;
	virtual			void							resetFiltering(PxActor& actor);
	virtual			void							resetFiltering(PxRigidActor& actor, PxShape*const* shapes, PxU32 shapeCount);

	// Get Physics SDK
	virtual			PxPhysics&						getPhysics();

	// new API methods
	virtual			void							simulate(PxReal _elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation);
	virtual			void							solve(PxReal _elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation = true);
	virtual			void							collide(PxReal _elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize);
	virtual			bool							checkResults(bool block);
	virtual			bool							checkCollision(bool block);
	virtual			bool							fetchResults(bool block, PxU32* errorState);
	virtual			void							flush(bool sendPendingReports) { flushSimulation(sendPendingReports); }
	virtual			void							flushSimulation(bool sendPendingReports);
	virtual			void							flushQueryUpdates();
	virtual			const PxRenderBuffer&			getRenderBuffer();

	virtual			PxBatchQuery*					createBatchQuery(const PxBatchQueryDesc& desc);
					void							releaseBatchQuery(PxBatchQuery* bq);
	virtual			PxVolumeCache*					createVolumeCache(PxU32 maxStaticShapes, PxU32 maxDynamicShapes);
					void							releaseVolumeCache(NpVolumeCache* cache);
	virtual			void							setDynamicTreeRebuildRateHint(PxU32 dynamicTreeRebuildRateHint);
	virtual			PxU32							getDynamicTreeRebuildRateHint() const;
	virtual			void							forceDynamicTreeRebuild(bool rebuildStaticStructure, bool rebuildDynamicStructure);

	virtual			void							setSolverBatchSize(PxU32 solverBatchSize);
	virtual			PxU32							getSolverBatchSize(void) const;

	virtual			bool							setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value);
	virtual			PxReal							getVisualizationParameter(PxVisualizationParameter::Enum param) const;

	virtual			void							setVisualizationCullingBox(const PxBounds3& box);
	virtual			const PxBounds3&				getVisualizationCullingBox() const;

	virtual			PxTaskManager*			getTaskManager()	{ return mTaskManager; }
					void							checkBeginWrite() const {}
					
	virtual         void							setNbContactDataBlocks(PxU32 numBlocks);
	virtual         PxU32							getNbContactDataBlocksUsed() const;
	virtual         PxU32							getMaxNbContactDataBlocksUsed() const;

	virtual			PxU32							getContactReportStreamBufferSize() const;

#ifdef PX_PS3
					void							setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value);
					PxU32							getSceneParamInt(PxPS3ConfigParam::Enum param);
					void							getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks);
#endif

	virtual			PxU32							getTimestamp()	const;
	virtual			PxU32							getSceneQueryStaticTimestamp()	const;

	virtual			PxCpuDispatcher*				getCpuDispatcher() const;
	virtual			PxGpuDispatcher*				getGpuDispatcher() const;
	virtual			PxSpuDispatcher*				getSpuDispatcher() const ;

	virtual			PxPruningStructure::Enum		getStaticStructure() const;
	virtual			PxPruningStructure::Enum		getDynamicStructure() const;

	virtual			PxReal							getMeshContactMargin() const;
	
	virtual			PxBroadPhaseType::Enum			getBroadPhaseType()									const;
	virtual			bool							getBroadPhaseCaps(PxBroadPhaseCaps& caps)			const;
	virtual			PxU32							getNbBroadPhaseRegions()							const;
	virtual			PxU32							getBroadPhaseRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;
	virtual			PxU32							addBroadPhaseRegion(const PxBroadPhaseRegion& region, bool populateRegion);
	virtual			bool							removeBroadPhaseRegion(PxU32 handle);

	virtual			void							addActors(PxActor*const* actors, PxU32 nbActors);
	virtual			void							removeActors(PxActor*const* actors, PxU32 nbActors, bool wakeOnLostTouch);

	virtual			void							lockRead(const char* file=NULL, PxU32 line=0);
	virtual			void							unlockRead();

	virtual			void							lockWrite(const char* file=NULL, PxU32 line=0);
	virtual			void							unlockWrite();

	virtual			PxReal							getWakeCounterResetValue() const;

	virtual			void							shiftOrigin(const PxVec3& shift);

#if (USE_GRB_INTEROP == 1)
			virtual GrbInteropEventStream3 *			createSceneEventStream();
			virtual void                            clearEventStream(Ps::Array<GrbInteropEvent3> * eventStream);
			virtual void							releaseSceneEventStream(GrbInteropEventStream3 *);
			virtual PxU32							getNumEventStreams();
			virtual void							eventStreamSend(const GrbInteropEvent3 & sceneEvent, PxU32 stream);
			GrbInterop3::StackAllocator &		getEventStreamStackAlloc(PxU32 eventStream);
#endif

	//internal public methods:
	public:
													NpScene(const PxSceneDesc& desc);
													~NpScene();

	PX_FORCE_INLINE	PxReal							getElapsedTime()			const	{ return elapsedTime;			}
	PX_FORCE_INLINE	bool							isPhysicsRunning()			const	{ return mPhysicsRunning;		}
	PX_FORCE_INLINE	bool							isPhysicsBuffering()		const	{ return mIsBuffering;			}
	PX_FORCE_INLINE	PxTaskManager*					getTaskManager()			const	{ return mTaskManager;			}

					void							prepareSolve(PxReal elapsedTime, void* scratchMemBlock = 0, PxU32 scratchMemBlockSize = 0);
					void							prepareCollide(PxReal _elapsedTime);
					void							addActorInternal(PxActor& actor);
					void							removeActorInternal(PxActor& actor, bool wakeOnLostTouch, bool removeFromAggregate);
					bool							addActorsInternal(PxU32 nbActors, PxActor** PX_RESTRICT actors, const PxU8* PX_RESTRICT actorTypes);
					bool							addActorsInternalSingleType(PxU32 nbActors, PxActor** PX_RESTRICT actors, const PxU8& actorType);

					void							addArticulationInternal(PxArticulation&);
					void							removeArticulationInternal(PxArticulation&, bool wakeOnLostTouch,  bool removeFromAggregate);
	// materials
					bool							addMaterial(NpMaterial& mat);
					void							updateMaterial(NpMaterial& mat);
					void							removeMaterial(NpMaterial& mat);

					void							executeScene(PxBaseTask* continuation);
					void							executeCollide(PxBaseTask* continuation);
					void							executeSolve(PxBaseTask* continuation);
					void							constraintBreakEventNotify(PxConstraint *const *constraints, PxU32 count);

					bool							loadFromDesc(const PxSceneDesc&);

					void							removeFromRigidActorList(const PxU32&);					
	PX_FORCE_INLINE	void							removeFromArticulationList(PxArticulation& articulation);
	PX_FORCE_INLINE	void							removeFromAggregateList(PxAggregate& aggregate);

	PX_FORCE_INLINE	void							addToConstraintList(PxConstraint&);
	PX_FORCE_INLINE	void							removeFromConstraintList(PxConstraint&);

#if PX_USE_PARTICLE_SYSTEM_API
					void							removeFromParticleBaseList(PxParticleBase&);
#endif

					void							addArticulationLink(NpArticulationLink& link);
					void							addArticulationLinkBody(NpArticulationLink& link);
					void							addArticulationLinkConstraint(NpArticulationLink& link);
					void							removeArticulationLink(NpArticulationLink& link, bool wakeOnLostTouch);

					bool							startWrite(bool allowReentry);
					void							stopWrite(bool allowReentry);

					bool							startRead() const;
					void							stopRead() const;

					PxU32							getReadWriteErrorCount() const { return (PxU32)mConcurrentErrorCount; }

#ifdef PX_CHECKED
					void							checkPositionSanity(const PxRigidActor& a, const PxTransform& pose, const char* fnName) const;
#endif

#if PX_USE_CLOTH_API
					void							addCloth(NpCloth&);
					void							removeCloth(NpCloth&);
#endif

#if PX_SUPPORT_GPU_PHYSX
					void							updatePhysXIndicator();
#else
	PX_FORCE_INLINE	void							updatePhysXIndicator() {}
#endif

	PX_FORCE_INLINE PxReal							getWakeCounterResetValueInteral() const { return mScene.getWakeCounterResetValue(); }

private:
					bool							checkResultsInternal(bool block);
					bool							checkCollisionInternal(bool block);
					void							addRigidStatic(NpRigidStatic&);
					void							removeRigidStatic(NpRigidStatic&, bool wakeOnLostTouch, bool removeFromAggregate);
					void							addRigidDynamic(NpRigidDynamic&);
					void							removeRigidDynamic(NpRigidDynamic&, bool wakeOnLostTouch, bool removeFromAggregate);

					bool							addRigidActorsInternal(PxU32 nbActors, PxActor** PX_RESTRICT actors);

#if PX_USE_PARTICLE_SYSTEM_API
					void							addParticleSystem(NpParticleSystem&);
					void							removeParticleSystem(NpParticleSystem&);
					void							addParticleFluid(NpParticleFluid&);
					void							removeParticleFluid(NpParticleFluid&);
#endif

#if PX_USE_CLOTH_API
	PX_FORCE_INLINE	void							removeFromClothList(PxCloth&);
#endif

					Ps::ProfilerManager*			getProfilerManager();
					void							visualize();

					void							updateDirtyShaders();

					void							fireCallBacksPreSync();

					void							updateScbStateAndSetupSq(const PxRigidActor& rigidActor, Scb::Actor& actor, NpShapeManager& shapeManager, bool actorDynamic, PxBounds3* bounds);
	PX_FORCE_INLINE	void							updateScbStateAndSetupSq(const PxRigidActor& rigidActor, Scb::Body& body, NpShapeManager& shapeManager, bool actorDynamic, PxBounds3* bounds);

					Cm::RenderBuffer				mRenderBuffer;

					Ps::Array<PxConstraint*>		mConstraints;
					Ps::Array<PxRigidActor*>		mRigidActorArray;	//  for NpScene::getActors()
					Ps::Array<PxArticulation*>		mArticulations;
					Ps::Array<PxAggregate*>			mAggregates;
					Ps::HashSet<NpVolumeCache*>		mVolumeCaches;
					Ps::Array<NpBatchQuery*>		mBatchQueries;

#if PX_USE_PARTICLE_SYSTEM_API
					Ps::Array<PxParticleBase*>		mPxParticleBaseArray;	//  for NpScene::getParticleBaseArray()
#endif

#if PX_USE_CLOTH_API
					Ps::Array<PxCloth*>				mPxClothArray; //  for NpScene::getCloths()
#endif

					PxBounds3						mSanityBounds;
#if PX_SUPPORT_GPU_PHYSX
					PhysXIndicator					mPhysXIndicator;
#endif

					Ps::Sync						mPhysicsDone;		// physics thread signals this when update ready
					Ps::Sync						mCollisionDone;		// physics thread signals this when all collisions ready
					bool							mPhysicsRunning;	// is true between simulate() and fetchResults()
					bool							mIsBuffering;		// is true between collideStep() and fetchResults() 
					bool							mCollisionRunning;

		//legacy timing settings:
					PxReal							elapsedTime;		//needed to transfer the elapsed time param from the user to the sim thread.

					PxU32							mNbClients;		// Tracks reserved clients for multiclient support.
					Ps::Array<PxU32>				mClientBehaviorFlags;// Tracks behavior bits for clients.

					struct SceneCompletion : public Cm::Task
					{
						SceneCompletion(Ps::Sync& sync) : mSync(sync){}
						virtual void runInternal() {}
						virtual void release() 
						{ 
							PxBaseTask* c = mCont; 
							mSync.set(); 
							if(c) c->removeReference(); 
						}
						virtual const char* getName() const { return "NpScene.completion"; }
						Ps::Sync& mSync;
					private:
						SceneCompletion& operator=(const SceneCompletion&);
					};
					typedef Cm::DelegateTask<NpScene, &NpScene::executeScene> SceneExecution;
					typedef Cm::DelegateTask<NpScene, &NpScene::executeCollide> SceneCollide;
					typedef Cm::DelegateTask<NpScene, &NpScene::executeSolve> SceneSolve;

					PxTaskManager*					mTaskManager;
					SceneCompletion					mSceneCompletion;
					SceneCompletion					mCollisionCompletion;
					SceneExecution					mSceneExecution;
					SceneCollide					mSceneCollide;
					SceneSolve						mSceneSolve;
					bool                            mControllingSimulation;

					PxU32							mSimThreadStackSize;

					volatile PxI32					mConcurrentWriteCount;
					mutable volatile PxI32			mConcurrentReadCount;					
					mutable volatile PxI32			mConcurrentErrorCount;

					// TLS slot index, keeps track of re-entry depth for this thread
					PxU32							mThreadReadWriteDepth;
					Ps::Thread::Id					mCurrentWriter;
					ReadWriteLock					mRWLock;

#if (USE_GRB_INTEROP == 1)
					Array< Array<GrbInteropEvent3> >	mGrbEventStreams;
					GrbInterop3::PoolSet*		mGrbEventPools;
					Array< GrbInterop3::StackAllocator * > mGrbEventStreamAllocs;
					static const PxU32	mGrbEventStreamStackAllocatorPageSize = 1024 * 1024; //1 MB
#endif
					bool							mHasSimulated;
};


PX_FORCE_INLINE	void NpScene::addToConstraintList(PxConstraint& constraint)
{
	mConstraints.pushBack(&constraint);
}


PX_FORCE_INLINE	void NpScene::removeFromConstraintList(PxConstraint& constraint)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,findAndReplaceWithLast);
	bool status = mConstraints.findAndReplaceWithLast(&constraint);
	PX_ASSERT(status);
	PX_UNUSED(status);
}


PX_FORCE_INLINE void NpScene::removeFromArticulationList(PxArticulation& articulation)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,findAndReplaceWithLast);
	bool status = mArticulations.findAndReplaceWithLast(&articulation);
	PX_ASSERT(status);
	PX_UNUSED(status);
}

PX_FORCE_INLINE void NpScene::removeFromAggregateList(PxAggregate& aggregate)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,findAndReplaceWithLast);
	bool status = mAggregates.findAndReplaceWithLast(&aggregate);
	PX_ASSERT(status);
	PX_UNUSED(status);
}

#if PX_USE_CLOTH_API
PX_FORCE_INLINE void NpScene::removeFromClothList(PxCloth& cloth)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,findAndReplaceWithLast);
	bool status = mPxClothArray.findAndReplaceWithLast(&cloth);
	PX_ASSERT(status);
	PX_UNUSED(status);
}
#endif  // PX_USE_CLOTH_API

}

#endif

/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLE_SCENE_H__
#define __DESTRUCTIBLE_SCENE_H__

#include "NxApex.h"
#include "NiApexScene.h"
#include "NiModule.h"
#include "DestructibleActor.h"
#include "DestructibleAsset.h"
#include "DestructibleStructure.h"
#include "ModuleDestructible.h"
#include "PsHashMap.h"
#include "PsHashSet.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxUserContactReport.h"
#include <NxUserNotify.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include <PxSimulationEventCallback.h>
#include <PxContactModifyCallback.h>
#include <PhysX3ClassWrapper.h>
#endif

#include "NxDestructibleActorJoint.h"

#include "NiApexRenderDebug.h"

#include "DebugRenderParams.h"
#include "DestructibleDebugRenderParams.h"

#include "PsTime.h"
#include "PsMutex.h"
#include "ApexContext.h"

#include "PxTaskManager.h"
#include "PxTask.h"

#if APEX_RUNTIME_FRACTURE
#include "SimScene.h"
#endif

#include "ModulePerfScope.h"

// We currently have to copy into the contact buffer because the native contact
// stream is compressed within PhysX.  If we need to optimize we could use a
// contact iterator like what's found in PxContactPair::extractContacts()
#define USE_EXTRACT_CONTACTS						1
#define PAIR_POINT_ALLOCS							0

#ifndef PX_VERIFY
#ifndef NDEBUG
#define PX_VERIFY(f) PX_ASSERT(f)
#else   // NDEBUG
#define PX_VERIFY(f) ((void)(f))
#endif
#endif

namespace physx
{
namespace apex
{
namespace destructible
{

class ModuleDestructible;
class DestructibleScene;
class DestructibleActorProxy;
class DestructibleBeforeTick;

struct IndexedReal
{
	physx::PxF32	value;
	physx::PxU32	index;

	PX_INLINE bool	operator > (IndexedReal& i)
	{
		return value > i.value;
	}
};

#if NX_SDK_VERSION_MAJOR == 2
class DestructibleContactReport : public NxUserContactReport
{
public:
	DestructibleContactReport() : destructibleScene(NULL) {}

	void onContactNotify(NxContactPair& pair, physx::PxU32 events);

	DestructibleScene*		destructibleScene;
};

class DestructibleContactModify : public NxUserContactModify
{
public:
	DestructibleContactModify() : destructibleScene(NULL) {}

	bool onContactConstraint(
	    NxU32& changeFlags,
	    const NxShape* shape0,
	    const NxShape* shape1,
	    const NxU32 featureIndex0,
	    const NxU32 featureIndex1,
	    NxContactCallbackData& data);

	DestructibleScene*		destructibleScene;
};


class DestructibleUserNotify : public NxUserNotify
{
public:
	DestructibleUserNotify(ModuleDestructible& module, DestructibleScene* destructibleScene);

private:
	virtual bool onJointBreak(physx::PxF32 breakingImpulse, NxJoint& brokenJoint);
	virtual void onWake(NxActor** actors, physx::PxU32 count);
	virtual void onSleep(NxActor** actors, physx::PxU32 count);

	void operator=(const DestructibleUserNotify&) {}

private:
	DestructibleScene*	mDestructibleScene;
	ModuleDestructible& mModule;
};
#elif NX_SDK_VERSION_MAJOR == 3
class DestructibleContactModify : public PxContactModifyCallback
{
public:
	DestructibleContactModify() : destructibleScene(NULL) {}

	void onContactModify(PxContactModifyPair* const pairs, PxU32 count);

	DestructibleScene*		destructibleScene;
};

class DestructibleUserNotify : public PxSimulationEventCallback
{
public:
	DestructibleUserNotify(ModuleDestructible& module, DestructibleScene* destructibleScene);

private:
	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count);
	virtual void onWake(PxActor** actors, physx::PxU32 count);
	virtual void onSleep(PxActor** actors, physx::PxU32 count);
	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count);

	void operator=(const DestructibleUserNotify&) {}

private:
	ModuleDestructible&			mModule;
	DestructibleScene*			mDestructibleScene;
#if USE_EXTRACT_CONTACTS
	physx::Array<physx::PxU8>	mPairPointBuffer;
#endif
};
#endif

class ApexDamageEventReportData : public NxApexDamageEventReportData
{
public:
	ApexDamageEventReportData()
	{
		clear();
	}

	ApexDamageEventReportData(const ApexDamageEventReportData& other)
	{
		*this = other;
	}

	ApexDamageEventReportData&	operator = (const ApexDamageEventReportData& other)
	{
		m_destructible = other.m_destructible;
		m_fractureEvents = other.m_fractureEvents;
		m_chunkReportBitMask = other.m_chunkReportBitMask;
		m_chunkReportMaxFractureEventDepth = other.m_chunkReportMaxFractureEventDepth;
		destructible = other.destructible;
		hitDirection = other.hitDirection;
		worldBounds = other.worldBounds;
		totalNumberOfFractureEvents = other.totalNumberOfFractureEvents;
		minDepth = other.minDepth;
		maxDepth = other.maxDepth;
		fractureEventList = m_fractureEvents.size() > 0 ? &m_fractureEvents[0] : NULL;
		fractureEventListSize = m_fractureEvents.size();
		impactDamageActor = other.impactDamageActor;
		appliedDamageUserData = other.appliedDamageUserData;
		hitPosition = other.hitPosition;
		return *this;
	}

	void						setDestructible(DestructibleActor* inDestructible);

	physx::PxU32				addFractureEvent(const DestructibleStructure::Chunk& chunk, physx::PxU32 flags);

	NxApexChunkData&			getFractureEvent(physx::PxU32 index)
	{
		return m_fractureEvents[index];
	}

	void						clearChunkReports();

private:
	void						clear()
	{
		m_destructible = NULL;
		m_fractureEvents.reset();
		m_chunkReportBitMask = 0xffffffff;
		m_chunkReportMaxFractureEventDepth = 0xffffffff;
		destructible = NULL;
		hitDirection = physx::PxVec3(0.0f);
		worldBounds.setEmpty();
		totalNumberOfFractureEvents = 0;
		minDepth = 0xffff;
		maxDepth = 0;
		fractureEventList = NULL;
		fractureEventListSize = 0;
		impactDamageActor = NULL;
		appliedDamageUserData = NULL;
		hitPosition = physx::PxVec3(0.0f);
	}

	DestructibleActor*				m_destructible;
	physx::Array<NxApexChunkData>	m_fractureEvents;
	physx::PxU32					m_chunkReportBitMask;
	physx::PxU32					m_chunkReportMaxFractureEventDepth;
};

struct ActorFIFOEntry
{
	enum Flag
	{
		IsDebris =			1 << 0,
		ForceLODRemove =	1 << 1,

		MassUpdateNeeded =	1 << 8,
	};

	ActorFIFOEntry() : actor(NULL), origin(0.0f), age(0.0f), benefitCache(PX_MAX_F32), maxSpeed(0.0f), averagedLinearVelocity(0.0f), averagedAngularVelocity(0.0f), unscaledMass(0.0f), flags(0) {}
	ActorFIFOEntry(NxActor* inActor, physx::PxF32 inUnscaledMass, physx::PxU32 inFlags) : actor(inActor), age(0.0f), benefitCache(PX_MAX_F32), maxSpeed(0.0f),
		averagedLinearVelocity(0.0f), averagedAngularVelocity(0.0f), unscaledMass(inUnscaledMass), flags(inFlags)
	{
		PxFromNxVec3(origin, actor->getCMassGlobalPosition());
	}

	NxActor*		actor;
	physx::PxVec3	origin;
	physx::PxF32	age;
	physx::PxF32	benefitCache;	//for LOD
	physx::PxF32	maxSpeed;
	physx::PxVec3	averagedLinearVelocity;
	physx::PxVec3	averagedAngularVelocity;
	physx::PxF32	unscaledMass;
	physx::PxU32	flags;
};

struct DormantActorEntry
{
	DormantActorEntry() : actor(NULL), unscaledMass(0.0f), flags(0) {}
	DormantActorEntry(NxActor* inActor, physx::PxF32 inUnscaledMass, physx::PxU32 inFlags) : actor(inActor), unscaledMass(inUnscaledMass), flags(inFlags) {}

	NxActor*		actor;
	physx::PxF32	unscaledMass;
	physx::PxU32	flags;
};

struct ChunkBenefitCoefs
{
	ChunkBenefitCoefs() : perChunkInitialBenefit(1.0f), benefitDecayTimeConstant(1.0f), benefitPerPercentScreenArea(1.0f) {}

	physx::PxF32	perChunkInitialBenefit;
	physx::PxF32	benefitDecayTimeConstant;
	physx::PxF32	benefitPerPercentScreenArea;
};

struct ChunkCostCoefs
{
	ChunkCostCoefs() : perChunkCost(1.0f) {}

	physx::PxF32	perChunkCost;
};

struct ChunkLODRef
{
	physx::PxU32	structureID;
	physx::PxU32	chunkIndex;
	physx::PxF32	cost;
	physx::PxF32	benefit;

	static int cmpCostIncreasing(const void* a, const void* b)
	{
		const physx::PxF32 diff = ((const ChunkLODRef*)a)->cost - ((const ChunkLODRef*)b)->cost;
		return diff == 0 ? 0 : (diff > 0 ? 1 : -1);
	}
	static int cmpBenefitDecreasing(const void* a, const void* b)
	{
		const physx::PxF32 diff = ((const ChunkLODRef*)b)->benefit - ((const ChunkLODRef*)a)->benefit;
		return diff == 0 ? 0 : (diff > 0 ? 1 : -1);
	}
};

/* Class which manages DestructibleActors and DestructibleStructures
 * associate with a single NxApexScene
 */

class DestructibleScene : public NiModuleScene, public ApexContext, public NxApexResource, public ApexResource
{
public:
	DestructibleScene(ModuleDestructible& module, NiApexScene& scene, NiApexRenderDebug* debugRender, NxResourceList& list);
	~DestructibleScene();

	/* NiModuleScene */
	virtual void				submitTasks(PxF32 elapsedTime, PxF32 substepSize, PxU32 numSubSteps);
	virtual void				setTaskDependencies();
	virtual void				tasked_beforeTick(PxF32 elapsedTime);
	virtual void				fetchResults();

	virtual void				setModulePhysXScene(NxScene* s);
	virtual NxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	virtual void				release()
	{
		mModule->releaseNiModuleScene(*this);
	}
	virtual void				visualize();

	virtual NxModule*			getNxModule()
	{
		return mModule;
	}
	virtual physx::PxF32		getBenefit();
	virtual physx::PxF32		setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit);

	ModuleDestructible*			getModule() const
	{
		return mModule;
	}

	NiApexScene*				getApexScene()
	{
		return mApexScene;
	}

	void						initModuleSettings();

#if APEX_RUNTIME_FRACTURE
	::physx::fracture::SimScene*		getDestructibleRTScene(bool create = true);
#endif

	enum StatsDataEnum
	{
		VisibleDestructibleChunkCount,
		DynamicDestructibleChunkIslandCount,
		NumberOfShapes,
		NumberOfAwakeShapes,
		RbThroughput,

		// insert new items before this line
		NumberOfStats			// The number of stats
	};

	virtual NxApexSceneStats* getStats();

	/* NxApexResource */
	physx::PxU32				getListIndex() const
	{
		return m_listIndex;
	}
	void						setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	/* DestructibleScene methods */

	void						applyRadiusDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, physx::PxF32 radius, bool falloff);

	bool						isActorCreationRateExceeded();
	bool						isFractureBufferProcessRateExceeded();

	void						addToAwakeList(DestructibleActor& actor);
	void						removeFromAwakeList(DestructibleActor& actor);

	enum
	{
		InvalidReportID =		0xFFFFFFFF
	};

private:

#if NX_SDK_VERSION_MAJOR == 3
	void						addActorsToScene();
	physx::Array<physx::PxActor*>	mActorsToAdd;
	HashMap<physx::PxActor*, PxU32>	mActorsToAddIndexMap;

	void addForceToAddActorsMap(physx::PxActor* actor, const ActorForceAtPosition& force);
	HashMap<physx::PxActor*, ActorForceAtPosition> mForcesToAddToActorsMap;
#endif

	ModuleDestructible*			mModule;
	NiApexScene*				mApexScene;
	NxScene*					mPhysXScene;
#if NX_SDK_VERSION_MAJOR == 2
	DestructibleContactReport	mContactReport;
#endif
	DestructibleContactModify	mContactModify;
	physx::Array<ChunkLODRef>	mNonessentialChunks;

	NxResourceList				mDestructibleActorJointList;
	physx::PxF32				mLodSumBenefit, mLodRelativeBenefit;
	physx::PxF32				mCurrentCostOfEssentialChunks;
	physx::PxF32				mCurrentCostOfAllChunks;
	physx::PxF32				mCurrentBudget;
	physx::PxU32				mLodTotalValidChunks;
	DestructibleBeforeTick*		mBeforeTickTask;

	DestructibleUserNotify      mUserNotify;
	NxApexSceneStats			mModuleSceneStats;
	physx::PxF32				mElapsedTime;

	physx::Array<DestructibleActor*> mInstancedActors;

#if APEX_RUNTIME_FRACTURE
	::physx::fracture::SimScene*	mRTScene;
#endif

	// These values are set per-scene by the destruction module
	physx::PxF32				mMassScale;
	physx::PxF32				mMassScaleInv;
	physx::PxF32				mScaledMassExponent;
	physx::PxF32				mScaledMassExponentInv;

	int							mPreviousVisibleDestructibleChunkCount;			// Must keep previous value because of our goofy cumulative-only stats system
	int							mPreviousDynamicDestructibleChunkIslandCount;	// Must keep previous value because of our goofy cumulative-only stats system

public:
	void						destroy();
	void						reset();
	NxDestructibleActorJoint*	createDestructibleActorJoint(const NxDestructibleActorJointDesc& destructibleActorJointDesc);
	NxDestructibleActor*		getDestructibleAndChunk(const NxShape* shape, physx::PxI32* chunkIndex) const;

	bool						insertDestructibleActor(NxDestructibleActor* destructible);

	bool						removeStructure(DestructibleStructure* structure, bool immediate = true);
	void						addActor(NiApexPhysXObjectDesc& desc, NxActor& actor, physx::PxF32 unscaledMass, bool isDebris);
	bool						destroyActorChunks(NxActor& actor, physx::PxU32 chunkFlag);
	void						capDynamicActorCount();
	void						removeReferencesToActor(DestructibleActor& actor);

	void						addInstancedActor(DestructibleActor* actor)
	{
		mInstancedActors.pushBack(actor);
	}

	void						releasePhysXActor(NxActor& actor);

	void						resetEmitterActors();

	NxActor*					createRoot(DestructibleStructure::Chunk& chunk, const physx::PxMat34Legacy& pose, bool dynamic, physx::PxMat34Legacy* relTM = NULL, bool fromInitialData = false);
	bool						appendShapes(DestructibleStructure::Chunk& chunk, bool dynamic, physx::PxMat34Legacy* relTM = NULL, NxActor* actor = NULL);

#if NX_SDK_VERSION_MAJOR == 2
	void						setWorldSupportPhysXScene(NxScene* physxScene)
#elif NX_SDK_VERSION_MAJOR == 3
	void						setWorldSupportPhysXScene(PxScene* physxScene)
#endif
	{
		m_worldSupportPhysXScene = physxScene;
	}

	bool						testWorldOverlap(const ConvexHull& convexHull, const physx::PxMat34Legacy& tm, const physx::PxVec3& scale, physx::PxF32 padding, const NxGroupsMask* groupsMask = NULL);

	DestructibleStructure*			getChunkStructure(DestructibleStructure::Chunk& chunk) const
	{
		return mDestructibles.direct(chunk.destructibleID)->getStructure();
	}

	DestructibleStructure::Chunk*	getChunk(const NxShape* shape) const
	{
		// TODO: this fires all the time with PhysX3
		//PX_ASSERT(mModule->owns((NxActor*)&shape->getActor()));
#if NX_SDK_VERSION_MAJOR == 2
		const NxActor* actor = &shape->getActor();
#elif NX_SDK_VERSION_MAJOR == 3
		const PxRigidActor* actor = shape->getActor();
#endif		
		if (!mModule->owns(actor))
		{
			return NULL;
		}

		const NxApexPhysXObjectDesc* shapeDesc = mModule->mSdk->getPhysXObjectInfo(shape);
		if (shapeDesc == NULL)
		{
			return NULL;
		}

		return (DestructibleStructure::Chunk*)shapeDesc->userData;
	}

	const DestructibleActor*		getChunkActor(const DestructibleStructure::Chunk& chunk) const
	{
		return mDestructibles.direct(chunk.destructibleID);
	}

	PX_INLINE void					createChunkReportDataForFractureEvent(const FractureEvent& fractureEvent,  DestructibleActor* destructible, DestructibleStructure::Chunk& chunk)
	{
		PX_UNUSED(fractureEvent);	// Keeping this parameter assuming we'll merge in a later change which requires fractureEvent
		PX_ASSERT(destructible != NULL && destructible->mDamageEventReportIndex < mDamageEventReportData.size());
		if (destructible != NULL && destructible->mDamageEventReportIndex < mDamageEventReportData.size())
		{
			ApexDamageEventReportData& damageReportData = mDamageEventReportData[destructible->mDamageEventReportIndex];
			const physx::PxU32 fractureEventIndex = damageReportData.addFractureEvent(chunk, NxApexChunkFlag::FRACTURED);
			mFractureEventCount++;
			chunk.reportID = mChunkReportHandles.size();
			IntPair& handle = mChunkReportHandles.insert();
			handle.set((physx::PxI32)destructible->mDamageEventReportIndex, (physx::PxI32)fractureEventIndex);
		}
	}

	PX_INLINE NxApexChunkData*		getChunkReportData(DestructibleStructure::Chunk& chunk, physx::PxU32 flags);

	PX_INLINE NxActor*				chunkIntact(DestructibleStructure::Chunk& chunk);

	bool							scheduleChunkShapesForDelete(DestructibleStructure::Chunk& chunk);
	bool							scheduleNxActorForDelete(NiApexPhysXObjectDesc& actorDesc);

	void							processFIFOForLOD();

	PX_INLINE physx::PxF32			calculateBenefit(const DestructibleActorParamNS::LODWeights_Type& weights, const physx::PxVec3& center, physx::PxF32 radiusSquared, const physx::PxVec3& eyePos, physx::PxF32 age) const;

	PX_INLINE void					calculatePotentialCostAndBenefit(physx::PxF32& cost, physx::PxF32& benefit, physx::Array<physx::PxU8*>& trail, physx::Array<physx::PxU8>& undo, const FractureEvent& fractureEvent) const;

	template <class ChunkOpClass>
	void							forSubtree(DestructibleStructure::Chunk& chunk, ChunkOpClass chunkOp, bool skipRoot = false);

	bool							setMassScaling(physx::PxF32 massScale, physx::PxF32 scaledMassExponent);

	void							invalidateBounds(const physx::PxBounds3* bounds, physx::PxU32 boundsCount);

	void							setDamageApplicationRaycastFlags(physx::NxDestructibleActorRaycastFlags::Enum flags);

	physx::NxDestructibleActorRaycastFlags::Enum	getDamageApplicationRaycastFlags() const { return (physx::NxDestructibleActorRaycastFlags::Enum)m_damageApplicationRaycastFlags; }

	PX_INLINE physx::PxF32			scaleMass(physx::PxF32 mass)
	{
		if (physx::PxAbs(mScaledMassExponent - 0.5f) < 1e-7)
		{
			return mMassScale * physx::PxSqrt(mass * mMassScaleInv);
		}
		return mMassScale * physx::PxPow(mass * mMassScaleInv, mScaledMassExponent);
	}

	PX_INLINE physx::PxF32			unscaleMass(physx::PxF32 scaledMass)
	{
		if (physx::PxAbs(mScaledMassExponentInv - 2.f) < 1e-7)
		{
			PxF32 tmp = scaledMass * mMassScaleInv;
			return mMassScale * tmp * tmp;
		}
		return mMassScale * physx::PxPow(scaledMass * mMassScaleInv, mScaledMassExponentInv);
	}

	void							updateActorPose(DestructibleActor* actor, UserChunkMotionHandler* callback)
	{
		const bool processChunkPoseForSyncing = ((NULL != callback) && (	actor->getSyncParams().isSyncFlagSet(NxDestructibleActorSyncFlags::CopyChunkTransform) ||
																			actor->getSyncParams().isSyncFlagSet(NxDestructibleActorSyncFlags::ReadChunkTransform)));
        actor->setRenderTMs(processChunkPoseForSyncing);
	}

	bool							setRenderLockMode(NxApexRenderLockMode::Enum renderLockMode)
	{
		if ((int)renderLockMode < 0 || (int)renderLockMode > NxApexRenderLockMode::PER_MODULE_SCENE_RENDER_LOCK)
		{
			return false;
		}

		mRenderLockMode = renderLockMode;

		return true;
	}

	NxApexRenderLockMode::Enum		getRenderLockMode() const
	{
		return mRenderLockMode;
	}

	bool							lockModuleSceneRenderLock()
	{
		return mRenderDataLock.lock();
	}

	bool							unlockModuleSceneRenderLock()
	{
		return mRenderDataLock.unlock();
	}

	bool							lockRenderResources()
	{
		switch (getRenderLockMode())
		{
		case NxApexRenderLockMode::PER_ACTOR_RENDER_LOCK:
			renderLockAllActors();
			return true;
		case NxApexRenderLockMode::PER_MODULE_SCENE_RENDER_LOCK:
			return lockModuleSceneRenderLock();
		case NxApexRenderLockMode::NO_RENDER_LOCK:
			break;
		}
		return true;
	}

	bool							unlockRenderResources()
	{
		switch (getRenderLockMode())
		{
		case NxApexRenderLockMode::PER_ACTOR_RENDER_LOCK:
			renderUnLockAllActors();
			return true;
		case NxApexRenderLockMode::PER_MODULE_SCENE_RENDER_LOCK:
			return unlockModuleSceneRenderLock();
		case NxApexRenderLockMode::NO_RENDER_LOCK:
			break;
		}
		return true;
	}

	physx::Array<ApexDamageEventReportData>	mDamageEventReportData;
	physx::Array<IntPair>					mChunkReportHandles;	// i0 = m_damageEventReportData index, i1 = ApexDamageEventReportData::m_fractureEvents

	physx::Array<NxImpactDamageEventData>	mImpactDamageEventData;	// Impact damage notify

	physx::Array<DestructibleActor*>		mActorsWithChunkStateEvents;	// Filled when module->m_chunkStateEventCallbackSchedule is not NxDestructibleCallbackSchedule::Disabled

	void setStructureSupportRebuild(DestructibleStructure* structure, bool rebuild);
	void setStructureUpdate(DestructibleStructure* structure, bool update);
	void setStressSolverTick(DestructibleStructure* structure, bool update);

	// FIFO
	physx::Array<ActorFIFOEntry>	mActorFIFO;
	physx::Array<IndexedReal>		mActorBenefitSortArray;
	physx::PxU32					mDynamicActorFIFONum;	// Tracks the valid entries in actorFIFO;
	physx::PxU32					mTotalChunkCount;		// Tracks the chunks;

	physx::PxU32					mNumFracturesProcessedThisFrame;
	physx::PxU32					mNumActorsCreatedThisFrame;

	physx::PxU32					mFractureEventCount;

	// DestructibleActors with scene-global IDs
	NxBank<DestructibleActor*, physx::PxU32>	mDestructibles;

	// Structure container
	NxBank<DestructibleStructure*, physx::PxU32>	mStructures;
	physx::Array<DestructibleStructure*>			mStructureKillList;
	NxBank<DestructibleStructure*, physx::PxU32>	mStructureUpdateList;
	NxBank<DestructibleStructure*, physx::PxU32>	mStructureSupportRebuildList;
	NxBank<DestructibleStructure*, physx::PxU32>	mStressSolverTickList;

	// For delayed release of NxActors
	physx::Array<NxActor*>			mActorKillList;
	NxResourceList					mApexActorKillList;

	// Damage queue
	NxRingBuffer<DamageEvent>		mDamageBuffer[2];	// Double-buffering
	physx::PxU32					mDamageBufferWriteIndex;

	// Fracture queue
	NxRingBuffer<FractureEvent>		mFractureBuffer;

	// Destroy queue
	physx::Array<IntPair>			mChunkKillList;

	// Destructibles on death row
	physx::Array<NxDestructibleActor*>	destructibleActorKillList;

	// Bank of dormant (kinematic dynamic) actors
	NxBank<DormantActorEntry, physx::PxU32>	mDormantActors;

	// list of awake destructible actor IDs (mDestructibles bank)
	physx::NxIndexBank<physx::PxU32> mAwakeActors; 
	bool							mUsingActiveTransforms;

	// Wake/sleep event actors
	physx::Array<physx::NxDestructibleActor*>	mOnWakeActors;
	physx::Array<physx::NxDestructibleActor*>	mOnSleepActors;

#if NX_SDK_VERSION_MAJOR == 3
	physx::Array<PxClientID>		mSceneClientIDs;
#endif

	// Scene to use for world support calculations if NULL, mPhysXScene is used
#if NX_SDK_VERSION_MAJOR == 2
	NxScene*						m_worldSupportPhysXScene;
#elif NX_SDK_VERSION_MAJOR == 3
	physx::PxScene*					m_worldSupportPhysXScene;
#endif
	physx::Array<physx::PxBounds3>	m_invalidBounds;

	physx::PxU32					m_damageApplicationRaycastFlags;

	NiApexRenderDebug*				mDebugRender;	//debug renderer created and owned by the owning ApexScene and passed down.  Not owned.

	DebugRenderParams*				mDebugRenderParams;
	DestructibleDebugRenderParams*	mDestructibleDebugRenderParams;

#if NX_SDK_VERSION_MAJOR == 3
	physx::Array<physx::PxOverlapHit>	mOverlapHits;
#endif		

	NxApexRenderLockMode::Enum		mRenderLockMode;

	physx::AtomicLock				mRenderDataLock;

	// Access to the double-buffered damage events
	NxRingBuffer<DamageEvent>&		getDamageWriteBuffer()
	{
		return mDamageBuffer[mDamageBufferWriteIndex];
	}
	NxRingBuffer<DamageEvent>&		getDamageReadBuffer()
	{
		return mDamageBuffer[mDamageBufferWriteIndex^1];
	}
	void							swapDamageBuffers()
	{
		mDamageBufferWriteIndex ^= 1;
	}

	// For visualization
	struct LODReductionData
	{
		physx::PxVec3	pos;
		physx::PxF32	benefit;
	};
	physx::Array<LODReductionData>	mLODDeletionDataList;
	physx::Array<LODReductionData>	mLODReductionDataList;

	class CollectVisibleChunks
	{
		physx::Array<DestructibleStructure::Chunk*>& chunkArray;

	public:

		CollectVisibleChunks(physx::Array<DestructibleStructure::Chunk*>& inChunkArray) :
			chunkArray(inChunkArray) {}

		bool execute(DestructibleStructure* structure, DestructibleStructure::Chunk& chunk)
		{
			PX_UNUSED(structure);
			if ((chunk.state & ChunkVisible) != 0)
			{
				chunkArray.pushBack(&chunk);
				return false;
			}
			return true;
		}
	private:
		CollectVisibleChunks& operator=(const CollectVisibleChunks&);
	};

	class VisibleChunkSetDescendents
	{
		physx::PxU32 visibleChunkIndex;
		physx::PxU8 raiseStateFlags;

	public:
		VisibleChunkSetDescendents(physx::PxU32 chunkIndex, bool makeDynamic) 
			: visibleChunkIndex(chunkIndex)
			, raiseStateFlags(makeDynamic ? (physx::PxU8)ChunkDynamic : (physx::PxU8)0)
		{
		}
		bool execute(DestructibleStructure* structure, DestructibleStructure::Chunk& chunk)
		{
			PX_UNUSED(structure);
			chunk.clearShapes();
			chunk.state |= raiseStateFlags;	// setting dynamic flag, but these chunks are descendents of a visible chunk and therefore invisible.  No need to update visible dynamic chunk shape counts
			chunk.visibleAncestorIndex = (physx::PxI32)visibleChunkIndex;
			return true;
		}
	};

	class ChunkClearDescendents
	{
		physx::PxI32 rootChunkIndex;

	public:
		ChunkClearDescendents(physx::PxI32 chunkIndex) : rootChunkIndex(chunkIndex) {}

		bool execute(DestructibleStructure* structure, DestructibleStructure::Chunk& chunk)
		{
			PX_UNUSED(structure);
			if (chunk.visibleAncestorIndex == rootChunkIndex)
			{
				chunk.clearShapes();
				chunk.visibleAncestorIndex = DestructibleStructure::InvalidChunkIndex;
			}
			return true;
		}
	};

	class ChunkLoadState
	{
	public:
		ChunkLoadState(DestructibleScene& scene) : mScene(scene) { }
		bool execute(DestructibleStructure* structure, DestructibleStructure::Chunk& chunk);
	private:
		ChunkLoadState& operator=(const ChunkLoadState&);
		DestructibleScene& mScene;
	};

private:
	void createModuleStats();
	void destroyModuleStats();
	void setStatValue(physx::PxI32 index, ApexStatValue dataVal);
	static PX_INLINE physx::PxF32 ticksToMilliseconds(physx::PxU64 t0, physx::PxU64 t1)
	{
		// this is a copy of ApexScene::tickToMilliseconds. Unfortunately can't easily use the other
		static const CounterFrequencyToTensOfNanos freq = physx::Time::getBootCounterFrequency();
		static const double freqMultiplier = (double)freq.mNumerator/(double)freq.mDenominator * 0.00001; // convert from 10nanoseconds to milliseconds

		physx::PxF32 ret = (physx::PxF32)((double)(t1 - t0) * freqMultiplier);
		return ret;
	}

	friend class DestructibleActor;
	friend class DestructibleActorProxy;
	friend class DestructibleActorJoint;
	friend class DestructibleStructure;
	friend class DestructibleContactReport;
	friend class DestructibleContactModify;
	friend class OverlapSphereShapesReport;
	friend class ModuleDestructible;
	friend class ApexDamageEventReportData;
	friend class DestructibleUserNotify;

	/*
	<--- Destructible Sync-ables Memory Layout --->

    all actor data => buffer (consists of)
        per actor data => segment (consists of)
            1) header
                1.1) userActorID
                1.2) _X_unit count, pointer to _X_buffer start
                1.3) _Y_unit count, pointer to _Y_buffer start
                1.4) pointer to next header. NULL if last
            2) _X_buffer
                2.1) made up of '_X_count' of '_X_unit'
            3) _Y_buffer (if used)
                3.1) made up of '_Y_count' of '_Y_unit'

    |-------------|----------|---------------|----------|---------------|------|-----------|-----------|-----------------
    | userActorID | _X_count | _X_buffer_ptr | _Y_count | _Y_buffer_ptr | next | _X_buffer | _Y_buffer | userActorID  ...
    |-------------|----------|---------------|----------|---------------|------|-----------|-----------|-----------------

	<------------------------------------------------------ buffer ------------------------------------------------------...
    <-------------------------------------------- segment  -------------------------------------------->
	<--------------------------------- header --------------------------------->
																			   <-----------> uniform buffer
																						   <-----------> uniform buffer


	<--- Destructible Sync-ables Function Call Flow --->

	1) write
		onProcessWriteData()
			loadDataForWrite()
				interceptLoad()
			processWriteBuffer()
				getBufferSizeRequired()
					getSegmentSizeRequired()
				callback.onWriteBegin()
				writeUserBuffer()
					writeUserSegment()
						writeUserUniformBuffer()
							interpret()
							interceptEdit()
				callback.onWriteDone()
			unloadDataForWrite()

		___buffer full___
		a)	after generateFractureProfilesInDamageBuffer()
			before fillFractureBufferFromDamage()
		b)	before processFractureBuffer()
		c)	after fetchResults()
				actor->setRenderTMs()

	2) read
		onPreProcessReadData()
			processReadBuffer()
				callback.onPreProcessReadBegin()
				writeUserBufferPointers()
				callback.onPreProcessReadDone()
				callback.onReadBegin()
			loadDataForRead()

		___begin read___
		a)	generateFractureProfilesInDamageBuffer()
				interpret()
				interceptEdit()
			fillFractureBufferFromDamage()
		b)	processFractureBuffer()
				interpret()
				interceptEdit()
		c)	fetchResults()
				actor->setRenderTMs()
		___end read___

		onPostProcessReadData()
			unloadDataForRead()
			callback.onReadDone()
	*/

    /*** DestructibleScene::SyncParams ***/
public:
    class SyncParams
    {
    public:
		struct UserDamageEvent;
		struct UserFractureEvent;
        explicit SyncParams(const ModuleDestructible::SyncParams & moduleParams);
        ~SyncParams();
	public:
		bool					setSyncActor(physx::PxU32 entryIndex, DestructibleActor * entry, DestructibleActor *& erasedEntry);
	private:
		DestructibleActor *		getSyncActor(physx::PxU32 entryIndex) const;
		physx::PxU32			getUserActorID(physx::PxU32 destructibleID) const;

	public: //read and write - public APIs
		void					onPreProcessReadData(UserDamageEventHandler & callback, const physx::Array<UserDamageEvent> *& userSource);
		void					onPreProcessReadData(UserFractureEventHandler & callback, const physx::Array<UserFractureEvent> *& userSource);
		void					onPreProcessReadData(UserChunkMotionHandler & callback);
		
		DamageEvent &			interpret(const UserDamageEvent & userDamageEvent);
		FractureEvent &			interpret(const UserFractureEvent & userFractureEvent);
		void					interceptEdit(DamageEvent & damageEvent, const DestructibleActor::SyncParams & actorParams) const;
		void					interceptEdit(FractureEvent & fractureEvent, const DestructibleActor::SyncParams & actorParams) const;

		void					onPostProcessReadData(UserDamageEventHandler & callback);
		void					onPostProcessReadData(UserFractureEventHandler & callback);
		void					onPostProcessReadData(UserChunkMotionHandler & callback);

		void					onProcessWriteData(UserDamageEventHandler & callback, const NxRingBuffer<DamageEvent> & localSource);
		void					onProcessWriteData(UserFractureEventHandler & callback, const NxRingBuffer<FractureEvent> & localSource);
		void					onProcessWriteData(UserChunkMotionHandler & callback);

	private: //read - helper functions
		void					loadDataForRead(const NxApexDamageEventHeader * bufferStart, const physx::Array<UserDamageEvent> *& userSource);
		void					loadDataForRead(const NxApexFractureEventHeader * bufferStart, const physx::Array<UserFractureEvent> *& userSource);
		void					loadDataForRead(const NxApexChunkTransformHeader * bufferStart);

		bool					unloadDataForRead(const UserDamageEventHandler & dummy);
		bool					unloadDataForRead(const UserFractureEventHandler & dummy);
		bool					unloadDataForRead(const UserChunkMotionHandler & dummy);

		template<typename Callback, typename Header, typename Unit>	const Header *	processReadBuffer(Callback & callback) const;
		template<typename Header, typename Unit>					physx::PxI32	writeUserBufferPointers(Header * bufferStart, physx::PxU32 bufferSize) const;
		template<typename Callback>									void			postProcessReadData(Callback & callback);

	private: //write - helper functions
		physx::PxU32			loadDataForWrite(const NxRingBuffer<DamageEvent> & localSource);
		physx::PxU32			loadDataForWrite(const NxRingBuffer<FractureEvent> & localSource);
		physx::PxU32			loadDataForWrite(const UserChunkMotionHandler & dummy);

		void					unloadDataForWrite(const UserDamageEventHandler & dummy);
		void					unloadDataForWrite(const UserFractureEventHandler & dummy);
		void					unloadDataForWrite(const UserChunkMotionHandler & dummy);

        template<typename Callback, typename Header, typename Unit>	physx::PxU32	processWriteBuffer(Callback & callback);
		template<typename Header, typename Unit>					physx::PxU32	getBufferSizeRequired() const;
		template<typename Header, typename Unit>					physx::PxU32	getSegmentSizeRequired(const DestructibleActor::SyncParams & actorParams) const;
		template<typename Header, typename Unit>					physx::PxU32	writeUserBuffer(Header * bufferStart, physx::PxU32 & headerCount);
		template<typename Header, typename Unit>					physx::PxU32	writeUserSegment(Header * header, bool isLast, const DestructibleActor::SyncParams & actorParams);
		template<typename Unit>										physx::PxU32	writeUserUniformBuffer(Unit * bufferStart, const DestructibleActor::SyncParams & actorParams);
		
		bool								interceptLoad(const DamageEvent & damageEvent, const DestructibleActor & syncActor) const;
		bool								interceptLoad(const FractureEvent & fractureEvent, const DestructibleActor & syncActor) const;
		NxApexDamageEventUnit *				interpret(const DamageEvent & damageEvent);
		NxApexFractureEventUnit *			interpret(const FractureEvent & fractureEvent);
		void								interceptEdit(NxApexDamageEventUnit & nxApexDamageEventUnit, const DestructibleActor::SyncParams & actorParams) const;
		void								interceptEdit(NxApexFractureEventUnit & nxApexFractureEventUnit, const DestructibleActor::SyncParams & actorParams) const;

	private: //read and write - template helper functions
		template<typename Item>		physx::PxU32	getUniformBufferCount(const NxApexDamageEventHeader &		header) const;
		template<typename Item>		physx::PxU32	getUniformBufferCount(const NxApexFractureEventHeader &		header) const;
		template<typename Item>		physx::PxU32	getUniformBufferCount(const NxApexChunkTransformHeader &	header) const;

		template<typename Item>		physx::PxU32 &	getUniformBufferCountMutable(NxApexDamageEventHeader &		header) const;
		template<typename Item>		physx::PxU32 &	getUniformBufferCountMutable(NxApexFractureEventHeader &	header) const;
		template<typename Item>		physx::PxU32 &	getUniformBufferCountMutable(NxApexChunkTransformHeader &	header) const;

		template<typename Item>		Item *			getUniformBufferStart(const NxApexDamageEventHeader &		header) const;
		template<typename Item>		Item *			getUniformBufferStart(const NxApexFractureEventHeader &		header) const;
		template<typename Item>		Item *			getUniformBufferStart(const NxApexChunkTransformHeader &	header) const;

		template<typename Item>		Item *&			getUniformBufferStartMutable(NxApexDamageEventHeader &		header) const;
		template<typename Item>		Item *&			getUniformBufferStartMutable(NxApexFractureEventHeader &	header) const;
		template<typename Item>		Item *&			getUniformBufferStartMutable(NxApexChunkTransformHeader &	header) const;

#if 1
    public:
        bool ditchStaleBuffers;
		bool lockSyncParams;
#endif // lionel: TODO...if necessary

    private:
		SyncParams();
        DECLARE_DISABLE_COPY_AND_ASSIGN(SyncParams);
		const ModuleDestructible::SyncParams &			moduleParams;
		mutable physx::Array<physx::PxU32>				segmentSizeChecker;
		const NxRingBuffer<DamageEvent>	*				damageEventWriteSource;
		NxApexDamageEventUnit							damageEventUserInstance;
		physx::Array<UserDamageEvent>					damageEventReadSource;
		DamageEvent										damageEventImplInstance;
		const NxRingBuffer<FractureEvent> *				fractureEventWriteSource;
        NxApexFractureEventUnit							fractureEventUserInstance;
		physx::Array<UserFractureEvent>					fractureEventReadSource;
        FractureEvent									fractureEventImplInstance;
		physx::Array<DestructibleStructure::Chunk*>		chunksWithUserControlledChunk;

		class SyncActorRecord
		{
			friend bool DestructibleScene::SyncParams::setSyncActor(physx::PxU32, DestructibleActor *, DestructibleActor *&);
		public:
			SyncActorRecord();
			~SyncActorRecord();
			const physx::Array<physx::PxU32> &			getIndexContainer() const;
			const physx::Array<DestructibleActor*> &	getActorContainer() const;
		private:
			DECLARE_DISABLE_COPY_AND_ASSIGN(SyncActorRecord);
			void										onRebuild(const physx::PxU32 & newCount);
			physx::Array<physx::PxU32>					indexContainer;
			physx::Array<DestructibleActor*>			actorContainer;
		private:
			bool										assertActorContainerOk() const;
		}												syncActorRecord;

		struct ActorEventPair
		{
		public:
			const DestructibleActor *			actorAlias;
		protected:
			union
			{
			public:
				const NxApexDamageEventUnit *	userDamageEvent;
				const NxApexFractureEventUnit *	userFractureEvent;
			};
			ActorEventPair(const DestructibleActor * actorAlias, const NxApexDamageEventUnit * userDamageEvent);
			ActorEventPair(const DestructibleActor * actorAlias, const NxApexFractureEventUnit * userFractureEvent);
			~ActorEventPair();
		private:
			ActorEventPair();
		};

	public:
		struct UserDamageEvent : public ActorEventPair
		{
		public:
			UserDamageEvent(const DestructibleActor * actorAlias, const NxApexDamageEventUnit * userDamageEvent);
			~UserDamageEvent();
			const NxApexDamageEventUnit *		get() const;
		private:
			UserDamageEvent();
		};

		struct UserFractureEvent : public ActorEventPair
		{
		public:
			UserFractureEvent(const DestructibleActor * actorAlias, const NxApexFractureEventUnit * userFractureEvent);
			~UserFractureEvent();
			const NxApexFractureEventUnit *		get() const;
		private:
			UserFractureEvent();
		};

	public:
		bool				assertUserDamageEventOk(const DamageEvent & damageEvent, const DestructibleScene & scene) const;
		bool				assertUserFractureEventOk(const FractureEvent & fractureEvent, const DestructibleScene & scene) const;
	private:
		bool				assertCachedChunkContainerOk(const DestructibleActor::SyncParams & actorParams) const;
		bool				assertControlledChunkContainerOk() const;
	private:
		static const char *	errorMissingUserActorID;
		static const char *	errorMissingFlagReadDamageEvents;
		static const char *	errorMissingFlagReadFractureEvents;
		static const char *	errorMissingFlagReadChunkMotion;
		static const char *	errorOutOfBoundsActorChunkIndex;
		static const char *	errorOutOfBoundsStructureChunkIndex;
		static const char *	errorOverrunBuffer;
    };

	const DestructibleScene::SyncParams &		getSyncParams() const;
	DestructibleScene::SyncParams &				getSyncParamsMutable();
private:
	SyncParams									mSyncParams;
	
	//these methods participate in the sync process
private:
	physx::PxF32								processEventBuffers(physx::PxF32 resourceBudget);
	void										generateFractureProfilesInDamageBuffer(physx::NxRingBuffer<DamageEvent> & userDamageBuffer, const physx::Array<SyncParams::UserDamageEvent> * userSource = NULL);
	physx::PxF32								setDamageBufferBudget(physx::PxF32 budget, physx::NxRingBuffer<DamageEvent> * userDamageBuffer = NULL);
	void										calculateDamageBufferCostProfiles(physx::PxF32 & minCost, physx::PxF32 & maxCost, physx::NxRingBuffer<DamageEvent> & subjectDamageBuffer);
	void										fillFractureBufferFromDamage(physx::NxRingBuffer<DamageEvent> * userDamageBuffer = NULL);
	void										processFractureBuffer();
	void										processFractureBuffer(const physx::Array<SyncParams::UserFractureEvent> * userSource);
	void										processDamageColoringBuffer();

private:
	NxRingBuffer<FractureEvent>					mDeprioritisedFractureBuffer;
	NxRingBuffer<FractureEvent>					mDeferredFractureBuffer;
	NxRingBuffer<SyncDamageEventCoreDataParams>	mSyncDamageEventCoreDataBuffer;
};

template <class ChunkOpClass>
PX_INLINE void DestructibleScene::forSubtree(DestructibleStructure::Chunk& chunk, ChunkOpClass chunkOp, bool skipRoot)
{
	physx::PxU16 indexInAsset = chunk.indexInAsset;
	DestructibleStructure::Chunk* pProcessChunk = &chunk;
	physx::Array<physx::PxU16> stack;

	DestructibleActor* destructible = mDestructibles.direct(chunk.destructibleID);
	const DestructibleAssetParametersNS::Chunk_Type* sourceChunks = destructible->getAsset()->mParams->chunks.buf;
	const physx::PxU32 firstChunkIndex = destructible->getFirstChunkIndex();

	for (;;)
	{
		bool recurse = skipRoot ? true : chunkOp.execute(destructible->getStructure(), *pProcessChunk);
		skipRoot = false;
		const DestructibleAssetParametersNS::Chunk_Type& source = sourceChunks[indexInAsset];
		if (recurse && source.numChildren)
		{
			for (indexInAsset = PxU16(source.firstChildIndex + source.numChildren); --indexInAsset > source.firstChildIndex;)
			{
				stack.pushBack(indexInAsset);
			}
		}
		else
		{
			if (stack.size() == 0)
			{
				return;
			}
			indexInAsset = stack.back();
			stack.popBack();
		}
		pProcessChunk = destructible->getStructure()->chunks.begin() + (firstChunkIndex + indexInAsset);
	}
}

PX_INLINE NxApexChunkData* DestructibleScene::getChunkReportData(DestructibleStructure::Chunk& chunk, physx::PxU32 flags)
{
	if (!mModule->m_chunkReport)
	{
		return NULL;	// don't give out data if we won't be reporting it
	}

	if (chunk.reportID == InvalidReportID)
	{
		// No chunk report data.  Create.
		const physx::PxU32 damageEventIndex = mDamageEventReportData.size();
		ApexDamageEventReportData& damageReportData = mDamageEventReportData.insert();
		damageReportData.setDestructible(mDestructibles.direct(chunk.destructibleID));
		const physx::PxU32 fractureEventIndex = damageReportData.addFractureEvent(chunk, flags);
		chunk.reportID = mChunkReportHandles.size();
		IntPair& handle = mChunkReportHandles.insert();
		handle.set((physx::PxI32)damageEventIndex, (physx::PxI32)fractureEventIndex);
	}

	PX_ASSERT(chunk.reportID < mChunkReportHandles.size());
	if (chunk.reportID < mChunkReportHandles.size())
	{
		IntPair& handle = mChunkReportHandles[chunk.reportID];
		PX_ASSERT((physx::PxU32)handle.i0 < mDamageEventReportData.size());
		if ((physx::PxU32)handle.i0 < mDamageEventReportData.size() && (physx::PxU32)handle.i1 != 0xFFFFFFFF)
		{
			return &mDamageEventReportData[(physx::PxU32)handle.i0].getFractureEvent((physx::PxU32)handle.i1);
		}
	}

	return NULL;
}

PX_INLINE NxActor* DestructibleScene::chunkIntact(DestructibleStructure::Chunk& chunk)
{
	DestructibleStructure::Chunk* child = &chunk;
	DestructibleActor* destructible = mDestructibles.direct(chunk.destructibleID);

	if (!chunk.isDestroyed())
	{
		return (NxActor*)destructible->getStructure()->getChunkActor(chunk);
	}

	if ((chunk.flags & ChunkMissingChild) != 0)
	{
		return NULL;
	}

	DestructibleAsset* asset = destructible->getAsset();
	do
	{
		DestructibleAssetParametersNS::Chunk_Type& source = asset->mParams->chunks.buf[child->indexInAsset];
		PX_ASSERT(source.numChildren > 0);
		child = &destructible->getStructure()->chunks[destructible->getFirstChunkIndex() + source.firstChildIndex];

		if (!child->isDestroyed())
		{
			return (NxActor*)destructible->getStructure()->getChunkActor(chunk);
		}
	}
	while ((child->flags & ChunkMissingChild) == 0);

	return NULL;
}

PX_INLINE physx::PxF32 DestructibleScene::calculateBenefit(const DestructibleActorParamNS::LODWeights_Type& weights, const physx::PxVec3& center, physx::PxF32 radiusSquared, const physx::PxVec3& eyePos, physx::PxF32 age) const
{
	const PxF32 eyeDist2 = (eyePos - center).magnitudeSquared();
	const PxF32 distanceBenefit = solidAngleFiniteRangeImportance(eyeDist2, radiusSquared, weights.maxDistance * weights.maxDistance);
	const PxF32 ageBenefit = ageImportance(age, weights.maxAge);
	return weights.distanceWeight * distanceBenefit + weights.ageWeight * ageBenefit + weights.bias;
}

PX_INLINE void cacheChunkTempState(DestructibleStructure::Chunk& chunk, physx::Array<physx::PxU8*>& trail, physx::Array<physx::PxU8>& undo)
{
	// ChunkTemp0 = chunk state has been cached
	// ChunkTemp1 = chunk exists
	// ChunkTemp2 = chunk is visible
	// ChunkTemp3 = chunk is dynamic

	undo.pushBack((physx::PxU8)(chunk.state & (physx::PxU8)ChunkTempMask));

	if ((chunk.state & ChunkTemp0) == 0)
	{
		chunk.state |= ChunkTemp0;
		if (!chunk.isDestroyed())
		{
			chunk.state |= ChunkTemp1;
			if (chunk.state & ChunkVisible)
			{
				chunk.state |= ChunkTemp2;
			}
			if (chunk.state & ChunkDynamic)
			{
				chunk.state |= ChunkTemp3;
			}
		}
		trail.pushBack(&chunk.state);
	}
}

PX_INLINE void DestructibleScene::calculatePotentialCostAndBenefit(physx::PxF32& cost, physx::PxF32& benefit, physx::Array<physx::PxU8*>& trail, physx::Array<physx::PxU8>& undo, const FractureEvent& fractureEvent) const
{
	cost = 0.0f;
	benefit = 0.0f;

	DestructibleActor* destructible = mDestructibles.direct(fractureEvent.destructibleID);
	if (destructible == NULL)
	{
		return;
	}

	DestructibleStructure* structure = destructible->getStructure();
	if (structure == NULL)
	{
		return;
	}

	DestructibleStructure::Chunk* chunk = structure->chunks.begin() + fractureEvent.chunkIndexInAsset + destructible->getFirstChunkIndex();

	// ChunkTemp0 = chunk state has been cached
	// ChunkTemp1 = chunk exists
	// ChunkTemp2 = chunk is visible
	// ChunkTemp3 = chunk is dynamic

	cacheChunkTempState(*chunk, trail, undo);

	if ((chunk->state & ChunkTemp1) == 0)
	{
		return;
	}

	if ((chunk->state & ChunkTemp3) != 0)	// If the chunk is dynamic, we're done... no extra cost or benefit, and it's disconnected from its parent
	{
		return;
	}

	const physx::PxU32 chunkIndexInAsset = fractureEvent.chunkIndexInAsset;
	const physx::PxU32 chunkShapeCount = destructible->getAsset()->getChunkHullCount(chunkIndexInAsset);
	const physx::PxF32 chunkCost = chunkShapeCount * mModule->getLODUnitCost();

	cost += chunkCost;
	benefit += destructible->getBenefit()*(physx::PxF32)chunkShapeCount/(physx::PxF32)physx::PxMax<physx::PxU32>(destructible->getVisibleDynamicChunkShapeCount(), 1);

	chunk->state |= ChunkTemp3;	// Mark it as dynamic

	if ((chunk->state & ChunkTemp2) != 0)
	{
		return;	// It was visible already, so we just made it dynamic.  It will have no parent and iits siblings will already be accounted for, if they exist.
	}

	chunk->state |= ChunkTemp2;	// Mark it as visible

	for (;;)
	{
		// Climb the hierarchy
		DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk->indexInAsset];
		if (source.parentIndex == DestructibleAsset::InvalidChunkIndex)
		{
			break;
		}
		chunk = structure->chunks.begin() + (source.parentIndex + destructible->getFirstChunkIndex());
		cacheChunkTempState(*chunk, trail, undo);
		if ((chunk->state & ChunkTemp1) == 0)
		{
			break;	// Parent doesn't exist
		}

		// Make children visible
		DestructibleAssetParametersNS::Chunk_Type& parentSource = destructible->getAsset()->mParams->chunks.buf[chunk->indexInAsset];
		const physx::PxU32 firstChildIndex = parentSource.firstChildIndex + destructible->getFirstChunkIndex();
		const physx::PxU32 endChildIndex = firstChildIndex + parentSource.numChildren;
		for (physx::PxU32 childIndex = firstChildIndex; childIndex < endChildIndex; ++childIndex)
		{
			DestructibleStructure::Chunk& child = structure->chunks[childIndex];
			cacheChunkTempState(child, trail, undo);
			if ((child.state & ChunkTemp1) == 0)
			{
				continue; // Child doesn't exist, skip
			}
			child.state |= ChunkTemp2;	// Make it visible
			if ((chunk->state & ChunkTemp3) != 0 || (child.flags & ChunkBelowSupportDepth) != 0)
			{
				// Add cost and benefit for children
				const physx::PxU32 childndexInAsset = childIndex - destructible->getFirstChunkIndex();
				const physx::PxU32 childShapeCount = destructible->getAsset()->getChunkHullCount(childndexInAsset);
				const physx::PxF32 childCost = childShapeCount * mModule->getLODUnitCost();

				cost += childCost;
				benefit += destructible->getBenefit()*(physx::PxF32)childShapeCount/(physx::PxF32)physx::PxMax<physx::PxU32>(destructible->getVisibleDynamicChunkShapeCount(), 1);

				// Parent is dynamic (so its children will be too), or the child is below the support depth, so make it dynamic
				child.state |= ChunkTemp3;
			}
		}

		chunk->state &= ~(physx::PxU8)(ChunkTemp1 | ChunkTemp2 | ChunkTemp3);	// The parent will cease to exist
	}
}

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLE_SCENE_H__

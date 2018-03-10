/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEACTOR_H__
#define __DESTRUCTIBLEACTOR_H__

#include "NxApex.h"
#include "ApexInterface.h"
#include "ApexActor.h"
#include "DestructibleAssetProxy.h"
#include "DestructibleActorState.h"
#include "NxDestructibleActor.h"
#include "DestructibleStructure.h"
#include "NiApexRenderMeshAsset.h"
#include "NiResourceProvider.h"
#include "NxFromPx.h"
#include "PhysX3ClassWrapper.h"
#include "DestructibleRenderable.h"
#if APEX_RUNTIME_FRACTURE
#include "SimScene.h"
#include "Actor.h"
#endif

namespace physx
{
namespace apex
{

class NiApexPhysXObjectDesc;
class NxApexEmitterActor;

namespace destructible
{

class DestructibleScene;
class DestructibleActorProxy;
class DestructibleActorJoint;

struct PhysXActorFlags
{
	enum Enum
	{
		DEPTH_PARAM_USER_FLAG_0 = 0,
		DEPTH_PARAM_USER_FLAG_1,
		DEPTH_PARAM_USER_FLAG_2,
		DEPTH_PARAM_USER_FLAG_3,
		CREATED_THIS_FRAME,
		IS_SLEEPING // external sleep state tracking to be in sync with mAwakeActorCount that is updated in onWake/onSleep callbacks
	};
};

class DestructibleActor : public ApexResource
	, public ApexActorSource
	, public ApexActor
	, public NxParameterized::SerializationCallback
{
public:

	friend class DestructibleActorProxy;
	friend class DestructibleRenderable;
	friend class DestructibleScopedReadLock;
	friend class DestructibleScopedWriteLock;

	enum
	{
		InvalidID =	0xFFFFFFFF
	};

	enum Flag
	{
		Dynamic	=	(1 << 0)
	};

	enum InternalFlag
	{
		IslandMarker =	(1 << 15)
	};

public:
	
	void							setState(NxParameterized::Interface* state);
	const DestructibleActorState*	getState() const					{ return mState; }
	const DestructibleActorChunks*  getChunks() const					{ return mChunks; }
	      DestructibleActorChunks*  getChunks() 						{ return mChunks; }
	const DestructibleActorParam*	getParams() const					{ return mParams; }
	      DestructibleActorParam*	getParams()							{ return mParams; }
	const NxDestructibleParameters&	getDestructibleParameters() const	{ return mDestructibleParameters; }
	      NxDestructibleParameters&	getDestructibleParameters() 		{ return mDestructibleParameters; }
	void							setDestructibleParameters(const NxDestructibleParameters& destructibleParameters);
	const DestructibleScene*		getDestructibleScene() const		{ return mDestructibleScene; }
	      DestructibleScene*		getDestructibleScene()				{ return mDestructibleScene; }
	const NxDestructibleActor*		getAPI() const						{ return mAPI; }
	      NxDestructibleActor*		getAPI()							{ return mAPI; }
	
	void					incrementWakeCount(void);
	void					decrementWakeCount(void);
	void					removeActorAtIndex(physx::PxU32 index);

	void					setPhysXScene(NxScene*);
	NxScene*				getPhysXScene() const;

	void					release();
	void					destroy();
	void					reset();
	
	virtual void			getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const;
	virtual physx::PxF32	getActivePhysicalLod() const;
	virtual void			forcePhysicalLod(physx::PxF32 lod);

	void					cacheModuleData() const;
	void					removeSelfFromStructure();
	void					removeSelfFromScene();
	bool					isInitiallyDynamic() const				{ return 0 != (getFlags() & (physx::PxU32)Dynamic); }
	physx::PxU16			getFlags() const						{ return mFlags; }
	physx::PxU16			getInternalFlags() const				{ return mInternalFlags; }
	physx::PxU16&			getInternalFlags()						{ return mInternalFlags; }
	DestructibleStructure*		getStructure()						{ return mStructure; }
	const DestructibleStructure*getStructure() const				{ return mStructure; }
	void 					setStructure(DestructibleStructure* s)	{ mStructure = s; }
	DestructibleAsset*			getAsset()							{ return mAsset; }
	const DestructibleAsset*	getAsset() const					{ return mAsset; }
	physx::PxU32				getID() const						{ return mID; }
	physx::PxU32&				getIDRef()							{ return mID; }
	const physx::PxBounds3&		getOriginalBounds() const			{ return mOriginalBounds; }
	physx::PxU32				getFirstChunkIndex() const			{ return mFirstChunkIndex; }
	void						setFirstChunkIndex(physx::PxU32 i)	{ mFirstChunkIndex = i; }
	const NxApexEmitterActor*	getCrumbleEmitter() const			{ return mCrumbleEmitter; }
	      NxApexEmitterActor*	getCrumbleEmitter()					{ return mCrumbleEmitter; }
		  physx::PxF32			getCrumbleParticleSpacing() const;
	const NxApexEmitterActor*	getDustEmitter() const				{ return mDustEmitter; }
	      NxApexEmitterActor*	getDustEmitter()					{ return mDustEmitter; }
		  physx::PxF32			getDustParticleSpacing() const;
	const physx::PxMat44&	getInitialGlobalPose() const			{ return mTM; }
	void					setInitialGlobalPose(const physx::PxMat44& pose);
	const physx::PxVec3&	getScale() const						{ return mParams->scale; }
	void					setCrumbleEmitterEnabled(bool enabled)	{ mState->enableCrumbleEmitter = enabled; }
	bool					isCrumbleEmitterEnabled() const			{ return mState->enableCrumbleEmitter; }
	void					setDustEmitterEnabled(bool enabled)		{ mState->enableDustEmitter = enabled; }
	bool					isDustEmitterEnabled() const			{ return mState->enableDustEmitter; }
	void					setCrumbleEmitterName(const char*);
	const char*				getCrumbleEmitterName() const;
	void					setDustEmitterName(const char*);
	const char*				getDustEmitterName() const;

	physx::PxU32			getSupportDepth() const					{ return mParams->supportDepth; }
	bool					formExtendedStructures() const			{ return mParams->formExtendedStructures; }
	bool					performDetailedOverlapTestForExtendedStructures() const { return mParams->performDetailedOverlapTestForExtendedStructures; }
	bool					useAssetDefinedSupport() const			{ return mParams->useAssetDefinedSupport; }
	bool					useWorldSupport() const					{ return mParams->useWorldSupport; }
	bool					drawStaticChunksInSeparateMesh() const	{ return mParams->renderStaticChunksSeparately; }
	bool					keepVisibleBonesPacked() const			{ return mParams->keepVisibleBonesPacked; }
	bool					createChunkEvents() const				{ return mParams->createChunkEvents; }
	bool					keepPreviousFrameBoneBuffer() const		{ return mParams->keepPreviousFrameBoneBuffer; }
	physx::PxF32			getSleepVelocityFrameDecayConstant() const	{ return mParams->sleepVelocityFrameDecayConstant; }
	bool					useHardSleeping() const					{ return mParams->useHardSleeping; }
	bool					useStressSolver() const					{ return mParams->structureSettings.useStressSolver; }
	physx::PxF32			getStressSolverTimeDelay() const		{ return mParams->structureSettings.stressSolverTimeDelay; }
	physx::PxF32			getStressSolverMassThreshold() const	{ return mParams->structureSettings.stressSolverMassThreshold; }

	void					enableHardSleeping();
	void					disableHardSleeping(bool wake);
	bool					setChunkPhysXActorAwakeState(physx::PxU32 chunkIndex, bool awake);
	bool					addForce(PxU32 chunkIndex, const PxVec3& force, physx::PxForceMode::Enum mode, const PxVec3* position = NULL, bool wakeup = true);

	void					setLODWeights(physx::PxF32 maxDistance, physx::PxF32 distanceWeight, physx::PxF32 maxAge, physx::PxF32 ageWeight, physx::PxF32 bias);
	const DestructibleActorParamNS::LODWeights_Type& getLODWeights(){ return reinterpret_cast<DestructibleActorParamNS::LODWeights_Type&>(mState->internalLODWeights); }
	physx::PxU32			getLOD() const { return mState->lod; }

	physx::PxU32			getRenderSubmeshCount() 
	{
		return mAsset->getRenderMeshAsset()->getSubmeshCount();
	}
	physx::PxU32			getAwakeActorCount() const { return mAwakeActorCount; }
#if APEX_RUNTIME_FRACTURE
	::physx::fracture::Actor*		getRTActor()
	{
		return mRTActor;
	}
#endif

	void					setGlobalPose(const physx::PxMat44& pose);
	void					setGlobalPoseForStaticChunks(const physx::PxMat44& pose);
	bool					getGlobalPoseForStaticChunks(physx::PxMat44& pose) const;

	void					setChunkPose(physx::PxU32 index, physx::PxMat44 worldPose);
	void					setLinearVelocity(const physx::PxVec3& linearVelocity);
	void					setAngularVelocity(const physx::PxVec3& angularVelocity);

	void					setDynamic(physx::PxI32 chunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX, bool immediate = false);
	bool					getDynamic(physx::PxI32 chunkIndex) const
	{
		PX_ASSERT(mAsset != NULL && mStructure != NULL && chunkIndex >= 0 && chunkIndex < (physx::PxI32)mAsset->getChunkCount());
		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + chunkIndex];
		return (chunk.state & ChunkDynamic) != 0;
	}

	void					getChunkVisibilities(physx::PxU8* visibilityArray, physx::PxU32 visibilityArraySize) const;
	void					setChunkVisibility(physx::PxU16 index, bool visibility);

	// Chunk event buffer API
	bool					acquireChunkEventBuffer(const NxDestructibleChunkEvent*& buffer, physx::PxU32& bufferSize);
	bool					releaseChunkEventBuffer(bool clearBuffer = true);

	// PhysX actor buffer API
#if NX_SDK_VERSION_MAJOR == 2
	bool					acquirePhysXActorBuffer(NxActor**& buffer, physx::PxU32& bufferSize, physx::PxU32 flags);
#elif NX_SDK_VERSION_MAJOR == 3
	bool					acquirePhysXActorBuffer(physx::PxRigidDynamic**& buffer, physx::PxU32& bufferSize, physx::PxU32 flags);
#endif
	bool					releasePhysXActorBuffer();

	physx::PxF32			getContactReportThreshold(const DestructibleStructure::Chunk& chunk) const
	{
		const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = getBehaviorGroup(chunk.indexInAsset);
		return getContactReportThreshold(behaviorGroup);
	}

	physx::PxF32			getContactReportThreshold(const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup) const
	{
		physx::PxF32 contactReportThreshold = PX_MAX_F32;
		if (mDestructibleParameters.forceToDamage > 0)
		{
			const physx::PxF32 maxEstimatedTimeStep = 0.1f;
			const physx::PxF32 thresholdFraction = 0.5f;
			const physx::PxF32 damageThreshold = behaviorGroup.damageThreshold;
			contactReportThreshold = thresholdFraction * maxEstimatedTimeStep * damageThreshold / mDestructibleParameters.forceToDamage;
		}

		return contactReportThreshold;
	}

	physx::PxF32		getAge(physx::PxF32 elapsedTime) const		{ return elapsedTime - mStartTime; }

	bool				getUseLegacyChunkBoundsTesting() const;
	bool				getUseLegacyDamageRadiusSpread() const;

	const NxIndexBank<physx::PxU16>&	getStaticRoots() const			{ return mStaticRoots; }
	      NxIndexBank<physx::PxU16>&	getStaticRoots()				{ return mStaticRoots; }

	physx::PxU32			getNumVisibleChunks() const		{ return mVisibleChunks.usedCount(); }
	const physx::PxU16*		getVisibleChunks() const		{ return mVisibleChunks.usedIndices(); }
	bool					initializedFromState() const 	{ return mInitializedFromState; }

	physx::PxU32			getVisibleDynamicChunkShapeCount() const			{ return mVisibleDynamicChunkShapeCount; }
	physx::PxU32			getEssentialVisibleDynamicChunkShapeCount() const	{ return mEssentialVisibleDynamicChunkShapeCount; }

	void					increaseVisibleDynamicChunkShapeCount(const physx::PxU32 number)			{ mVisibleDynamicChunkShapeCount += number; }
	void					increaseEssentialVisibleDynamicChunkShapeCount(const physx::PxU32 number) 	{ mEssentialVisibleDynamicChunkShapeCount += number; }

	void					initializeChunk(physx::PxU32 index, DestructibleStructure::Chunk& chunk) const;
	bool					getInitialChunkDynamic(physx::PxU32 index) const;
	bool					getInitialChunkVisible(physx::PxU32 index) const;
	bool					getInitialChunkDestroyed(physx::PxU32 index) const;
	physx::PxTransform		getInitialChunkGlobalPose(physx::PxU32 index) const;
	physx::PxTransform		getInitialChunkLocalPose(physx::PxU32 index) const;
	physx::PxVec3			getInitialChunkLinearVelocity(physx::PxU32 index) const;
	physx::PxVec3			getInitialChunkAngularVelocity(physx::PxU32 index) const;
	physx::PxMat44			getChunkPose(physx::PxU32 index) const;
	physx::PxTransform		getChunkTransform(physx::PxU32 index) const;
	physx::PxVec3			getChunkLinearVelocity(physx::PxU32 index) const;
	physx::PxVec3			getChunkAngularVelocity(physx::PxU32 index) const;
	const NxActor*			getChunkActor(physx::PxU32 index) const;
	NxActor*				getChunkActor(physx::PxU32 index);
#if NX_SDK_VERSION_MAJOR == 2
	physx::PxU32			getChunkPhysXShapes(NxShape**& shapes, physx::PxU32 chunkIndex) const;
#elif NX_SDK_VERSION_MAJOR == 3
	physx::PxU32			getChunkPhysXShapes(physx::PxShape**& shapes, physx::PxU32 chunkIndex) const;
#endif
	physx::PxU32			getChunkCount() const { return getAsset()->getChunkCount(); }
	const DestructibleStructure::Chunk& getChunk(physx::PxU32 index) const;

	void					setBenefit(physx::PxF32 benefit)	{ mBenefit = benefit; }
	physx::PxF32			getBenefit() const					{ return mBenefit; }

	physx::PxF32			getChunkMass(physx::PxU32 index) const
	{
		physx::PxF32 volume = 0.0f;
		for (physx::PxU32 hullIndex = mAsset->getChunkHullIndexStart(index); hullIndex < mAsset->getChunkHullIndexStop(index); ++hullIndex)
		{
			volume += physx::PxAbs(mAsset->chunkConvexHulls[hullIndex].mParams->volume);
		}

		// override actor descriptor if (behaviorGroup.density > 0)
		const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = getBehaviorGroup(index);
		PxReal density = behaviorGroup.density;
		if (density == 0)
		{
#if NX_SDK_VERSION_MAJOR == 2
			density = actorTemplate.data.density;
#elif NX_SDK_VERSION_MAJOR == 3
			density = physX3Template.data.density;
#endif
		}
		return volume * density * physx::PxAbs(getScale().x * getScale().y * getScale().z);
	}

	const physx::PxMat44	getChunkTM(physx::PxU32 index) const
	{
		NxDestructibleActorMeshType::Enum typeN;
		if (getDynamic((physx::PxI32)index) || !drawStaticChunksInSeparateMesh())
		{
			typeN = NxDestructibleActorMeshType::Skinned;
		}
		else
		{
			typeN = NxDestructibleActorMeshType::Static;
			index = 0;	// Static rendering only has one transform
		}
		NxRenderMeshActor* rma = getRenderMeshActor(typeN);
		if (rma != NULL)
		{
			return rma->getTM(mAsset->getPartIndex(index));
		}
		return physx::PxMat44::createIdentity();
	}

	physx::PxI32			getBehaviorGroupIndex(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mAsset->mParams->chunks.arraySizes[0]);
		const DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mAsset->mParams->chunks.buf[chunkIndex];

		return (physx::PxI32)sourceChunk.behaviorGroupIndex;
	}

	physx::PxU32			getChunkActorFlags(physx::PxU32 chunkIndex) const;

	const DestructibleActorParamNS::BehaviorGroup_Type& getBehaviorGroupImp(physx::PxI8 behaviorGroupIndex) const
	{
		if (behaviorGroupIndex == -1)
		{
			return mParams->defaultBehaviorGroup;
		}
		else
		{
			PX_ASSERT(behaviorGroupIndex < (physx::PxU16)mParams->behaviorGroups.arraySizes[0]);
			return mParams->behaviorGroups.buf[behaviorGroupIndex];
		}
	}

	const DestructibleActorParamNS::BehaviorGroup_Type& getBehaviorGroup(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mAsset->mParams->chunks.arraySizes[0]);
		const DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mAsset->mParams->chunks.buf[chunkIndex];

		return getBehaviorGroupImp(sourceChunk.behaviorGroupIndex);
	}

	const DestructibleActorParamNS::BehaviorGroup_Type& getRTFractureBehaviorGroup() const
	{
		return getBehaviorGroupImp(mAsset->mParams->RTFractureBehaviorGroup);
	}

	void					setSkinnedOverrideMaterial(PxU32 index, const char* overrideMaterialName);
	void					setStaticOverrideMaterial(PxU32 index, const char* overrideMaterialName);
	void					setRuntimeFracturePattern(const char* fracturePatternName);

	void					updateRenderResources(bool rewriteBuffers, void* userRenderData);
	void					dispatchRenderResources(NxUserRenderer& renderer);

	void					applyDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, const physx::PxVec3& direction, physx::PxI32 chunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX, void* userData = NULL);
	void					applyRadiusDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, physx::PxF32 radius, bool falloff, void* userData = NULL);
#if NX_SDK_VERSION_MAJOR == 2
	void					takeImpact(const physx::PxVec3& force, const physx::PxVec3& position, physx::PxU16 chunkIndex, NxActor const* damageActor);
#elif NX_SDK_VERSION_MAJOR == 3
	void					takeImpact(const physx::PxVec3& force, const physx::PxVec3& position, physx::PxU16 chunkIndex, PxActor const* damageActor);
#endif

	bool					takesImpactDamageAtDepth(physx::PxU32 depth)
	{
		if (depth < mDestructibleParameters.depthParametersCount)
		{
			if (mDestructibleParameters.depthParameters[depth].overrideImpactDamage())
			{
				return mDestructibleParameters.depthParameters[depth].overrideImpactDamageValue();
			}
		}
		return (physx::PxI32)depth <= mDestructibleParameters.impactDamageDefaultDepth;
	}

	bool					isChunkSolitary(physx::PxI32 chunkIndex) const
	{
		PX_ASSERT(mAsset != NULL && mStructure != NULL && chunkIndex >= 0 && chunkIndex < (physx::PxI32)mAsset->getChunkCount());
		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + chunkIndex];
		return mStructure->chunkIsSolitary( chunk );
	}

	bool					isChunkDestroyed(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(mAsset != NULL && mStructure != NULL && chunkIndex < mAsset->getChunkCount());
		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + chunkIndex];
		return chunk.isDestroyed();
	}

	physx::PxBounds3		getChunkLocalBounds(physx::PxU32 chunkIndex) const
	{ 
		PX_ASSERT(mAsset != NULL && mStructure != NULL && chunkIndex < mAsset->getChunkCount());

		physx::PxBounds3 localBounds = getAsset()->getChunkShapeLocalBounds( chunkIndex );

		physx::PxVec3 boundsExtents = localBounds.getExtents();
		physx::PxVec3 scaledExtents( boundsExtents.x * getScale().x, boundsExtents.y * getScale().y, boundsExtents.z * getScale().z );

		physx::PxBounds3 scaledLocalBounds = physx::PxBounds3::centerExtents( localBounds.getCenter(), scaledExtents );

		return scaledLocalBounds;
	}

	physx::PxBounds3		getChunkBounds(physx::PxU32 chunkIndex) const
	{ 
		PX_ASSERT(mAsset != NULL && mStructure != NULL && chunkIndex < mAsset->getChunkCount());

		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + chunkIndex];
		physx::PxMat34Legacy chunkGlobalPose = mStructure->getChunkGlobalPose( chunk );

		physx::PxBounds3 chunkBounds = getAsset()->getChunkShapeLocalBounds( chunkIndex );

		// Apply scaling.
		chunkGlobalPose.M.multiplyDiagonal( getScale() );

		// Transform into actor local space.
		PxBounds3Transform( chunkBounds, chunkGlobalPose.M, chunkGlobalPose.t );

		return chunkBounds;
	}

	physx::PxU32			getSupportDepthChunkIndices(physx::PxU32* const OutChunkIndices, physx::PxU32  MaxOutIndices) const
	{
		return mStructure->getSupportDepthChunkIndices( OutChunkIndices,  MaxOutIndices );
	}

	physx::PxBounds3		getLocalBounds() const
	{
		physx::PxBounds3 bounds = getAsset()->getBounds();
		physx::PxVec3 scale = getScale();

		bounds.minimum.x *= scale.x;
		bounds.minimum.y *= scale.y;
		bounds.minimum.z *= scale.z;

		bounds.maximum.x *= scale.x;
		bounds.maximum.y *= scale.y;
		bounds.maximum.z *= scale.z;

		return bounds;
	}

	physx::PxF32			getLinearSize() const { return mLinearSize; }

	void					applyDamage_immediate(struct DamageEvent& damageEvent);
	void					applyRadiusDamage_immediate(struct DamageEvent& damageEvent);

	physx::PxI32			rayCast(physx::PxF32& time, physx::PxVec3& normal, const NxRay& worldRay, NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX) const
	{
		PX_ASSERT(worldRay.orig.isFinite() && worldRay.dir.isFinite());

		NxBox worldBox;
		worldBox.setEmpty();
		worldBox.center		= worldRay.orig;
		worldBox.extents	= NxVec3(0.0f, 0.0f, 0.0f);
		physx::PxI32 chunk;
		if (flags & NxDestructibleActorRaycastFlags::DynamicChunks)
		{
			chunk = pointOrOBBSweep(time, normal, worldBox, PXFROMNXVEC3(worldRay.dir), flags, parentChunkIndex);
		}
		else
		{
			chunk = pointOrOBBSweepStatic(time, normal, worldBox, PXFROMNXVEC3(worldRay.dir), flags, parentChunkIndex);
		}
#if APEX_RUNTIME_FRACTURE
		if (mRTActor != NULL)
		{
			physx::PxF32 rtTime = PX_MAX_F32;
			// TODO: Refactor the runtime actor raycasting to use a more sensible approach
			//  (Perhaps we can re-use some of the existing pointOrOBBSweep code
			if (mRTActor->rayCast(PXFROMNXVEC3(worldRay.orig), PXFROMNXVEC3(worldRay.dir), rtTime) && 
				(chunk == NxModuleDestructibleConst::INVALID_CHUNK_INDEX || rtTime < time))
			{
				time = rtTime;
				chunk = 0;
			}
		}
#endif
		return chunk;
	}

	physx::PxI32			obbSweep(physx::PxF32& time, physx::PxVec3& normal, const NxBox& worldBox, const physx::PxVec3& worldDisplacement, NxDestructibleActorRaycastFlags::Enum flags) const
	{
		if (flags & NxDestructibleActorRaycastFlags::DynamicChunks)
		{
			return pointOrOBBSweep(time, normal, worldBox, worldDisplacement, flags, NxModuleDestructibleConst::INVALID_CHUNK_INDEX);
		}
		else
		{
			return pointOrOBBSweepStatic(time, normal, worldBox, worldDisplacement, flags, NxModuleDestructibleConst::INVALID_CHUNK_INDEX);
		}
	}

	void					setActorObjDescFlags(class NiApexPhysXObjectDesc* actorObjDesc, physx::PxU32 depth) const;

	void					fillInstanceBuffers();
	void					setRenderTMs(bool processChunkPoseForSyncing = false);
	void					setRelativeTMs();

	virtual bool			recreateApexEmitter(NxDestructibleEmitterType::Enum type);
	bool					initCrumbleSystem(const char* crumbleEmitterName);
	bool					initDustSystem(const char* dustEmitterName);
	bool					initFracturePattern(const char* fracturePatternName);

	virtual void			preSerialize(void *userData = NULL);

	virtual void				setPreferredRenderVolume(NxApexRenderVolume* volume, NxDestructibleEmitterType::Enum type);
	virtual NxApexEmitterActor*	getApexEmitter(NxDestructibleEmitterType::Enum type);

	void					spawnParticles(class NxApexEmitterActor* emitter, NxUserChunkParticleReport* report, DestructibleStructure::Chunk& chunk, physx::Array<physx::PxVec3>& positions, bool deriveVelocitiesFromChunk = false, const physx::PxVec3* overrideVelocity = NULL);

	void					setDeleteFracturedChunks(bool inDeleteChunkMode);

	bool					useDamageColoring()
	{
		return mUseDamageColoring;
	}

	/**
		Return ture means these core data of Damage Event should be saved for the damage coloring
	*/
	bool					applyDamageColoring(physx::PxU16 indexInAsset, const physx::PxVec3& position, physx::PxF32 damage, physx::PxF32 damageRadius);
	bool					applyDamageColoringRecursive(physx::PxU16 indexInAsset, const physx::PxVec3& position, physx::PxF32 damage, physx::PxF32 damageRadius);

	void					fillBehaviorGroupDesc(NxDestructibleBehaviorGroupDesc& behaviorGroupDesc, const DestructibleActorParamNS::BehaviorGroup_Type behaviorGroup) const;

	/*** Public behavior group functions ***/
	physx::PxU32			getCustomBehaviorGroupCount() const
	{
		return (physx::PxU32)mParams->behaviorGroups.arraySizes[0];
	}

	bool					getBehaviorGroup(NxDestructibleBehaviorGroupDesc& behaviorGroupDesc, physx::PxI32 index) const
	{
		if (index == -1)
		{
			fillBehaviorGroupDesc(behaviorGroupDesc, mParams->defaultBehaviorGroup);
			return true;
		}
		if (index >= 0 && index < (physx::PxI32)mParams->behaviorGroups.arraySizes[0])
		{
			fillBehaviorGroupDesc(behaviorGroupDesc, mParams->behaviorGroups.buf[index]);
			return true;
		}
		return false;
	}

	/*** NxDestructibleHitChunk operations ***/
	bool					setHitChunkTrackingParams(bool flushHistory, bool startTracking, physx::PxU32 trackingDepth, bool trackAllChunks = true);
	bool					getHitChunkHistory(const NxDestructibleHitChunk *& hitChunkContainer, physx::PxU32 & hitChunkCount) const;
	bool					forceChunkHits(const NxDestructibleHitChunk * hitChunkContainer, physx::PxU32 hitChunkCount, bool removeChunks = true, bool deferredEvent = false, physx::PxVec3 damagePosition = physx::PxVec3(0.0f), physx::PxVec3 damageDirection = physx::PxVec3(0.0f));
	void					evaluateForHitChunkList(const FractureEvent & fractureEvent);

	struct CachedHitChunk : public NxDestructibleHitChunk
	{
		CachedHitChunk(physx::PxU32 chunkIndex_, physx::PxU32 hitChunkFlags_)
		{
#if defined WIN32
			extern char enforce[sizeof(*this) == sizeof(NxDestructibleHitChunk)?1:-1];
#endif // WIN32
			NxDestructibleHitChunk::chunkIndex = chunkIndex_;
			NxDestructibleHitChunk::hitChunkFlags = hitChunkFlags_;
		}
		~CachedHitChunk() {}
	private:
		CachedHitChunk();
	};

	/*** Damage Coloring ***/
	bool					getDamageColoringHistory(const NxDamageEventCoreData *& damageEventCoreDataContainer, physx::PxU32 & damageEventCoreDataCount) const;
	bool					forceDamageColoring(const NxDamageEventCoreData * damageEventCoreDataContainer, physx::PxU32 damageEventCoreDataCount);
	void					collectDamageColoring(const physx::PxI32 indexInAsset, const physx::PxVec3& position, const physx::PxF32 damage, const physx::PxF32 damageRadius);
	void					applyDamageColoring_immediate(const physx::PxI32 indexInAsset, const physx::PxVec3& position, const physx::PxF32 damage, const physx::PxF32 damageRadius);

	struct CachedDamageEventCoreData : public NxDamageEventCoreData
	{
		CachedDamageEventCoreData(physx::PxI32 chunkIndex_, physx::PxVec3 position_, physx::PxF32 damage_, physx::PxF32 radius_)
		{
#if defined WIN32
			extern char enforce[sizeof(*this) == sizeof(NxDamageEventCoreData)?1:-1];
#endif // WIN32			
			NxDamageEventCoreData::chunkIndexInAsset = chunkIndex_;
			NxDamageEventCoreData::position = position_;
			NxDamageEventCoreData::damage = damage_;
			NxDamageEventCoreData::radius = radius_;
		}
		~CachedDamageEventCoreData() {}
	private:
		CachedDamageEventCoreData();
	};

private:
	struct HitChunkParams
	{
	public:
		HitChunkParams():cacheChunkHits(false),cacheAllChunks(false),trackingDepth(0)
		{
			manualFractureEventInstance.position			= physx::PxVec3(0.0f);
			manualFractureEventInstance.impulse				= physx::PxVec3(0.0f);
			manualFractureEventInstance.hitDirection		= physx::PxVec3(0.0f);
		}
		~HitChunkParams() {}
		bool									cacheChunkHits;
		bool									cacheAllChunks;
		physx::PxU32							trackingDepth;
		physx::Array<CachedHitChunk>			hitChunkContainer;
		FractureEvent							manualFractureEventInstance;
	}											hitChunkParams;

	/*** structure for damage coloring ***/
	struct DamageColoringParams
	{
		physx::Array<CachedDamageEventCoreData>			damageEventCoreDataContainer;
		SyncDamageEventCoreDataParams					damageEventCoreDataInstance;
	} damageColoringParams;


    /*** DestructibleActor::SyncParams ***/
public:
    bool setSyncParams(physx::PxU32 userActorID, physx::PxU32 actorSyncFlags, const NxDestructibleActorSyncState * actorSyncState, const NxDestructibleChunkSyncState * chunkSyncState);
public:
    class SyncParams
    {
		friend bool DestructibleActor::setSyncParams(physx::PxU32, physx::PxU32, const NxDestructibleActorSyncState *, const NxDestructibleChunkSyncState *);
    public:
        SyncParams();
        ~SyncParams();
		physx::PxU32							getUserActorID() const;
		bool									isSyncFlagSet(NxDestructibleActorSyncFlags::Enum flag) const;
		const NxDestructibleActorSyncState *	getActorSyncState() const;
		const NxDestructibleChunkSyncState *	getChunkSyncState() const;

		void									pushDamageBufferIndex(physx::PxU32 index);
		void									pushFractureBufferIndex(physx::PxU32 index);
		void									pushCachedChunkTransform(const CachedChunk & cachedChunk);

		const physx::Array<physx::PxU32> &		getDamageBufferIndices() const;
		const physx::Array<physx::PxU32> &		getFractureBufferIndices() const;
		const physx::Array<CachedChunk> &		getCachedChunkTransforms() const;

		template<typename Unit> void			clear();
		template<typename Unit> physx::PxU32	getCount() const;
	private:
		DECLARE_DISABLE_COPY_AND_ASSIGN(SyncParams);
		void									onReset();
		physx::PxU32							userActorID;
        physx::PxU32							actorSyncFlags;
		bool									useActorSyncState;
		NxDestructibleActorSyncState			actorSyncState;
		bool									useChunkSyncState;
		NxDestructibleChunkSyncState			chunkSyncState;

		physx::Array<physx::PxU32>				damageBufferIndices;
		physx::Array<physx::PxU32>				fractureBufferIndices;
		physx::Array<CachedChunk>				cachedChunkTransforms;
    };

	const DestructibleActor::SyncParams &		getSyncParams() const;
	DestructibleActor::SyncParams &				getSyncParamsMutable();
private:
	SyncParams									mSyncParams;

private:
	DestructibleActor(NxDestructibleActor* _api, DestructibleAsset& _asset, DestructibleScene& scene);
	virtual					~DestructibleActor();

	void					initialize(NxParameterized::Interface* stateOrParams);
	void					initializeFromState(NxParameterized::Interface* state);
	void					initializeFromParams(NxParameterized::Interface* params);
	void					initializeCommon(void);
	void					createRenderable(void);
	void					initializeActor(void);
	void					initializeRTActor(void);
	void					initializeEmitters(void);
	void					deinitialize(void);

	void					setDestructibleParameters(const DestructibleActorParamNS::DestructibleParameters_Type& destructibleParameters,
	                                                  const DestructibleActorParamNS::DestructibleDepthParameters_DynamicArray1D_Type& destructibleDepthParameters);

	physx::PxI32			pointOrOBBSweep(physx::PxF32& time, physx::PxVec3& normal, const NxBox& worldBox, const physx::PxVec3& worldDisp,
	                                        NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex) const;

	physx::PxI32			pointOrOBBSweepStatic(physx::PxF32& time, physx::PxVec3& normal, const NxBox& worldBox, const physx::PxVec3& pxWorldDisp,
											NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex) const;

	void					wakeUp(void);
	void					putToSleep(void);

	// Renderable support:
public:
	NxApexRenderable*			getRenderable()
	{
		return static_cast<NxApexRenderable*>(mRenderable);
	}
	NxDestructibleRenderable*	acquireRenderableReference();
	NxRenderMeshActor*			getRenderMeshActor(NxDestructibleActorMeshType::Enum type = NxDestructibleActorMeshType::Skinned) const;

private:

	DestructibleActorState*		mState;					// Destructible asset state, contains data for serialization

	// Cached parameters
	DestructibleActorParam*		mParams;				// Destructible actor params, this is just a convenient reference to the params in mState
	DestructibleActorChunks*	mChunks;				// Destructible actor chunks, this is just a convenient reference to the chunks in mState

	// Read-write cached parameters (require writeback on preSerialization)
	NxDestructibleParameters	mDestructibleParameters;
	physx::PxMat44				mTM;
	physx::PxMat44				mRelTM;					// Relative transform between the actor 

	// Derived parameters
	NxIndexBank<physx::PxU16>	mStaticRoots;
	NxIndexBank<physx::PxU16>	mVisibleChunks;
	physx::PxU32				mVisibleDynamicChunkShapeCount;
	physx::PxU32				mEssentialVisibleDynamicChunkShapeCount;
	DestructibleScene*			mDestructibleScene;
	NxDestructibleActor*		mAPI;
	physx::PxU16				mFlags;
	physx::PxU16				mInternalFlags;			// Internal flags, currently indicates whether the actor has been marked as being part of an 'island' or not.
	DestructibleStructure*		mStructure;				// The destructible structure that this actor is a member of.  Multiple destructible actors can belong to a single destructible structure.
	DestructibleAsset*			mAsset;					// The asset associated with this actor.
	physx::PxU32				mID;						// A unique 32 bit GUID given to this actor.
	physx::PxF32				mLinearSize;
	physx::PxBounds3			mOriginalBounds;
	physx::PxBounds3			mNonInstancedBounds;	// Recording this separately, since instanced buffers get updated separately
	physx::PxBounds3			mInstancedBounds;		// Recording this separately, since instanced buffers get updated separately
	physx::PxU32				mFirstChunkIndex;		// This first chunk index used from the destructible structure
	NxApexEmitterActor*			mCrumbleEmitter;		// The crumble emitter associated with this actor.
	NxApexEmitterActor*			mDustEmitter;			// The dust emitter associated with this actor.
	NxApexRenderVolume*			mCrumbleRenderVolume;	// The render volume to use for crumble effects.
	NxApexRenderVolume*			mDustRenderVolume;		// The render volume to use for dust effects.
	physx::PxF32				mStartTime;				// Time the actor became "alive."
	physx::PxF32				mBenefit;
	bool						mInitializedFromState;			// Whether the actor was initialized from state
	physx::Array<NxDestructibleChunkEvent>	mChunkEventBuffer;
	physx::Mutex				mChunkEventBufferLock;
#if NX_SDK_VERSION_MAJOR == 2
	physx::Array<NxActor*>		mPhysXActorBuffer;
#elif NX_SDK_VERSION_MAJOR == 3
	physx::Array<physx::PxRigidDynamic*>	mPhysXActorBuffer;
#endif
	physx::Mutex				mPhysXActorBufferLock;

	DestructibleRenderable*		mRenderable;

	physx::Array< physx::Array<PxColorRGBA> >	mDamageColorArrays;
	bool						mUseDamageColoring;

	physx::PxU32				mDescOverrideSkinnedMaterialCount;
	const char**				mDescOverrideSkinnedMaterials;
	physx::PxU32				mDescOverrideStaticMaterialCount;
	const char** 				mDescOverrideStaticMaterials;
	bool						mPhysXActorBufferAcquired;
public:
	bool						mInDeleteChunkMode;

	physx::PxU32				mAwakeActorCount;	// number of awake NxActors.
	physx::PxU32				mActiveFrames;		// Bit representation of active frame history.  Only used when mDestructibleScene->mUsingActiveTransforms == true

	physx::PxU32				mDamageEventReportIndex;

#if USE_DESTRUCTIBLE_RWLOCK
	physx::ReadWriteLock*		mLock;
#endif

#if APEX_RUNTIME_FRACTURE
	::physx::fracture::Actor*			mRTActor;
#endif

	bool						mWakeForEvent;

	physx::Array<NxActor*>		mReferencingActors;

public:
	void						referencedByActor(NxActor* actor);
	void						unreferencedByActor(NxActor* actor);

	NxActor**					getReferencingActors(physx::PxU32& count)
	{
		count = mReferencingActors.size();
		return count ? &mReferencingActors[0] : NULL;
	}

	void				wakeForEvent();
	void				resetWakeForEvent();

#if NX_SDK_VERSION_MAJOR == 2	// In 3.x this is put into the asset
	physx::Array<NxConvexMesh*>*	mCollisionMeshes;
#endif

public:

	NxConvexMesh*				getConvexMesh(physx::PxU32 hullIndex)
	{
#if NX_SDK_VERSION_MAJOR == 2
		if (mCollisionMeshes == NULL || mCollisionMeshes->size() == 0)
		{
			mCollisionMeshes = mAsset->module->mCachedData->getConvexMeshesForActor(*this);
			PX_ASSERT(mCollisionMeshes != NULL && mCollisionMeshes->size() > 0);
			if (mCollisionMeshes == NULL || mCollisionMeshes->size() == 0)
			{
				return NULL;
			}
		}
		return (*mCollisionMeshes)[hullIndex];
#else
		if (mAsset->mCollisionMeshes == NULL)
		{
			mAsset->mCollisionMeshes = mAsset->module->mCachedData->getConvexMeshesForScale(*mAsset, mAsset->module->getChunkCollisionHullCookingScale());
			PX_ASSERT(mAsset->mCollisionMeshes != NULL);
			if (mAsset->mCollisionMeshes == NULL)
			{
				return NULL;
			}
		}
		return (*mAsset->mCollisionMeshes)[hullIndex];
#endif
	}
};


struct DamageEvent : public NxDamageEventCoreData
{
	// Default constructor for a damage event.
	DamageEvent() :
		destructibleID((physx::PxU32)DestructibleActor::InvalidID),
		momentum(0.0f), 
		direction(physx::PxVec3(0.0f, 0.0f, 1.0f)),
		flags(0), 
		impactDamageActor(NULL),
		appliedDamageUserData(NULL),
		minDepth(0), 
		maxDepth(0), 
		processDepth(0)
	{
		NxDamageEventCoreData::chunkIndexInAsset = 0;
		NxDamageEventCoreData::damage = 0.0f;
		NxDamageEventCoreData::radius = 0.0f;
		NxDamageEventCoreData::position = physx::PxVec3(0.0f);

		for (physx::PxU32 i = 0; i <= MaxDepth; ++i)
		{
			cost[i] = benefit[i] = 0;
			new(fractures + i) physx::Array<FractureEvent>();
		}
	}

	// Copy constructor for a damage event.
	DamageEvent(const DamageEvent& that)
	{
		destructibleID = that.destructibleID;
		damage = that.damage;
		momentum = that.momentum;
		radius = that.radius;
		position = that.position;
		direction = that.direction;
		chunkIndexInAsset = that.chunkIndexInAsset;
		flags = that.flags;

		minDepth = that.minDepth;
		maxDepth = that.maxDepth;
		processDepth = that.processDepth;

		impactDamageActor = that.impactDamageActor;
		appliedDamageUserData = that.appliedDamageUserData;

		for (physx::PxU32 i = 0; i < MaxDepth + 1; i++)
		{
			cost[i] = that.cost[i];
			benefit[i] = that.benefit[i];
			fractures[i] = that.fractures[i];
		}
	}

	~DamageEvent()
	{
		for (physx::PxU32 i = MaxDepth + 1; i--;)
		{
			fractures[i].reset();
		}
	}

	// Deep copy of one damage event to another.
	DamageEvent&	operator=(const DamageEvent& that)
	{
		if (this != &that)
		{
			destructibleID = that.destructibleID;
			damage = that.damage;
			momentum = that.momentum;
			radius = that.radius;
			position = that.position;
			direction = that.direction;
			chunkIndexInAsset = that.chunkIndexInAsset;
			flags = that.flags;

			minDepth = that.minDepth;
			maxDepth = that.maxDepth;
			processDepth = that.processDepth;

			impactDamageActor = that.impactDamageActor;
			appliedDamageUserData = that.appliedDamageUserData;

			for (physx::PxU32 i = 0; i < MaxDepth + 1; i++)
			{
				cost[i] = that.cost[i];
				benefit[i] = that.benefit[i];
				fractures[i] = that.fractures[i];
			}
		}
		return *this;

	}

	enum Flag
	{
		UseRadius =				(1U << 0),			// Indicates whether or not to process the damage event as a radius based effect
		HasFalloff =			(1U << 1),			// Indicates whether or not the damage amount (for radius damage) falls off with distance from the impact.
		IsFromImpact =			(1U << 2),			// Indicates that this is an impact event, where damage is applied to specific chunk rather than radius based damage.
		SyncDirect =			(1U << 3),			// Indicates whether this is a sync-ed damage event
		DeleteChunkModeUnused =	(1U << 30),			// Indicates whether or not to delete the chunk instead of breaking it off. Propagates down to FractureEvent.

		Invalid =				(1U << 31)			// Indicates that this event is invalid (can occur when a destructible is removed)
	};

	physx::PxU32	destructibleID;			// The ID of the destructible actor that is being damaged.
	physx::PxF32	momentum;				// The inherited momentum of the damage event.
	physx::PxVec3	direction;				// The direction of the damage event.
	physx::PxU32	flags;					// Flags which indicate whether this damage event is radius based and if we use a fall off computation for the amount of damage.
#if NX_SDK_VERSION_MAJOR == 2
	NxActor const* impactDamageActor;		// Other PhysX actor that caused damage to NxApexDamageEventReportData.
#elif NX_SDK_VERSION_MAJOR == 3
	physx::PxActor const* impactDamageActor;// Other PhysX actor that caused damage to NxApexDamageEventReportData.
#endif
	void*			appliedDamageUserData;	// User data from applyDamage or applyRadiusDamage.

	enum
	{
		MaxDepth	= 5
	};

	PX_INLINE	physx::PxU32	getMinDepth() const
	{
		return minDepth;
	}
	PX_INLINE	physx::PxU32	getMaxDepth() const
	{
		return maxDepth;
	}
	PX_INLINE	physx::PxU32	getProcessDepth() const
	{
		return processDepth;
	}
	PX_INLINE	physx::PxF32	getCost(physx::PxU32 depth = 0xFFFFFFFF) const
	{
		return cost[depth <= MaxDepth ? depth : processDepth];
	}
	PX_INLINE	physx::PxF32	getBenefit(physx::PxU32 depth = 0xFFFFFFFF) const
	{
		return benefit[depth <= MaxDepth ? depth : processDepth];
	}
	PX_INLINE bool isFromImpact(void) const
	{
		return flags & IsFromImpact ? true : false;
	};

	void resetFracturesInternal()
	{
		for(physx::PxU32 index = DamageEvent::MaxDepth + 1; index--; )
		{
			fractures[index].reset();
		}
	}
private:
	physx::PxU32				minDepth;					// The minimum structure depth that this damage event can apply to.
	physx::PxU32				maxDepth;					// The maximum structure depth that this damage event can apply to.
	physx::PxU32				processDepth;				// The exact depth that the damage event applies to.
	physx::PxF32				cost[MaxDepth + 1];			// The LOD 'cost' at each depth for this damage event.
	physx::PxF32				benefit[MaxDepth + 1];		// The LOD 'benefit' at each depth for this damage event.
	physx::Array<FractureEvent>	fractures[MaxDepth + 1];	// The array of fracture events associated with this damage event.

	friend class DestructibleScene;
	friend class DestructibleActor;
};

#if USE_DESTRUCTIBLE_RWLOCK
class DestructibleScopedReadLock : public physx::ScopedReadLock
{
public:
	DestructibleScopedReadLock(DestructibleActor& destructible) : physx::ScopedReadLock(*destructible.mLock) {}
};

class DestructibleScopedWriteLock : public physx::ScopedWriteLock
{
public:
	DestructibleScopedWriteLock(DestructibleActor& destructible) : physx::ScopedWriteLock(*destructible.mLock) {}
};
#endif

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLEACTOR_H__

/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEACTOR_PROXY_H__
#define __DESTRUCTIBLEACTOR_PROXY_H__

#include "NxApex.h"
#include "NxDestructibleActor.h"
#include "DestructibleActorJointProxy.h"
#include "DestructibleActor.h"
#include "DestructibleScene.h"
#include "PsUserAllocated.h"
#include "ApexActor.h"
#if APEX_USE_PARTICLES
#include "NxApexEmitterActor.h"
#endif

#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{
namespace destructible
{

class DestructibleActorProxy : public NxDestructibleActor, public NxApexResource, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleActor impl;

#pragma warning(disable : 4355) // disable warning about this pointer in argument list
	DestructibleActorProxy(const NxParameterized::Interface& input, DestructibleAsset& asset, NxResourceList& list, DestructibleScene& scene)
		: impl(this, asset, scene)
	{
		NX_WRITE_ZONE();
		NxParameterized::Interface* clonedInput = NULL;
		input.clone(clonedInput);
		list.add(*this);	// Doing this before impl.initialize, since the render proxy created in that function wants a unique ID (only needs to be unique at any given time, can be recycled)
		impl.initialize(clonedInput);
	}

	DestructibleActorProxy(NxParameterized::Interface* input, DestructibleAsset& asset, NxResourceList& list, DestructibleScene& scene)
		: impl(this, asset, scene)
	{
		NX_WRITE_ZONE();
		impl.initialize(input);
		list.add(*this);
	}

	~DestructibleActorProxy()
	{
	}

	virtual const NxDestructibleParameters& getDestructibleParameters() const
	{
		NX_READ_ZONE();
		return impl.getDestructibleParameters();
	}

	virtual void setDestructibleParameters(const NxDestructibleParameters& destructibleParameters)
	{
		NX_WRITE_ZONE();
		impl.setDestructibleParameters(destructibleParameters);
	}

	virtual const NxRenderMeshActor* getRenderMeshActor(NxDestructibleActorMeshType::Enum type = NxDestructibleActorMeshType::Skinned) const
	{
		NX_READ_ZONE();
		return impl.getRenderMeshActor(type);
	}

	virtual physx::PxMat44	getInitialGlobalPose() const
	{
		NX_READ_ZONE();
		return impl.getInitialGlobalPose();
	}

	virtual void setInitialGlobalPose(const physx::PxMat44& pose)
	{
		NX_WRITE_ZONE();
		impl.setInitialGlobalPose(pose);
	}

	virtual physx::PxVec3	getScale() const
	{
		NX_READ_ZONE();
		return impl.getScale();
	}

	virtual void	applyDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, const physx::PxVec3& direction, physx::PxI32 chunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX, void* damageActorUserData = NULL)
	{
		NX_WRITE_ZONE();
		return impl.applyDamage(damage, momentum, position, direction, chunkIndex, damageActorUserData);
	}

	virtual void	applyRadiusDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, physx::PxF32 radius, bool falloff, void* damageActorUserData = NULL)
	{
		NX_WRITE_ZONE();
		return impl.applyRadiusDamage(damage, momentum, position, radius, falloff, damageActorUserData);
	}	

	virtual void	getChunkVisibilities(physx::PxU8* visibilityArray, physx::PxU32 visibilityArraySize) const
	{
		NX_READ_ZONE();
		impl.getChunkVisibilities(visibilityArray, visibilityArraySize);
	}

	virtual physx::PxU32	getNumVisibleChunks() const
	{
		NX_READ_ZONE();
		return impl.getNumVisibleChunks();
	}

	virtual const physx::PxU16*	getVisibleChunks() const
	{
		NX_READ_ZONE();
		return impl.getVisibleChunks();
	}

	virtual bool					acquireChunkEventBuffer(const NxDestructibleChunkEvent*& buffer, physx::PxU32& bufferSize)
	{
		NX_READ_ZONE();
		return impl.acquireChunkEventBuffer(buffer, bufferSize);
	}

	virtual bool					releaseChunkEventBuffer(bool clearBuffer = true)
	{
		NX_READ_ZONE();
		return impl.releaseChunkEventBuffer(clearBuffer);
	}

#if NX_SDK_VERSION_MAJOR == 2
	virtual bool					acquirePhysXActorBuffer(NxActor**& buffer, physx::PxU32& bufferSize, physx::PxU32 flags = NxDestructiblePhysXActorQueryFlags::AllStates)
#elif NX_SDK_VERSION_MAJOR == 3
	virtual bool					acquirePhysXActorBuffer(physx::PxRigidDynamic**& buffer, physx::PxU32& bufferSize, physx::PxU32 flags = NxDestructiblePhysXActorQueryFlags::AllStates)
#endif
	{
		NX_READ_ZONE();
		return impl.acquirePhysXActorBuffer(buffer, bufferSize, flags);
	}

	virtual bool					releasePhysXActorBuffer()
	{
		NX_READ_ZONE();
		return impl.releasePhysXActorBuffer();
	}

#if NX_SDK_VERSION_MAJOR == 2
	virtual NxActor*				getChunkPhysXActor(physx::PxU32 index)
	{
		NX_READ_ZONE();
		return impl.getChunkActor(index);
	}
#elif NX_SDK_VERSION_MAJOR == 3
	virtual physx::PxRigidDynamic*	getChunkPhysXActor(physx::PxU32 index)
	{
		NX_READ_ZONE();
		physx::PxActor* actor = impl.getChunkActor(index);
		PX_ASSERT(actor == NULL || actor->isRigidDynamic());
		return (physx::PxRigidDynamic*)actor;
	}
#endif

#if NX_SDK_VERSION_MAJOR == 2
	virtual physx::PxU32			getChunkPhysXShapes(NxShape**& shapes, physx::PxU32 chunkIndex) const
#elif NX_SDK_VERSION_MAJOR == 3
	virtual physx::PxU32			getChunkPhysXShapes(physx::PxShape**& shapes, physx::PxU32 chunkIndex) const
#endif
	{
		NX_READ_ZONE();
		return impl.getChunkPhysXShapes(shapes, chunkIndex);
	}

	virtual physx::PxMat44			getChunkPose(physx::PxU32 index) const
	{
		NX_READ_ZONE();
		return impl.getChunkPose(index);
	}

	virtual physx::PxTransform		getChunkTransform(physx::PxU32 index) const
	{
		NX_READ_ZONE();
		return impl.getChunkTransform(index);
	}

	virtual physx::PxVec3			getChunkLinearVelocity(physx::PxU32 index) const
	{
		NX_READ_ZONE();
		return impl.getChunkLinearVelocity(index);
	}

	virtual physx::PxVec3			getChunkAngularVelocity(physx::PxU32 index) const
	{
		NX_READ_ZONE();
		return impl.getChunkAngularVelocity(index);
	}

	virtual const physx::PxMat44	getChunkTM(physx::PxU32 index) const
	{
		NX_READ_ZONE();
		return impl.getChunkTM(index);
	}

	virtual physx::PxI32			getChunkBehaviorGroupIndex(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getBehaviorGroupIndex(chunkIndex);
	}

	virtual physx::PxU32		 	getChunkActorFlags(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkActorFlags(chunkIndex);
	}

	virtual bool					isChunkDestroyed(physx::PxI32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.isChunkDestroyed((physx::PxU32)chunkIndex);
	}

	virtual void			setSkinnedOverrideMaterial(PxU32 index, const char* overrideMaterialName)
	{
		NX_WRITE_ZONE();
		impl.setSkinnedOverrideMaterial(index, overrideMaterialName);
	}

	virtual void			setStaticOverrideMaterial(PxU32 index, const char* overrideMaterialName)
	{
		NX_WRITE_ZONE();
		impl.setStaticOverrideMaterial(index, overrideMaterialName);
	}

	virtual void			setRuntimeFractureOverridePattern(const char* overridePatternName)
	{
		NX_WRITE_ZONE();
		impl.setRuntimeFracturePattern(overridePatternName);
	}

	virtual bool			isInitiallyDynamic() const
	{
		NX_READ_ZONE();
		return impl.isInitiallyDynamic();
	}

	virtual void			setLinearVelocity(const physx::PxVec3& linearVelocity)
	{
		NX_WRITE_ZONE();
		impl.setLinearVelocity(linearVelocity);
	}

	virtual void			setGlobalPose(const physx::PxMat44& pose)
	{
		NX_WRITE_ZONE();
		impl.setGlobalPose(pose);
	}

	virtual bool			getGlobalPose(physx::PxMat44& pose)
	{
		NX_READ_ZONE();
		return impl.getGlobalPoseForStaticChunks(pose);
	}

	virtual void			setAngularVelocity(const physx::PxVec3& angularVelocity)
	{
		NX_WRITE_ZONE();
		impl.setAngularVelocity(angularVelocity);
	}

	virtual void setDynamic(physx::PxI32 chunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
	{
		NX_WRITE_ZONE();
		impl.setDynamic(chunkIndex);
	}

	virtual bool isDynamic(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		if (impl.getAsset() != NULL && chunkIndex < impl.getAsset()->getChunkCount())
		{
			return impl.getDynamic((physx::PxI32)chunkIndex);
		}
		return false;
	}

	virtual void enableHardSleeping()
	{
		NX_WRITE_ZONE();
		impl.enableHardSleeping();
	}

	virtual void disableHardSleeping(bool wake = false)
	{
		NX_WRITE_ZONE();
		impl.disableHardSleeping(wake);
	}

	virtual bool isHardSleepingEnabled() const
	{
		NX_READ_ZONE();
		return impl.useHardSleeping();
	}

	virtual	bool setChunkPhysXActorAwakeState(physx::PxU32 chunkIndex, bool awake)
	{
		NX_WRITE_ZONE();
		return impl.setChunkPhysXActorAwakeState(chunkIndex, awake);
	}

	virtual bool addForce(PxU32 chunkIndex, const PxVec3& force, physx::PxForceMode::Enum mode, const PxVec3* position = NULL, bool wakeup = true)
	{
		NX_WRITE_ZONE();
		return impl.addForce(chunkIndex, force, mode, position, wakeup);
	}

#if NX_SDK_VERSION_MAJOR == 2
	virtual physx::PxI32			rayCast(physx::PxF32& time, physx::PxVec3& normal, const NxRay& worldRay, NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX) const
	{
		NX_READ_ZONE();
		return impl.rayCast(time, normal, worldRay, flags, parentChunkIndex);
	}

	virtual physx::PxI32			obbSweep(physx::PxF32& time, physx::PxVec3& normal, const NxBox& worldBox, const physx::PxVec3& worldDisplacement, NxDestructibleActorRaycastFlags::Enum flags) const
	{
		NX_READ_ZONE();
		return impl.obbSweep(time, normal, worldBox, worldDisplacement, flags);
	}
#elif NX_SDK_VERSION_MAJOR == 3
	virtual physx::PxI32			rayCast(physx::PxF32& time, physx::PxVec3& normal, const PxVec3& worldRayOrig, const PxVec3& worldRayDir, NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX) const
	{
		NX_READ_ZONE();
		NxRay	worldRay(worldRayOrig, worldRayDir);
		return impl.rayCast(time, normal, worldRay, flags, parentChunkIndex);
	}

	virtual physx::PxI32			obbSweep(physx::PxF32& time, physx::PxVec3& normal, const physx::PxVec3& worldBoxCenter, const physx::PxVec3& worldBoxExtents, const physx::PxMat33& worldBoxRot, const physx::PxVec3& worldDisplacement, NxDestructibleActorRaycastFlags::Enum flags) const
	{
		NX_READ_ZONE();
		NxBox	worldBox(worldBoxCenter, worldBoxExtents, worldBoxRot);
		return impl.obbSweep(time, normal, worldBox, worldDisplacement, flags);
	}
#endif
	virtual void cacheModuleData() const
	{
		NX_READ_ZONE();
		return impl.cacheModuleData();
	}

	virtual physx::PxBounds3		getLocalBounds() const
	{
		NX_READ_ZONE();
		return impl.getLocalBounds();
	}

	virtual physx::PxBounds3		getOriginalBounds() const
	{ 
		NX_READ_ZONE();
		return impl.getOriginalBounds();
	}

	virtual bool					isChunkSolitary(physx::PxI32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.isChunkSolitary( chunkIndex );
	}

	virtual physx::PxBounds3		getChunkBounds(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkBounds( chunkIndex );
	}

	virtual physx::PxBounds3		getChunkLocalBounds(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkLocalBounds( chunkIndex );
	}

	virtual physx::PxU32			getSupportDepthChunkIndices(physx::PxU32* const OutChunkIndices, physx::PxU32 MaxOutIndices) const
	{
		NX_READ_ZONE();
		return impl.getSupportDepthChunkIndices( OutChunkIndices, MaxOutIndices );
	}

	virtual physx::PxU32			getSupportDepth() const
	{ 
		NX_READ_ZONE();
		return impl.getSupportDepth();
	}

	// NxApexActor methods
	virtual NxApexAsset* getOwner() const
	{
		NX_READ_ZONE();
		return impl.mAsset->getNxAsset();
	}
	virtual void release()
	{
		impl.release();
	}
	virtual void destroy()
	{
		impl.destroy();

		delete this;
	}

	virtual NxDestructibleRenderable* acquireRenderableReference()
	{
		return impl.acquireRenderableReference();
	}

	// NxApexRenderable methods
	virtual void updateRenderResources(bool rewriteBuffers, void* userRenderData)
	{
		URR_SCOPE;
		impl.updateRenderResources(rewriteBuffers, userRenderData);
	}

	virtual void dispatchRenderResources(NxUserRenderer& api)
	{
		impl.dispatchRenderResources(api);
	}

	virtual void lockRenderResources()
	{
		NxApexRenderable* renderable = impl.getRenderable();
		if (renderable != NULL)
		{
			renderable->lockRenderResources();
		}
	}

	virtual void unlockRenderResources()
	{
		NxApexRenderable* renderable = impl.getRenderable();
		if (renderable != NULL)
		{
			renderable->unlockRenderResources();
		}
	}

	virtual physx::PxBounds3 getBounds() const
	{
		NX_READ_ZONE();
		NxApexRenderable* renderable = const_cast<DestructibleActor*>(&impl)->getRenderable();
		if (renderable != NULL)
		{
			return renderable->getBounds();
		}
		return impl.getBounds();
	}

	// NxApexActorSource methods
#if NX_SDK_VERSION_MAJOR == 2
	virtual void setActorTemplate(const NxActorDescBase* desc)
	{
		NX_WRITE_ZONE();
		impl.setActorTemplate(desc);
	}
	virtual bool getActorTemplate(NxActorDescBase& dest) const
	{
		NX_READ_ZONE();
		return impl.getActorTemplate(dest);
	}
	virtual void setShapeTemplate(const NxShapeDesc* desc)
	{
		NX_WRITE_ZONE();
		impl.setShapeTemplate(desc);
	}
	virtual bool getShapeTemplate(NxShapeDesc& dest) const
	{
		NX_READ_ZONE();
		return impl.getShapeTemplate(dest);
	}
	virtual void setBodyTemplate(const NxBodyDesc* desc)
	{
		NX_WRITE_ZONE();
		impl.setBodyTemplate(desc);
	}
	virtual bool getBodyTemplate(NxBodyDesc& dest) const
	{
		NX_READ_ZONE();
		return impl.getBodyTemplate(dest);
	}
#elif NX_SDK_VERSION_MAJOR == 3
	virtual void setPhysX3Template(const NxPhysX3DescTemplate* desc)
	{
		NX_WRITE_ZONE();
		impl.setPhysX3Template(desc);
	}

	virtual bool getPhysX3Template(NxPhysX3DescTemplate& dest) const
	{
		NX_READ_ZONE();
		return impl.getPhysX3Template(dest);
	}

	NxPhysX3DescTemplate* createPhysX3DescTemplate() const
	{
		return impl.createPhysX3DescTemplate();
	}
#endif
	// NxApexResource methods
	virtual void	setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		NX_WRITE_ZONE();
		impl.m_listIndex = index;
		impl.m_list = &list;
	}
	virtual physx::PxU32	getListIndex() const
	{
		NX_READ_ZONE();
		return impl.m_listIndex;
	}

	virtual void setLODWeights(physx::PxF32 maxDistance, physx::PxF32 distanceWeight, physx::PxF32 maxAge, physx::PxF32 ageWeight, physx::PxF32 bias)
	{
		NX_WRITE_ZONE();
		return impl.setLODWeights(maxDistance, distanceWeight, maxAge, ageWeight, bias);
	}

	virtual void getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const
	{
		NX_READ_ZONE();
		impl.getPhysicalLodRange(min, max, intOnly);
	}

	virtual physx::PxF32 getActivePhysicalLod() const
	{
		NX_READ_ZONE();
		return impl.getActivePhysicalLod();
	}

	virtual void forcePhysicalLod(physx::PxF32 lod)
	{
		NX_WRITE_ZONE();
		impl.forcePhysicalLod(lod);
	}

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		NX_WRITE_ZONE();
		impl.setEnableDebugVisualization(state);
	}


	virtual void setCrumbleEmitterState(bool enable)
	{
		NX_WRITE_ZONE();
		impl.setCrumbleEmitterEnabled(enable);
	}

	virtual void setDustEmitterState(bool enable)
	{
		NX_WRITE_ZONE();
		impl.setDustEmitterEnabled(enable);
	}

	/**
		Sets a preferred render volume for a dust or crumble emitter
	*/
	virtual void setPreferredRenderVolume(NxApexRenderVolume* volume, NxDestructibleEmitterType::Enum type)
	{
		NX_WRITE_ZONE();
		impl.setPreferredRenderVolume(volume, type);
	}

	virtual NxApexEmitterActor* getApexEmitter(NxDestructibleEmitterType::Enum type)
	{
		NX_READ_ZONE();
		return impl.getApexEmitter(type);
	}

	virtual bool recreateApexEmitter(NxDestructibleEmitterType::Enum type)
	{
		NX_WRITE_ZONE();
		return impl.recreateApexEmitter(type);
	}

	const NxParameterized::Interface* getNxParameterized(NxDestructibleParameterizedType::Enum type) const
	{
		NX_READ_ZONE();
		switch (type) 
		{
		case NxDestructibleParameterizedType::State:
			return (const NxParameterized::Interface*)impl.getState();
		case NxDestructibleParameterizedType::Params:
			return (const NxParameterized::Interface*)impl.getParams();
		default:
			return NULL;
		}
	}

	void setNxParameterized(NxParameterized::Interface* params)
	{
		NX_WRITE_ZONE();
		impl.initialize(params);
	}

	const NxParameterized::Interface* getChunks() const
	{
		return (const NxParameterized::Interface*)impl.getChunks();
	}

	void setChunks(NxParameterized::Interface* chunks)
	{
		impl.initialize(chunks);
	}

    virtual bool setSyncParams(physx::PxU32 userActorID, physx::PxU32 actorSyncFlags, const NxDestructibleActorSyncState * actorSyncState, const NxDestructibleChunkSyncState * chunkSyncState)
    {
		NX_WRITE_ZONE();
        return impl.setSyncParams(userActorID, actorSyncFlags, actorSyncState, chunkSyncState);
    }

	virtual bool setHitChunkTrackingParams(bool flushHistory, bool startTracking, physx::PxU32 trackingDepth, bool trackAllChunks)
	{
		NX_WRITE_ZONE();
		return impl.setHitChunkTrackingParams(flushHistory, startTracking, trackingDepth, trackAllChunks);
	}

	virtual bool getHitChunkHistory(const NxDestructibleHitChunk *& hitChunkContainer, physx::PxU32 & hitChunkCount) const
	{
		NX_READ_ZONE();
		return impl.getHitChunkHistory(hitChunkContainer, hitChunkCount);
	}

	virtual bool forceChunkHits(const NxDestructibleHitChunk * hitChunkContainer, physx::PxU32 hitChunkCount, bool removeChunks = true, bool deferredEvent = false, physx::PxVec3 damagePosition = physx::PxVec3(0.0f), physx::PxVec3 damageDirection = physx::PxVec3(0.0f))
	{
		NX_WRITE_ZONE();
		return impl.forceChunkHits(hitChunkContainer, hitChunkCount, removeChunks, deferredEvent, damagePosition, damageDirection);
	}

	virtual bool getDamageColoringHistory(const NxDamageEventCoreData *& damageEventCoreDataContainer, physx::PxU32 & damageEventCoreDataCount) const
	{
		NX_READ_ZONE();
		return impl.getDamageColoringHistory(damageEventCoreDataContainer, damageEventCoreDataCount);
	}

	virtual bool forceDamageColoring(const NxDamageEventCoreData * damageEventCoreDataContainer, physx::PxU32 damageEventCoreDataCount)
	{
		NX_WRITE_ZONE();
		return impl.forceDamageColoring(damageEventCoreDataContainer, damageEventCoreDataCount);
	}

	virtual void setDeleteFracturedChunks(bool inDeleteChunkMode)
	{
		NX_WRITE_ZONE();
		impl.setDeleteFracturedChunks(inDeleteChunkMode);
	}

#if NX_SDK_VERSION_MAJOR == 2
	virtual void takeImpact(const physx::PxVec3& force, const physx::PxVec3& position, physx::PxU16 chunkIndex, NxActor const* damageImpactActor)
#elif NX_SDK_VERSION_MAJOR == 3
	virtual void takeImpact(const physx::PxVec3& force, const physx::PxVec3& position, physx::PxU16 chunkIndex, PxActor const* damageImpactActor)
#endif
	{
		NX_WRITE_ZONE();
		impl.takeImpact(force, position, chunkIndex, damageImpactActor);
	}

	virtual physx::PxU32			getCustomBehaviorGroupCount() const
	{
		NX_READ_ZONE();
		return impl.getCustomBehaviorGroupCount();
	}

	virtual bool					getBehaviorGroup(NxDestructibleBehaviorGroupDesc& behaviorGroupDesc, physx::PxI32 index = -1) const
	{
		NX_READ_ZONE();
		return impl.getBehaviorGroup(behaviorGroupDesc, index);
	}
};


}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLEACTOR_PROXY_H__

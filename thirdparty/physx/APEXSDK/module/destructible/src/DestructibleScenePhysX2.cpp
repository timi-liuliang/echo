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
#if NX_SDK_VERSION_MAJOR == 2

#include "NxApex.h"
#include "NiApexScene.h"
#include "ModuleDestructible.h"
#include "DestructibleScene.h"
#include "DestructibleAsset.h"
#include "DestructibleActor.h"
#include "DestructibleActorProxy.h"
#include "DestructibleActorJointProxy.h"
#if APEX_USE_PARTICLES
#include "NxApexEmitterActor.h"
#include "NxEmitterGeoms.h"
#endif

#include <NxScene.h>
#include <NxConvexMeshDesc.h>
#include <NxConvexShapeDesc.h>


#include "ModulePerfScope.h"
#include "PsString.h"

#include "NiApexRenderMeshAsset.h"

namespace physx
{
namespace apex
{
namespace destructible
{


DestructibleUserNotify::DestructibleUserNotify(ModuleDestructible& module, DestructibleScene* destructibleScene) :
	mModule(module),
	mDestructibleScene(destructibleScene)
{

}

bool DestructibleUserNotify::onJointBreak(physx::PxF32 breakingImpulse, NxJoint& /* brokenJoint */)
{
	PX_UNUSED(breakingImpulse);
	//PX_UNUSED(brokenJoint);
	return false;
}

void DestructibleUserNotify::onWake(NxActor** actors, physx::PxU32 count)
{
	if (mDestructibleScene->mUsingActiveTransforms)	// The remaining code in this function only updates the destructible actor awake list when not using active transforms
	{
		return;
	}

	for (physx::PxU32 i = 0; i < count; i++)
	{
		NxActor* actor = actors[i];
		const NiApexPhysXObjectDesc* desc = static_cast<const NiApexPhysXObjectDesc*>(mModule.mSdk->getPhysXObjectInfo(actor));
		if (desc != NULL)
		{
			const physx::PxU32 dActorCount = desc->mApexActors.size();
			for (physx::PxU32 j = 0; j < dActorCount; ++j)
			{

				const NxDestructibleActor* dActor = static_cast<const NxDestructibleActor*>(desc->mApexActors[j]);
				if (dActor != NULL)
				{
					if (desc->mApexActors[j]->getOwner()->getObjTypeID() == DestructibleAsset::getAssetTypeID())
					{
						DestructibleActor& destructibleActor = const_cast<DestructibleActor&>(static_cast<const DestructibleActorProxy*>(dActor)->impl);
						destructibleActor.incrementWakeCount();
					}
				}
			}
		}
	}
}

void DestructibleUserNotify::onSleep(NxActor** actors, physx::PxU32 count)
{
	if (mDestructibleScene->mUsingActiveTransforms)	// The remaining code in this function only updates the destructible actor awake list when not using active transforms
	{
		return;
	}

	for (physx::PxU32 i = 0; i < count; i++)
	{
		NxActor* actor = actors[i];
		const NiApexPhysXObjectDesc* desc = static_cast<const NiApexPhysXObjectDesc*>(mModule.mSdk->getPhysXObjectInfo(actor));
		if (desc != NULL)
		{
			const physx::PxU32 dActorCount = desc->mApexActors.size();
			for (physx::PxU32 j = 0; j < dActorCount; ++j)
			{
				const NxDestructibleActor* dActor = static_cast<const NxDestructibleActor*>(desc->mApexActors[j]);
				if (dActor != NULL)
				{
					if (desc->mApexActors[j]->getOwner()->getObjTypeID() == DestructibleAsset::getAssetTypeID())
					{
						DestructibleActor& destructibleActor = const_cast<DestructibleActor&>(static_cast<const DestructibleActorProxy*>(dActor)->impl);
						destructibleActor.decrementWakeCount();
					}
				}
			}
		}
	}
}


void DestructibleContactReport::onContactNotify(NxContactPair& pair, physx::PxU32 events)
{
	PX_PROFILER_PERF_SCOPE("DestructibleOnContactNotify");

	PX_UNUSED(events);
	PX_ASSERT(pair.actors[0] && pair.actors[1]);

#if (NX_SDK_VERSION_NUMBER >= 280)
	if (pair.isDeletedActor[0] || pair.isDeletedActor[1])
	{
		return;
	}
#endif

	ModuleDestructible* module = destructibleScene->mModule;
	int moduleOwnsActor[2] =
	{
		(int)module->owns(pair.actors[0]),
		(int)module->owns(pair.actors[1])
	};

	if (!(moduleOwnsActor[0] | moduleOwnsActor[1]))
	{
		return;	// Neither is owned by the destruction module
	}

	NxContactStreamIterator It(pair.stream);
	while (It.goNextPair())
	{
		DestructibleActor*				destructibles[2]				= {NULL, NULL};
		DestructibleStructure::Chunk*	chunks[2]						= {NULL, NULL};
		physx::PxF32							minImpactVelocityThresholdsSq =	NX_MAX_REAL;

#if (NX_SDK_VERSION_NUMBER > 280)
		// LZHACK: should ignore deleted shapes
		if (It.isDeletedShape(0) || It.isDeletedShape(1))
		{
			break;
		}
#endif

		for (int i = 0; i < 2; ++i)
		{
			NxShape* shape = It.getShape(i);
			if (moduleOwnsActor[i] && module->getDestructibleAndChunk(shape, NULL) == NULL)
			{
				chunks[i] = NULL;
			}
			else
			{
				chunks[i] = destructibleScene->getChunk(shape);
			}
			if (chunks[i] != NULL)
			{
				destructibles[i] = destructibleScene->mDestructibles.direct(chunks[i]->destructibleID);
				PX_ASSERT(destructibles[i] != NULL);
				if (destructibles[i] != NULL)
				{
					physx::PxF32 ivts = destructibles[i]->getDestructibleParameters().impactVelocityThreshold;
					ivts *= ivts;
					if (ivts < minImpactVelocityThresholdsSq)
					{
						minImpactVelocityThresholdsSq = ivts;
					}
				}
			}
		}
		if (destructibles[0] == destructibles[1])
		{
			return; // No self-collision.  To do: multiply by a self-collision factor instead?
		}

		const physx::PxF32 masses[2] =
		{
			pair.actors[0]->isDynamic() && pair.actors[0]->readBodyFlag(NX_BF_KINEMATIC) == 0 ? pair.actors[0]->getMass() : 0,
			pair.actors[1]->isDynamic() && pair.actors[1]->readBodyFlag(NX_BF_KINEMATIC) == 0 ? pair.actors[1]->getMass() : 0
		};

		physx::PxF32 reducedMass;
		if (masses[0] == 0.0f)
		{
			reducedMass = masses[1];
		}
		else if (masses[1] == 0.0f)
		{
			reducedMass = masses[0];
		}
		else
		{
			reducedMass = masses[0] * masses[1] / (masses[0] + masses[1]);
		}


		NxVec3 destructibleForces[2] = {NxVec3(0.0f), NxVec3(0.0f)};
		NxVec3 avgContactPosition    = NxVec3(0.0f);
		physx::PxU32  numContacts           = 0;

		while (It.goNextPatch())
		{
			const NxVec3& patchNormal = It.getPatchNormal();
			while (It.goNextPoint())
			{
				const NxVec3& position = It.getPoint();
				const NxVec3 velocities[2] =
				{
					pair.actors[0]->getPointVelocity(position),
					pair.actors[1]->getPointVelocity(position),
				};
				const NxVec3 velocityDelta = velocities[0] - velocities[1];
				if (velocityDelta.magnitudeSquared() >= minImpactVelocityThresholdsSq)
				{
					for (int i = 0; i < 2; ++i)
					{
						DestructibleActor* destructible = destructibles[i];
						if (destructible)
						{
							// this is not really physically correct, but at least its determenistic...
							destructibleForces[i] += (patchNormal * patchNormal.dot(velocityDelta)) * reducedMass * (i ? 1.0f : -1.0f);
						}
					}
					avgContactPosition += position;
					numContacts++;
				}
			}

			if (numContacts)
			{
				avgContactPosition /= (physx::PxF32)numContacts;
				for (physx::PxU32 i = 0; i < 2; i++)
				{
					const NxVec3 force = destructibleForces[i] / (physx::PxF32)numContacts;
					DestructibleActor* destructible = destructibles[i];
					if (destructible && !force.isZero())
					{
						destructible->takeImpact(PXFROMNXVEC3(force), PXFROMNXVEC3(avgContactPosition), chunks[i]->indexInAsset, pair.actors[i^1]);
					}
				}
			}
		}

	}
}

bool DestructibleContactModify::onContactConstraint(
    NxU32& changeFlags,
    const NxShape* shape0,
    const NxShape* shape1,
    const NxU32 featureIndex0,
    const NxU32 featureIndex1,
    NxContactCallbackData& data)
{
	PX_UNUSED(shape0);
	PX_UNUSED(shape1);
	PX_UNUSED(featureIndex0);
	PX_UNUSED(featureIndex1);

	changeFlags = NX_CCC_NONE;

	const NxActor* actors[2] = { &shape0->getActor(), &shape1->getActor() };

	ModuleDestructible* module = destructibleScene->mModule;
	const int moduleOwnsActor[2] = { (int)module->owns(actors[0]), (int)module->owns(actors[1]) };

	if (moduleOwnsActor[0] == moduleOwnsActor[1])
	{
		return true;	// Neither is owned by the destruction module, or both are
	}

	const int externalRBIndex = (int)(moduleOwnsActor[1] == 0);

	if (!actors[externalRBIndex]->isDynamic())
	{
		return true;
	}

	PxI32 chunkIndex = 0;
	DestructibleActorProxy* proxy = static_cast<DestructibleActorProxy*>(module->getDestructibleAndChunk(externalRBIndex ? shape0 : shape1, &chunkIndex));
	if (proxy != NULL)
	{
		const physx::PxF32 materialStrengh = proxy->impl.getBehaviorGroup(chunkIndex).materialStrength;
		if (materialStrengh > 0.0f)
		{
			data.maxImpulse = materialStrengh;
			changeFlags = NX_CCC_MAXIMPULSE;
		}
	}

	return true;
}

PX_INLINE const NxVec3* GetConvexMeshVertex(NxConvexMeshDesc* desc, physx::PxU32 index)
{
	return (const NxVec3*)((physx::PxU8*)desc->points + index * desc->pointStrideBytes);
}

NxActor* DestructibleScene::createRoot(DestructibleStructure::Chunk& chunk, const physx::PxMat34Legacy& pose, bool dynamic, physx::PxMat34Legacy* relTM, bool fromInitialData)
{
	PX_PROFILER_PERF_SCOPE("DestructibleCreateRoot");

	DestructibleActor* destructible = mDestructibles.direct(chunk.destructibleID);

	NxActor* actor = NULL;
	if (!chunk.isDestroyed())
	{
		actor = destructible->getStructure()->getChunkActor(chunk);
	}

	if ((chunk.state & ChunkVisible) != 0)
	{
		NxApexPhysXObjectDesc* actorObjDesc = (NxApexPhysXObjectDesc*) mModule->mSdk->getPhysXObjectInfo(actor);
		PX_ASSERT(actor && actorObjDesc && actor->getNbShapes() >= 1);
		if (destructible->getStructure()->chunkIsSolitary(chunk))
		{
			bool actorIsDynamic = (chunk.state & ChunkDynamic) != 0;
			if (dynamic == actorIsDynamic)
			{
				return actor;	// nothing to do
			}
			if (!dynamic)
			{
				actor->raiseBodyFlag(NX_BF_KINEMATIC);
				actorObjDesc->userData = NULL;
				return actor;
			}
#if 0	// XXX \todo: why doesn't this work?
			else
			{
				actor->clearBodyFlag(NX_BF_KINEMATIC);
				addActor(*actorDesc, *actor);
				return;
			}
#endif
		}
		scheduleChunkShapesForDelete(chunk);
	}

	const physx::PxU32 firstHullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk.indexInAsset);
	const physx::PxU32 hullCount = destructible->getAsset()->getChunkHullIndexStop(chunk.indexInAsset) - firstHullIndex;
	NxConvexShapeDesc* convexShapeDescs = (NxConvexShapeDesc*)PxAlloca(sizeof(NxConvexShapeDesc) * hullCount);

	DestructibleAssetParametersNS::Chunk_Type* source = destructible->getAsset()->mParams->chunks.buf + chunk.indexInAsset;

	// Whether or not this chunk can be damaged by an impact
	const bool takesImpactDamage = destructible->takesImpactDamageAtDepth(source->depth);

	// Get user-defined descriptors, modify as needed

	// Create a new actor
	NxActorDesc actorDesc;
	destructible->getActorTemplate(actorDesc);

	const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroup(chunk.indexInAsset);

	if (dynamic && destructible->getDestructibleParameters().dynamicChunksDominanceGroup < 32)
	{
		actorDesc.dominanceGroup = destructible->getDestructibleParameters().dynamicChunksDominanceGroup;
	}

	// Shape(s):
	for (physx::PxU32 hullIndex = 0; hullIndex < hullCount; ++hullIndex)
	{
		NxConvexShapeDesc& convexShapeDesc = convexShapeDescs[hullIndex];
		PX_PLACEMENT_NEW(&convexShapeDesc, NxConvexShapeDesc);
		destructible->getShapeTemplate(convexShapeDesc);
		convexShapeDesc.meshData = destructible->getConvexMesh(hullIndex + firstHullIndex);
		PX_ASSERT(convexShapeDesc.meshData != NULL);
		convexShapeDesc.shapeFlags &= ~(physx::PxU32)NX_SF_DISABLE_COLLISION;

		if (behaviorGroup.groupsMask.useGroupsMask)
		{
			convexShapeDesc.groupsMask.bits0 = behaviorGroup.groupsMask.bits0;
			convexShapeDesc.groupsMask.bits1 = behaviorGroup.groupsMask.bits1;
			convexShapeDesc.groupsMask.bits2 = behaviorGroup.groupsMask.bits2;
			convexShapeDesc.groupsMask.bits3 = behaviorGroup.groupsMask.bits3;

		}
		else if (dynamic && destructible->getDestructibleParameters().useDynamicChunksGroupsMask)
		{
			convexShapeDesc.groupsMask = destructible->getDestructibleParameters().dynamicChunksGroupsMask;
		}

		if (relTM != NULL)
		{
			NxFromPxMat34(convexShapeDesc.localPose, *relTM);
			convexShapeDesc.localPose.t += NxFromPxVec3Fast(destructible->getScale().multiply(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset)));
		}
		else
		{
			convexShapeDesc.localPose.id();
			convexShapeDesc.localPose.t = NxFromPxVec3Fast(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset));
		}

		actorDesc.shapes.pushBack(&convexShapeDesc);
	}

	// Calculate mass
	const NxReal actorMass = destructible->getChunkMass(chunk.indexInAsset);
	const NxReal scaledMass = scaleMass(actorMass);

	// Body:
	NxBodyDesc bodyDesc;
	destructible->getBodyTemplate(bodyDesc);

	bodyDesc.mass = scaledMass;

	if (!dynamic)
	{
		// Make kinematic if the chunk is not dynamic
		bodyDesc.flags |= NX_BF_KINEMATIC;
	}
	else
	{
		bodyDesc.flags &= ~(physx::PxU32)NX_BF_KINEMATIC;
	}
	if (takesImpactDamage)
	{
		// Set contact report threshold if the actor can take impact damage
#if (NX_SDK_VERSION_NUMBER >= 280)
		bodyDesc.contactReportThreshold = destructible->getContactReportThreshold(behaviorGroup);
#endif
	}

	// Actor:
	actorDesc.body = &bodyDesc;
	actorDesc.density = 0.0f; // // The mass is set explicitly
	NxFromPxMat34(actorDesc.globalPose, pose);
	if (!fromInitialData)
		actorDesc.globalPose.t -= actorDesc.globalPose.M*NxFromPxVec3Fast(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset));
	if (takesImpactDamage)
	{
		// Set contact report flags if the actor can take impact damage
#if (NX_SDK_VERSION_NUMBER >= 280)
		actorDesc.contactReportFlags =
		    NX_NOTIFY_FORCES |
		    NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD |
		    NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD;
#endif
	}

	if (takesImpactDamage && behaviorGroup.materialStrength > 0.0f)
	{
		actorDesc.flags |= NX_AF_CONTACT_MODIFICATION;
		bodyDesc.contactReportThreshold = behaviorGroup.materialStrength;
	}

	NxActor* newActor = NULL;

	// Create actor
	PX_ASSERT(actorDesc.isValid());
	newActor = mPhysXScene->createActor(actorDesc);
	PX_ASSERT(newActor);

	if (newActor)
	{
		if (getModule()->m_destructiblePhysXActorReport != NULL)
		{
			getModule()->m_destructiblePhysXActorReport->onPhysXActorCreate(*newActor);
		}

		++mNumActorsCreatedThisFrame;
		++mTotalChunkCount;

		NiApexPhysXObjectDesc* actorObjDesc = mModule->mSdk->createObjectDesc(destructible->getAPI(), newActor);
		destructible->referencedByActor(newActor);
		actorObjDesc->userData = 0;
		destructible->setActorObjDescFlags(actorObjDesc, source->depth);

		if (!newActor->readBodyFlag(NX_BF_KINEMATIC))
		{
			bool isDebris = destructible->getDestructibleParameters().debrisDepth >= 0
			                && source->depth >= destructible->getDestructibleParameters().debrisDepth;
			addActor(*actorObjDesc, *newActor, actorMass, isDebris);
		}

		// Set the NxShape for the chunk and all descendants
		chunk.setShapes(newActor->getShapes(), newActor->getNbShapes());
		chunk.state |= (physx::PxU32)ChunkVisible;
		if (dynamic)
		{
			chunk.state |= (physx::PxU32)ChunkDynamic;
			// New visible dynamic chunk, add to visible dynamic chunk shape counts
			destructible->increaseVisibleDynamicChunkShapeCount(hullCount);
			if (source->depth <= destructible->getDestructibleParameters().essentialDepth)
			{
				destructible->increaseEssentialVisibleDynamicChunkShapeCount(hullCount);
			}
		}

		// Create and store an island ID for the root actor
		if (actor != NULL || chunk.islandID == DestructibleStructure::InvalidID)
		{
			chunk.islandID = destructible->getStructure()->newNxActorIslandReference(chunk, *newActor);
		}

		VisibleChunkSetDescendents chunkOp(chunk.indexInAsset + destructible->getFirstChunkIndex(), dynamic);
		forSubtree(chunk, chunkOp, true);
		
		physx::Array<NxShape*>& shapes = destructible->getStructure()->getChunkShapes(chunk);
		for (physx::PxU32 i = shapes.size(); i--;)
		{
			NxShape* shape = shapes[i];
			NiApexPhysXObjectDesc* shapeObjDesc = mModule->mSdk->createObjectDesc(destructible->getAPI(), shape);
			shapeObjDesc->userData = &chunk;
		}

		newActor->wakeUp();
	}

	// Add to static root list if static
	if (!dynamic)
	{
		destructible->getStaticRoots().use(chunk.indexInAsset);
	}

	return newActor;
}

bool DestructibleScene::appendShapes(DestructibleStructure::Chunk& chunk, bool dynamic, physx::PxMat34Legacy* relTM, NxActor* actor)
{
	// PX_PROFILER_PERF_SCOPE("DestructibleAppendShape");

	if (chunk.state & ChunkVisible)
	{
		return true;
	}

	if (chunk.isDestroyed() && actor == NULL)
	{
		return false;
	}

	DestructibleActor* destructible = mDestructibles.direct(chunk.destructibleID);

	if (actor == NULL)
	{
		actor = destructible->getStructure()->getChunkActor(chunk);
		relTM = NULL;
	}

	const physx::PxU32 oldActorShapeCount = actor->getNbShapes();

	// Shape(s):
	for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
	{
		NxConvexShapeDesc convexShapeDesc;
		destructible->getShapeTemplate(convexShapeDesc);
		convexShapeDesc.meshData = destructible->getConvexMesh(hullIndex);
		// Make sure we can get a collision mesh
		if (!convexShapeDesc.meshData)
		{
			return false;
		}

		const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroup(chunk.indexInAsset);
		if (behaviorGroup.groupsMask.useGroupsMask)
		{
			convexShapeDesc.groupsMask.bits0 = behaviorGroup.groupsMask.bits0;
			convexShapeDesc.groupsMask.bits1 = behaviorGroup.groupsMask.bits1;
			convexShapeDesc.groupsMask.bits2 = behaviorGroup.groupsMask.bits2;
			convexShapeDesc.groupsMask.bits3 = behaviorGroup.groupsMask.bits3;
		}
		else if (dynamic && destructible->getDestructibleParameters().useDynamicChunksGroupsMask)
		{
			convexShapeDesc.groupsMask = destructible->getDestructibleParameters().dynamicChunksGroupsMask;
		}

		if (relTM != NULL)
		{
			NxFromPxMat34(convexShapeDesc.localPose, *relTM);
			convexShapeDesc.localPose.t += NxFromPxVec3Fast(destructible->getScale().multiply(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset)));
		}
		else
		{
			NxFromPxMat34(convexShapeDesc.localPose, destructible->getStructure()->getChunkLocalPose(chunk));
		}

		NxShape* newShape = NULL;

		newShape = actor->createShape(convexShapeDesc);
		PX_ASSERT(newShape);
		if (!newShape)
		{
			APEX_INTERNAL_ERROR("Failed to create the PhysX shape.");
			return false;
		}

		NiApexPhysXObjectDesc* shapeObjDesc = mModule->mSdk->createObjectDesc(destructible->getAPI(), newShape);
		shapeObjDesc->userData = &chunk;
	}

	++mTotalChunkCount;
	chunk.state |= (physx::PxU8)ChunkVisible;
	if (dynamic)
	{
		chunk.state |= (physx::PxU8)ChunkDynamic;
		// New visible dynamic chunk, add to visible dynamic chunk shape counts
		const physx::PxU32 hullCount = destructible->getAsset()->getChunkHullCount(chunk.indexInAsset);
		destructible->increaseVisibleDynamicChunkShapeCount(hullCount);
		if (destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset].depth <= destructible->getDestructibleParameters().essentialDepth)
		{
			destructible->increaseEssentialVisibleDynamicChunkShapeCount(hullCount);
		}
	}

	// Retrieve the island ID associated with the parent NxActor
	chunk.islandID = destructible->getStructure()->actorToIsland[actor];

	chunk.setShapes(actor->getShapes() + oldActorShapeCount, actor->getNbShapes() - oldActorShapeCount);
	VisibleChunkSetDescendents chunkOp(chunk.indexInAsset + destructible->getFirstChunkIndex(), dynamic);
	forSubtree(chunk, chunkOp, true);

	// Update the mass
	{
		NiApexPhysXObjectDesc* actorObjDesc = (NiApexPhysXObjectDesc*)mModule->mSdk->getPhysXObjectInfo(actor);
		const uintptr_t cindex = (uintptr_t)actorObjDesc->userData;
		if (cindex != 0)
		{
			// In the FIFO, trigger mass update
			PX_ASSERT(mActorFIFO[(physx::PxU32)~cindex].actor == actor);
			ActorFIFOEntry& FIFOEntry = mActorFIFO[(physx::PxU32)~cindex];
			FIFOEntry.unscaledMass += destructible->getChunkMass(chunk.indexInAsset);
			if (!actor->readBodyFlag(NX_BF_KINEMATIC))
			{
				FIFOEntry.flags |= ActorFIFOEntry::MassUpdateNeeded;
			}
		}
	}

	if ((chunk.state & ChunkDynamic) == 0)
	{
		destructible->getStaticRoots().use(chunk.indexInAsset);
	}

	return true;
}

class DestructibleTestWorldOverlapReport : public NxUserEntityReport<NxShape*>
{
public:

	DestructibleTestWorldOverlapReport(const ConvexHull& convexHull, const physx::PxMat34Legacy& tm, const physx::PxVec3& scale, physx::PxF32 padding) :
		m_convexHull(&convexHull), m_tm(tm), m_scale(scale), m_padding(padding), m_overlapFound(false) {}
	virtual ~DestructibleTestWorldOverlapReport() {}

	bool	overlapFound() const
	{
		return m_overlapFound;
	}

	// NxUserEntityReport<NxShape*> interface
	bool	onEvent(physx::PxU32 nbEntities, NxShape** entities)
	{
		NxShape** stop = entities + nbEntities;
		while (entities < stop)
		{
			NxShape* shape = *entities++;
			if (shape != NULL && m_convexHull->intersects(*shape, m_tm, m_scale, m_padding))
			{
				m_overlapFound = true;
				return false;	// Stop looking
			}
		}
		return true;
	}

private:

	const ConvexHull*		m_convexHull;
	physx::PxMat34Legacy	m_tm;
	physx::PxVec3			m_scale;
	physx::PxF32			m_padding;

	bool					m_overlapFound;
};

bool DestructibleScene::testWorldOverlap(const ConvexHull& convexHull, const physx::PxMat34Legacy& tm, const physx::PxVec3& scale, physx::PxF32 padding, const NxGroupsMask* groupsMask)
{
	NxScene* physxSceneToUse = m_worldSupportPhysXScene != NULL ? m_worldSupportPhysXScene : mPhysXScene;
	if (physxSceneToUse == NULL)
	{
		return false;
	}

	physx::PxMat34Legacy scaledTM = tm;
	scaledTM.M.multiplyDiagonal(scale);
	physx::PxBounds3 worldBounds = convexHull.getBounds();
	PxBounds3Transform(worldBounds, scaledTM.M, scaledTM.t);
	PX_ASSERT(!worldBounds.isEmpty());
	worldBounds.fattenFast(padding);

	DestructibleTestWorldOverlapReport report(convexHull, tm, scale, padding);
	NxBounds3 nxWorldBounds;
	NxFromPxBounds3(nxWorldBounds, worldBounds);
	physxSceneToUse->overlapAABBShapes(nxWorldBounds, NX_STATIC_SHAPES, 0, NULL, &report, ~(physx::PxU32)0, groupsMask, true);
	return report.overlapFound();
}
}
}
} // end namespace physx::apex

#endif // NX_SDK_VERSION_MAJOR == 2

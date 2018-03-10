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
#if NX_SDK_VERSION_MAJOR == 3

#include "NxApex.h"
#include "NiApexScene.h"
#include "ModuleDestructible.h"
#include "DestructibleScene.h"
#include "DestructibleAsset.h"
#include "DestructibleActor.h"
#include "DestructibleActorProxy.h"
#include "DestructibleActorJointProxy.h"
#include "NxApexReadWriteLock.h"
#if APEX_USE_PARTICLES
#include "NxApexEmitterActor.h"
#include "NxEmitterGeoms.h"
#endif

#include "PsArray.h"
#include "PxScene.h"
#include "PxPhysics.h"
#include "PxConvexMeshDesc.h"
#include "PxConvexMeshGeometry.h"
#include "PxBoxGeometry.h"
#include "PxSphereGeometry.h"

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
void DestructibleUserNotify::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
	PX_UNUSED(constraints);
	PX_UNUSED(count);
}

void DestructibleUserNotify::onWake(PxActor** actors, physx::PxU32 count)
{
	if (mDestructibleScene->mUsingActiveTransforms)	// The remaining code in this function only updates the destructible actor awake list when not using active transforms
	{
		return;
	}

	for (physx::PxU32 i = 0; i < count; i++)
	{
		PxActor* actor = actors[i];
		NiApexPhysXObjectDesc* desc = (NiApexPhysXObjectDesc*)(mModule.mSdk->getPhysXObjectInfo(actor));
		if (desc != NULL)
		{
			if (desc->getUserDefinedFlag(PhysXActorFlags::CREATED_THIS_FRAME) || desc->getUserDefinedFlag(PhysXActorFlags::IS_SLEEPING))
			{
				// Only increase the counter in the first wake call, or
				// when the state has changed since the last callback.
				// IS_SLEEPING has to be checked, because with PhysX 3.3 we also
				// receive a callback when the user calls putToSleep on an awake actor
				// and the SDK wakes it up again.
				
				// increase wake count on each referenced destructible
				const physx::PxU32 dActorCount = desc->mApexActors.size();
				for (physx::PxU32 i = 0; i < dActorCount; ++i)
				{
					const NxDestructibleActor* dActor = static_cast<const NxDestructibleActor*>(desc->mApexActors[i]);
					if (dActor != NULL)
					{
						if (desc->mApexActors[i]->getOwner()->getObjTypeID() == DestructibleAsset::getAssetTypeID())
						{
							DestructibleActor& destructibleActor = const_cast<DestructibleActor&>(static_cast<const DestructibleActorProxy*>(dActor)->impl);
							destructibleActor.incrementWakeCount();
						}
					}
				}

				// update externally stored state of this physx actor
				desc->setUserDefinedFlag(PhysXActorFlags::IS_SLEEPING, false);
			}

			desc->setUserDefinedFlag(PhysXActorFlags::CREATED_THIS_FRAME, false);
		}
	}
}

void DestructibleUserNotify::onSleep(PxActor** actors, physx::PxU32 count)
{
	if (mDestructibleScene->mUsingActiveTransforms)	// The remaining code in this function only updates the destructible actor awake list when not using active transforms
	{
		return;
	}

	for (physx::PxU32 i = 0; i < count; i++)
	{
		PxActor* actor = actors[i];
		NiApexPhysXObjectDesc* desc = (NiApexPhysXObjectDesc*)(mModule.mSdk->getPhysXObjectInfo(actor));
		if (desc != NULL)
		{
			if (desc->getUserDefinedFlag(PhysXActorFlags::CREATED_THIS_FRAME))
			{
				// first sleep callback must be ignored, as it's not a state change, but initialization
				desc->setUserDefinedFlag(PhysXActorFlags::CREATED_THIS_FRAME, false);
				desc->setUserDefinedFlag(PhysXActorFlags::IS_SLEEPING, true);
				continue;
			}

			if (!desc->getUserDefinedFlag(PhysXActorFlags::IS_SLEEPING))
			{
				// Only decrease the wake count, if the state has really changed to sleeping
				// since the last callback
				const physx::PxU32 dActorCount = desc->mApexActors.size();
				for (physx::PxU32 i = 0; i < dActorCount; ++i)
				{
					const NxDestructibleActor* dActor = static_cast<const NxDestructibleActor*>(desc->mApexActors[i]);
					if (dActor != NULL)
					{
						if (desc->mApexActors[i]->getOwner()->getObjTypeID() == DestructibleAsset::getAssetTypeID())
						{
							DestructibleActor& destructibleActor = const_cast<DestructibleActor&>(static_cast<const DestructibleActorProxy*>(dActor)->impl);
							destructibleActor.decrementWakeCount();
						}
					}
				}

				desc->setUserDefinedFlag(PhysXActorFlags::IS_SLEEPING, true);
			}
		}
	}
}


void DestructibleUserNotify::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	if (pairHeader.flags & physx::PxContactPairHeaderFlag::eDELETED_ACTOR_0 ||
		pairHeader.flags & physx::PxContactPairHeaderFlag::eDELETED_ACTOR_1 ||
		pairHeader.actors[0] == NULL ||
		pairHeader.actors[1] == NULL)
	{
		return;
	}

	ModuleDestructible* module = mDestructibleScene->mModule;
	int moduleOwnsActor[2] =
	{
		(int)module->owns(pairHeader.actors[0]->isRigidActor()),
		(int)module->owns(pairHeader.actors[1]->isRigidActor())
	};

	if (!(moduleOwnsActor[0] | moduleOwnsActor[1]))
	{
		return;	// Neither is owned by the destruction module
	}

	for (PxU32 pairIdx=0; pairIdx<nbPairs; pairIdx++)
	{
		const PxContactPair& currentPair = pairs[pairIdx];

		if (currentPair.flags & physx::PxContactPairFlag::eDELETED_SHAPE_0 || 
			currentPair.flags & physx::PxContactPairFlag::eDELETED_SHAPE_1 ||
			currentPair.shapes[0] == NULL ||
			currentPair.shapes[1] == NULL)
		{
			continue;
		}

		DestructibleActor*				destructibles[2]				= {NULL, NULL};
		DestructibleStructure::Chunk*	chunks[2]						= {NULL, NULL};
		bool							takesImpactDamage[2]			= {false, false};
		physx::PxF32					minImpactVelocityThresholdsSq	= PX_MAX_REAL;
		
		for (int i = 0; i < 2; ++i)
		{
			NxShape* shape = currentPair.shapes[i];
			if (moduleOwnsActor[i] && module->getDestructibleAndChunk(shape, NULL) == NULL)
			{
				chunks[i] = NULL;
			}
			else
			{
				chunks[i] = mDestructibleScene->getChunk(shape);
			}
			if (chunks[i] != NULL)
			{
				destructibles[i] = mDestructibleScene->mDestructibles.direct(chunks[i]->destructibleID);
				PX_ASSERT(destructibles[i] != NULL);
				if (destructibles[i] != NULL)
				{
					physx::PxF32 ivts = destructibles[i]->getDestructibleParameters().impactVelocityThreshold;
					ivts *= ivts;
					if (ivts < minImpactVelocityThresholdsSq)
					{
						minImpactVelocityThresholdsSq = ivts;
					}

					PxI32 depth = destructibles[i]->getAsset()->getChunkDepth(chunks[i]->indexInAsset);
					takesImpactDamage[i] = destructibles[i]->takesImpactDamageAtDepth((physx::PxU32)depth);
				}
			}
		}
		if (destructibles[0] == destructibles[1])
		{
			return; // No self-collision.  To do: multiply by a self-collision factor instead?
		}

		if (!takesImpactDamage[0] && !takesImpactDamage[1])
		{
			return;
		}

		const physx::PxF32 masses[2] =
		{
			((NxActor*)pairHeader.actors[0])->isDynamic() && ((NxActor*)pairHeader.actors[0])->readBodyFlag(NX_BF_KINEMATIC) == 0 ? ((NxActor*)pairHeader.actors[0])->getMass() : 0,
			((NxActor*)pairHeader.actors[1])->isDynamic() && ((NxActor*)pairHeader.actors[1])->readBodyFlag(NX_BF_KINEMATIC) == 0 ? ((NxActor*)pairHeader.actors[1])->getMass() : 0
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
		NxVec3 avgContactNormal		 = NxVec3(0.0f);
		physx::PxU32  numContacts           = 0;

#if USE_EXTRACT_CONTACTS
#if PAIR_POINT_ALLOCS
		PxContactPairPoint* pairPointBuffer = (PxContactPairPoint*)PX_ALLOC(currentPair.contactCount * sizeof(PxContactPairPoint), PX_DEBUG_EXP("PxContactPairPoints"));
#else
		mPairPointBuffer.reserve(currentPair.contactCount * sizeof(PxContactPairPoint));

		// if this method isn't used, the operator[] method will fail because the actual size may be zero
		mPairPointBuffer.forceSize_Unsafe(currentPair.contactCount * sizeof(PxContactPairPoint));
		PxContactPairPoint* pairPointBuffer =  currentPair.contactCount > 0 ? (PxContactPairPoint*)&(mPairPointBuffer[0]) : NULL;
#endif
		physx::PxU32 numContactsInStream = pairPointBuffer != NULL ? currentPair.extractContacts(pairPointBuffer, currentPair.contactCount) : 0;
#else
		physx::PxU32 numContactsInStream = currentPair.contactCount;
		const PxContactPoint* contacts = reinterpret_cast<const PxContactPoint*>(currentPair.contactStream);
#endif


		for (physx::PxU32 contactIdx=0; contactIdx<numContactsInStream; contactIdx++)
		{
#if USE_EXTRACT_CONTACTS
			PxContactPairPoint& currentPoint = pairPointBuffer[contactIdx];

			const NxVec3& patchNormal = currentPoint.normal;
			const NxVec3& position = currentPoint.position;
#else
			const PxContactPoint& cp = contacts[contactIdx];

			const NxVec3& patchNormal = cp.normal;
			const NxVec3& position = cp.point;
#endif
			const NxVec3 velocities[2] =
			{
				((NxActor*)pairHeader.actors[0])->getPointVelocity(position),
				((NxActor*)pairHeader.actors[1])->getPointVelocity(position),
			};
			const NxVec3 velocityDelta = velocities[0] - velocities[1];
			if (velocityDelta.magnitudeSquared() >= minImpactVelocityThresholdsSq || reducedMass == 0.0f)	// If reduced mass == 0, this is kineamtic vs. kinematic.  Generate damage.
			{
				for (int i = 0; i < 2; ++i)
				{
					DestructibleActor* destructible = destructibles[i];
					if (destructible)
					{
						// this is not really physically correct, but at least its deterministic...
						destructibleForces[i] += (patchNormal * patchNormal.dot(velocityDelta)) * reducedMass * (i ? 1.0f : -1.0f);
					}
				}
				avgContactPosition += position;
				avgContactNormal += patchNormal;
				numContacts++;
			}
			if (numContacts)
			{
				avgContactPosition /= (physx::PxF32)numContacts;
				avgContactNormal.normalize();
				for (physx::PxU32 i = 0; i < 2; i++)
				{
					if (!takesImpactDamage[i])
						continue;

					const NxVec3 force = destructibleForces[i] / (physx::PxF32)numContacts;
					DestructibleActor* destructible = destructibles[i];
					if (destructible != NULL)
					{
						if (!force.isZero())
						{
							destructible->takeImpact(PXFROMNXVEC3(force), PXFROMNXVEC3(avgContactPosition), chunks[i]->indexInAsset, (NxActor*)pairHeader.actors[i^1]);
						}
						else if (reducedMass == 0.0f)	// Handle kineamtic vs. kinematic
						{
							const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroup(chunks[i]->indexInAsset);
							destructible->applyDamage(2.0f*behaviorGroup.damageThreshold, 0.0f, PXFROMNXVEC3(avgContactPosition), (avgContactNormal * (i ? 1.0f : -1.0f)), chunks[i]->indexInAsset);
						}
					}
				}
			}
		}

#if PAIR_POINT_ALLOCS
		if (pairPointBuffer != NULL)
		{
			PX_FREE(pairPointBuffer);
		}
#endif
	}
}


void DestructibleUserNotify::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	PX_UNUSED(pairs);
	PX_UNUSED(count);
}

void DestructibleContactModify::onContactModify(PxContactModifyPair* const pairs, PxU32 count)
{
	PX_PROFILER_PERF_SCOPE("DestructibleOnContactConstraint");

	for (PxU32 iPair = 0 ; iPair < count; iPair++)
	{
		PxContactModifyPair&	pair	= pairs[iPair];

		ModuleDestructible* module = destructibleScene->mModule;

		PxI32 chunkIndex0 = 0;
		DestructibleActorProxy* proxy0 = static_cast<DestructibleActorProxy*>(module->getDestructibleAndChunk((NxShape*)pair.shape[0], &chunkIndex0));
		PxI32 chunkIndex1 = 0;
		DestructibleActorProxy* proxy1 = static_cast<DestructibleActorProxy*>(module->getDestructibleAndChunk((NxShape*)pair.shape[1], &chunkIndex1));

		const bool moduleOwnsActor[2] = { proxy0 != NULL, proxy1 != NULL };

		if (moduleOwnsActor[0] == moduleOwnsActor[1])
		{
			continue;	// Neither is owned by the destruction module, or both are
		}

		const int externalRBIndex = (int)(moduleOwnsActor[1] == 0);

		destructibleScene->mApexScene->getPhysXScene()->lockRead();
		const bool externalActorDynamic = pair.actor[externalRBIndex]->isRigidDynamic() != NULL;
		destructibleScene->mApexScene->getPhysXScene()->unlockRead();

		if (!externalActorDynamic)
		{
			continue;
		}

		DestructibleActorProxy* proxy = externalRBIndex ? proxy0 : proxy1;
		const physx::PxF32 materialStrength = proxy->impl.getBehaviorGroup(physx::PxU32(externalRBIndex ? chunkIndex0 :chunkIndex1)).materialStrength;
		if (materialStrength > 0.0f)
		{
			for (physx::PxU32 contactIndex = 0; contactIndex < pair.contacts.size(); ++contactIndex)
			{
				pair.contacts.setMaxImpulse(contactIndex, materialStrength);
			}
		}
	}
}

PX_INLINE const PxVec3* GetConvexMeshVertex(PxConvexMeshDesc* desc, physx::PxU32 index)
{
	return (const PxVec3*)((physx::PxU8*)desc->points.data + index * desc->points.stride);
}
NxActor* DestructibleScene::createRoot(DestructibleStructure::Chunk& chunk, const physx::PxMat34Legacy& pose, bool dynamic, physx::PxMat34Legacy* relTM, bool fromInitialData)
{
	// apan2 need verify 3.0 (createRoot)
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
				actor->isRigidDynamic()->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
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
	PxConvexMeshGeometry* convexShapeDescs = (PxConvexMeshGeometry*)PxAlloca(sizeof(PxConvexMeshGeometry) * hullCount);

	DestructibleAssetParametersNS::Chunk_Type* source = destructible->getAsset()->mParams->chunks.buf + chunk.indexInAsset;

	// Whether or not this chunk can be damaged by an impact
	const bool takesImpactDamage = destructible->takesImpactDamageAtDepth(source->depth);

	// Get user-defined descriptors, modify as needed

	// Create a new actor
	PhysX3DescTemplate physX3Template;
	destructible->getPhysX3Template(physX3Template);

	const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroup(chunk.indexInAsset);

	PxRigidDynamic* newActor = NULL;
	PxMat34Legacy offsetPose = pose;
	if (!fromInitialData)
		offsetPose.t -= offsetPose.M*destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset);
	PxTransform	poseActor(offsetPose.toPxTransform());

	{
		// there's a bug for PxMat34Legacy, crash in the following case:
		//		PxMat34Legacy legacy(PxMat33Legacy(PxVec3(0.00013983250f, 0.99992913f, -6.9439411e-005f),
		//									PxVec3(-6.8843365e-005f, 0.00014030933f, 0.99992913f),
		//									PxVec3(0.99992913f, -6.8962574e-005f, 0.00013971329f)), 
		//							PxVec3(19.999702f, 11.549180f, -40.000011f));  
		//		PxTransform pose = legacy.toPxTransform();  
		//		PX_ASSERT(pose.isValid());  // <--- CRASH !!!!
		poseActor.q.normalize();	
	}

	newActor	= mPhysXScene->getPhysics().createRigidDynamic(poseActor);

	PX_ASSERT(newActor && "creating actor failed");
	if (!newActor)
		return NULL;

	physX3Template.apply(newActor);

	if (dynamic && destructible->getDestructibleParameters().dynamicChunksDominanceGroup < 32)
	{
		newActor->setDominanceGroup(destructible->getDestructibleParameters().dynamicChunksDominanceGroup);
	}

	newActor->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
	newActor->setMaxDepenetrationVelocity(behaviorGroup.maxDepenetrationVelocity);

	// Shape(s):
	for (physx::PxU32 hullIndex = 0; hullIndex < hullCount; ++hullIndex)
	{
		PxConvexMeshGeometry& convexShapeDesc = convexShapeDescs[hullIndex];
		PX_PLACEMENT_NEW(&convexShapeDesc, PxConvexMeshGeometry);
		convexShapeDesc.convexMesh = destructible->getConvexMesh(hullIndex + firstHullIndex);
		PX_ASSERT(convexShapeDesc.convexMesh != NULL);
		convexShapeDesc.scale.scale = destructible->getScale();
		// Divide out the cooking scale to get the proper scaling
		const physx::PxVec3 cookingScale = getModule()->getChunkCollisionHullCookingScale();
		convexShapeDesc.scale.scale.x /= cookingScale.x;
		convexShapeDesc.scale.scale.y /= cookingScale.y;
		convexShapeDesc.scale.scale.z /= cookingScale.z;
		if ((convexShapeDesc.scale.scale - physx::PxVec3(1.0f)).abs().maxElement() < 10*PX_EPS_F32)
		{
			convexShapeDesc.scale.scale = physx::PxVec3(1.0f);
		}

		PxShape*	shape;

		PxTransform	localPose;
		if (relTM != NULL)
		{
			localPose	= relTM->toPxTransform();
			localPose.p += destructible->getScale().multiply(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset));
		}
		else
		{
			localPose	= PxTransform(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset));
		}

		shape	= newActor->createShape(convexShapeDesc, 
			physX3Template.materials.begin(), 
			static_cast<physx::PxU16>(physX3Template.materials.size()),
			static_cast<PxShapeFlags>(physX3Template.shapeFlags));
		PX_ASSERT(shape);
		if (!shape)
		{
			APEX_INTERNAL_ERROR("Failed to create the PhysX shape.");
			return NULL;
		}
		shape->setLocalPose(localPose);
		physX3Template.apply(shape);

		if (behaviorGroup.groupsMask.useGroupsMask)
		{
			physx::PxFilterData filterData(behaviorGroup.groupsMask.bits0,
										   behaviorGroup.groupsMask.bits1,
										   behaviorGroup.groupsMask.bits2,
										   behaviorGroup.groupsMask.bits3);
			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);
		}
		else if (dynamic && destructible->getDestructibleParameters().useDynamicChunksGroupsMask)
		{
			// Override the filter data
			shape->setSimulationFilterData(destructible->getDestructibleParameters().dynamicChunksFilterData);
			shape->setQueryFilterData(destructible->getDestructibleParameters().dynamicChunksFilterData);
		}

		// apan2 
		PxPairFlags	pairFlag	= (PxPairFlags)physX3Template.contactReportFlags;
		if (takesImpactDamage)
		{
			pairFlag	/* |= PxPairFlag::eNOTIFY_CONTACT_FORCES */
						|=  PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS
						|  PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND
						|  PxPairFlag::eNOTIFY_CONTACT_POINTS;

		}

		if (behaviorGroup.materialStrength > 0.0f)
		{
			pairFlag |= PxPairFlag::eMODIFY_CONTACTS;
		}

		if (mApexScene->getApexPhysX3Interface())
			mApexScene->getApexPhysX3Interface()->setContactReportFlags(shape, pairFlag, destructible->getAPI(), chunk.indexInAsset);

	}

	// Calculate mass
	const NxReal actorMass = destructible->getChunkMass(chunk.indexInAsset);
	const NxReal scaledMass = scaleMass(actorMass);

	if (!dynamic)
	{
		// Make kinematic if the chunk is not dynamic
		newActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
	}
	else
	{
		newActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false);
	}

	if (takesImpactDamage)
	{
		// Set contact report threshold if the actor can take impact damage
		newActor->setContactReportThreshold(destructible->getContactReportThreshold(behaviorGroup));
	}

	// Actor:
	if (takesImpactDamage && behaviorGroup.materialStrength > 0.0f)
	{
		newActor->setContactReportThreshold(behaviorGroup.materialStrength);
	}

	if (newActor)
	{
		// apan2
		// bodyDesc.apply(newActor);
		PxRigidBodyExt::setMassAndUpdateInertia(*newActor->isRigidDynamic(), scaledMass);
		newActor->setAngularDamping(0.05f);

		++mNumActorsCreatedThisFrame;
		++mTotalChunkCount;

		NiApexPhysXObjectDesc* actorObjDesc = mModule->mSdk->createObjectDesc(destructible->getAPI(), newActor);
		actorObjDesc->userData = 0;
		destructible->setActorObjDescFlags(actorObjDesc, source->depth);

		if (!(newActor->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC))
		{
			bool isDebris = destructible->getDestructibleParameters().debrisDepth >= 0
			                && source->depth >= destructible->getDestructibleParameters().debrisDepth;
			addActor(*actorObjDesc, *(NxActor*)newActor, actorMass, isDebris);
		}

		// Set the NxShape for the chunk and all descendants
		const physx::PxU32 shapeCount = newActor->getNbShapes();
		PX_ALLOCA(shapeArray, physx::PxShape*, shapeCount);
		newActor->getShapes(shapeArray, shapeCount);

		chunk.setShapes(shapeArray, shapeCount);
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
			chunk.islandID = destructible->getStructure()->newNxActorIslandReference(chunk, *(NxActor*)newActor);
		}

		VisibleChunkSetDescendents chunkOp(chunk.indexInAsset+destructible->getFirstChunkIndex(), dynamic);
		forSubtree(chunk, chunkOp, true);

		physx::Array<NxShape*>& shapes = destructible->getStructure()->getChunkShapes(chunk);
		for (physx::PxU32 i = shapes.size(); i--;)
		{
			NxShape* shape = shapes[i];
			NiApexPhysXObjectDesc* shapeObjDesc = mModule->mSdk->createObjectDesc(destructible->getAPI(), shape);
			shapeObjDesc->userData = &chunk;
		}

		{
			mActorsToAddIndexMap[newActor] = mActorsToAdd.size();
			mActorsToAdd.pushBack(newActor);

			// TODO do we really need this?
			SCOPED_PHYSX3_LOCK_WRITE(mPhysXScene);
			if ( !(newActor->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC) ) // cannot call 'wake-up' on kinematic bodies
			{
				PxReal initWakeCounter = mPhysXScene->getWakeCounterResetValue();
				newActor->setWakeCounter(initWakeCounter);
			}
		}

		destructible->referencedByActor((NxActor*)newActor);	// Must be done after adding to scene, since sleeping is checked
	}

	// Add to static root list if static
	if (!dynamic)
	{
		destructible->getStaticRoots().use(chunk.indexInAsset);
	}

	if (newActor && getModule()->m_destructiblePhysXActorReport != NULL)
	{
		getModule()->m_destructiblePhysXActorReport->onPhysXActorCreate(*newActor);
	}

	return (NxActor*)newActor;
}

void DestructibleScene::addForceToAddActorsMap(physx::PxActor* actor, const ActorForceAtPosition& force)
{
	mForcesToAddToActorsMap[actor] = force;
}

void DestructibleScene::addActorsToScene()
{
	PX_PROFILER_PERF_SCOPE("DestructibleAddActorsToScene");

	if (mActorsToAdd.empty() || mPhysXScene == NULL)
		return;

	{
		SCOPED_PHYSX3_LOCK_WRITE(mPhysXScene);
		mPhysXScene->addActors(&mActorsToAdd[0], mActorsToAdd.size());

		{
			PX_PROFILER_PERF_SCOPE("DestructibleAddForcesToNewSceneActors");
			for(HashMap<physx::PxActor*, ActorForceAtPosition>::Iterator iter = mForcesToAddToActorsMap.getIterator(); !iter.done(); iter++)
			{
				PxActor* const actor = iter->first;
				ActorForceAtPosition& forceToAdd = iter->second;
				if (actor->getScene())
				{
					PxRigidDynamic* rigidDynamic = actor->isRigidDynamic();
					if (rigidDynamic && !(rigidDynamic->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC))
					{
						if (!forceToAdd.force.isZero())
						{
							PxRigidBody* rigidBody = actor->isRigidBody();
							if (rigidBody)
							{
								if (forceToAdd.usePosition)
								{
									PxRigidBodyExt::addForceAtPos(*rigidBody, forceToAdd.force, forceToAdd.pos, forceToAdd.mode, forceToAdd.wakeup);
								}
								else
								{
									rigidBody->addForce(forceToAdd.force, forceToAdd.mode, forceToAdd.wakeup);
								}
							}
						}
						else
						{
							// No force, but we will apply the wakeup flag
							if (forceToAdd.wakeup)
							{
								rigidDynamic->wakeUp();
							}
							else
							{
								rigidDynamic->putToSleep();
							}
						}
					}
				}
			}
			mForcesToAddToActorsMap.clear();
		}
	}

	mActorsToAdd.clear();
	mActorsToAddIndexMap.clear();
}


bool DestructibleScene::appendShapes(DestructibleStructure::Chunk& chunk, bool dynamic, physx::PxMat34Legacy* relTM, NxActor* actor)
{
	// PX_PROFILER_PERF_SCOPE(DestructibleAppendShape);

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

	PhysX3DescTemplate physX3Template;

	destructible->getPhysX3Template(physX3Template);

	DestructibleAssetParametersNS::Chunk_Type* source = destructible->getAsset()->mParams->chunks.buf + chunk.indexInAsset;
	const bool takesImpactDamage = destructible->takesImpactDamageAtDepth(source->depth);

	// Shape(s):
	for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
	{
		PxConvexMeshGeometry convexShapeDesc;
		convexShapeDesc.convexMesh = destructible->getConvexMesh(hullIndex);
		// Make sure we can get a collision mesh
		if (!convexShapeDesc.convexMesh)
		{
			return false;
		}
		convexShapeDesc.scale.scale = destructible->getScale();
		// Divide out the cooking scale to get the proper scaling
		const physx::PxVec3 cookingScale = getModule()->getChunkCollisionHullCookingScale();
		convexShapeDesc.scale.scale.x /= cookingScale.x;
		convexShapeDesc.scale.scale.y /= cookingScale.y;
		convexShapeDesc.scale.scale.z /= cookingScale.z;
		if ((convexShapeDesc.scale.scale - physx::PxVec3(1.0f)).abs().maxElement() < 10*PX_EPS_F32)
		{
			convexShapeDesc.scale.scale = physx::PxVec3(1.0f);
		}

		PxTransform	localPose;
		if (relTM != NULL)
		{
			localPose	= relTM->toPxTransform();
			localPose.p += destructible->getScale().multiply(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset));
		}
		else
		{
			localPose	= destructible->getStructure()->getChunkLocalPose(chunk).toPxTransform();
		}

		PxShape* newShape = NULL;

		{
			{
				SCOPED_PHYSX3_LOCK_WRITE(actor->getScene());
				newShape = actor->isRigidActor()->createShape(convexShapeDesc, 
					physX3Template.materials.begin(), 
					static_cast<physx::PxU16>(physX3Template.materials.size()),
					static_cast<PxShapeFlags>(physX3Template.shapeFlags));
				newShape->setLocalPose(localPose);

				PX_ASSERT(newShape);
				if (!newShape)
				{
					APEX_INTERNAL_ERROR("Failed to create the PhysX shape.");
					return false;
				}
				physX3Template.apply(newShape);
			}

			const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroup(chunk.indexInAsset);
			if (behaviorGroup.groupsMask.useGroupsMask)
			{
				physx::PxFilterData filterData(behaviorGroup.groupsMask.bits0,
											   behaviorGroup.groupsMask.bits1,
											   behaviorGroup.groupsMask.bits2,
											   behaviorGroup.groupsMask.bits3);
				newShape->setSimulationFilterData(filterData);
				newShape->setQueryFilterData(filterData);
			}
			else if (dynamic && destructible->getDestructibleParameters().useDynamicChunksGroupsMask)
			{
				// Override the filter data
				newShape->setSimulationFilterData(destructible->getDestructibleParameters().dynamicChunksFilterData);
				newShape->setQueryFilterData(destructible->getDestructibleParameters().dynamicChunksFilterData);
			}

			PxPairFlags	pairFlag	= (PxPairFlags)physX3Template.contactReportFlags;

			if (takesImpactDamage)
			{
				pairFlag	/* |= PxPairFlag::eNOTIFY_CONTACT_FORCES */
							|=  PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS
							|  PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND
							|  PxPairFlag::eNOTIFY_CONTACT_POINTS;
			}

			if (behaviorGroup.materialStrength > 0.0f)
			{
				pairFlag |= PxPairFlag::eMODIFY_CONTACTS;
			}

			if (mApexScene->getApexPhysX3Interface())
				mApexScene->getApexPhysX3Interface()->setContactReportFlags(newShape, pairFlag, destructible->getAPI(), chunk.indexInAsset);
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
	chunk.islandID = destructible->getStructure()->actorToIsland[(NxActor*)actor];

	const physx::PxU32 shapeCount = actor->getNbShapes();
	PX_ALLOCA(shapeArray, physx::PxShape*, shapeCount);
	PxRigidActor* rigidActor = actor->isRigidActor();

	{
		SCOPED_PHYSX3_LOCK_READ(rigidActor->getScene());
		rigidActor->getShapes(shapeArray, shapeCount);
	}

	chunk.setShapes(shapeArray + oldActorShapeCount, shapeCount - oldActorShapeCount);
	VisibleChunkSetDescendents chunkOp(chunk.indexInAsset+destructible->getFirstChunkIndex(), dynamic);
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

bool DestructibleScene::testWorldOverlap(const ConvexHull& convexHull, const physx::PxMat34Legacy& tm, const physx::PxVec3& scale, physx::PxF32 padding, const PxFilterData* groupsMask)
{
	physx::PxScene* physxSceneToUse = m_worldSupportPhysXScene != NULL ? m_worldSupportPhysXScene : mPhysXScene;
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

	PxBoxGeometry			box(worldBounds.getExtents());
	PxSceneQueryFilterData	filterData;
	filterData.flags	= PxSceneQueryFilterFlag::eSTATIC;
	if (groupsMask)
		filterData.data	= *groupsMask;
	SCOPED_PHYSX3_LOCK_READ(physxSceneToUse);
	PxOverlapBuffer ovBuffer(&mOverlapHits[0], MAX_SHAPE_COUNT);
	physxSceneToUse->overlap(box, PxTransform(worldBounds.getCenter()), ovBuffer, filterData, NULL);
	PxU32	nbHit = ovBuffer.getNbAnyHits();
	//nbHit	= nbHit >= 0 ? nbHit : MAX_SHAPE_COUNT; //Ivan: it is always true and should be removed
	for (PxU32 i = 0 ; i < nbHit; i++)
	{
		if (convexHull.intersects(*mOverlapHits[i].shape, tm, scale, padding))
		{
			return true;
		}
	}
	return false;
}

}
}
} // end namespace physx::apex

#endif //NX_SDK_VERSION_MAJOR == 3

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
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "NxApex.h"
#include "DestructibleStructure.h"
#include "DestructibleScene.h"
#include "ModuleDestructible.h"
#include "ModulePerfScope.h"
#include "PxIOStream.h"
#if NX_SDK_VERSION_MAJOR == 2
#include <NxPlaneShapeDesc.h>
#include <NxScene.h>
#include <NxConvexShapeDesc.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include <PxPhysics.h>
#include <PxScene.h>
#include "PxConvexMeshGeometry.h"
#endif

#if APEX_USE_PARTICLES
#include "NxApexEmitterActor.h"
#include "NxEmitterGeoms.h"
#endif

#include "NiApexRenderDebug.h"
#include "DestructibleActorProxy.h"

#include "DestructibleStructureStressSolver.h"

#if APEX_RUNTIME_FRACTURE
#include "SimScene.h"
#endif

#include "ApexMerge.h"

namespace physx
{
namespace apex
{
namespace destructible
{

// Local definitions

#define REDUCE_DAMAGE_TO_CHILD_CHUNKS				1

// DestructibleStructure methods

DestructibleStructure::DestructibleStructure(DestructibleScene* inScene, physx::PxU32 inID) :
	dscene(inScene),
	ID(inID),
	supportDepthChunksNotExternallySupportedCount(0),
	supportInvalid(false),
	actorForStaticChunks(NULL),
	stressSolver(NULL)
{
}

DestructibleStructure::~DestructibleStructure()
{
	if(NULL != stressSolver)
	{
		PX_DELETE(stressSolver);
		stressSolver = NULL;
		dscene->setStressSolverTick(this, false);
	}
	
	for (physx::PxU32 destructibleIndex = destructibles.size(); destructibleIndex--;)
	{
		DestructibleActor* destructible = destructibles[destructibleIndex];
		if (destructible)
		{
			destructible->setStructure(NULL);
			dscene->mDestructibles.direct(destructible->getID()) = NULL;
			dscene->mDestructibles.free(destructible->getID());
		}
	}

	dscene->setStructureSupportRebuild(this, false);
	dscene->setStructureUpdate(this, false);

	for (physx::PxU32 i = 0; i < chunks.size(); ++i)
	{
		Chunk& chunk = chunks[i];
		if ((chunk.state & ChunkVisible) != 0)
		{
			physx::Array<NxShape*>& shapeArray = getChunkShapes(chunk);
			for (physx::PxU32 i = shapeArray.size(); i--;)
			{
				NxShape* shape = shapeArray[i];
				dscene->mModule->mSdk->releaseObjectDesc(shape);
#if NX_SDK_VERSION_MAJOR == 2
				NxActor& actor = (NxActor&)shape->getActor();
				actor.releaseShape(*shape);
				if (dscene->mTotalChunkCount > 0)
				{
					--dscene->mTotalChunkCount;
				}
#elif NX_SDK_VERSION_MAJOR == 3
				physx::PxRigidActor* pxRigidActor = shape->getActor();
				NxActor& actor = (NxActor&)*((NxActor*)pxRigidActor);
				pxRigidActor->detachShape(*shape);
				if (dscene->mTotalChunkCount > 0)
				{
					--dscene->mTotalChunkCount;
				}
#endif
				if (actor.getNbShapes() == 0)
				{
					dscene->releasePhysXActor(actor);
				}
			}
		}
		chunk.clearShapes();
#if USE_CHUNK_RWLOCK
		if (chunk.lock)
		{
			chunk.lock->~ReadWriteLock();
			PX_FREE(chunk.lock);
		}
		chunk.lock = NULL;
#endif
	}

	actorToIsland.clear();
	islandToActor.clear();

	dscene = NULL;
	ID = (physx::PxU32)InvalidID;
}

bool DestructibleStructure::addActors(const physx::Array<DestructibleActor*>& destructiblesToAdd)
{
	PX_PROFILER_PERF_SCOPE("DestructibleAddActors");

	// use the scene lock to protect chunk data for
	// multi-threaded obbSweep calls on destructible actors
	// (different lock would be good, but mixing locks can cause deadlocks)
	SCOPED_PHYSX_LOCK_WRITE(*dscene->mApexScene);

	const physx::PxU32 oldDestructibleCount = destructibles.size();

	const physx::PxU32 destructibleCount = oldDestructibleCount + destructiblesToAdd.size();
	destructibles.resize(destructibleCount);
	physx::PxU32 totalAddedChunkCount = 0;
	for (physx::PxU32 destructibleIndex = oldDestructibleCount; destructibleIndex < destructibleCount; ++destructibleIndex)
	{
		DestructibleActor* destructible = destructiblesToAdd[destructibleIndex - oldDestructibleCount];
		destructibles[destructibleIndex] = destructible;
		totalAddedChunkCount += destructible->getAsset()->getChunkCount();
	}

	const physx::PxU32 oldChunkCount = chunks.size();
	const physx::PxU32 chunkCount = oldChunkCount + totalAddedChunkCount;
	chunks.resize(chunkCount);

	NiApexSDK* sdk = dscene->mModule->mSdk;

	// Fix chunk pointers in shapes that may have been moved because of the resize() operation
	for (physx::PxU32 chunkIndex = 0; chunkIndex < oldChunkCount; ++chunkIndex)
	{
		Chunk& chunk = chunks[chunkIndex];
		if ((chunk.state & ChunkVisible) != 0)
		{
			physx::Array<NxShape*>& shapeArray = getChunkShapes(chunk);
			for (physx::PxU32 i = shapeArray.size(); i--;)
			{
				NxShape* shape = shapeArray[i];
				if (shape != NULL)
				{
					NiApexPhysXObjectDesc* objDesc = sdk->getGenericPhysXObjectInfo(shape);
					if (objDesc != NULL)
					{
						objDesc->userData = &chunk;
					}
				}
			}
		}
	}

	physx::PxU32 chunkIndex = oldChunkCount;
	for (physx::PxU32 destructibleIndex = oldDestructibleCount; destructibleIndex < destructibleCount; ++destructibleIndex)
	{
		DestructibleActor* destructible = destructiblesToAdd[destructibleIndex - oldDestructibleCount];

		const bool formsExtendedStructures = !destructible->isInitiallyDynamic() && destructible->formExtendedStructures();

		if (!formsExtendedStructures)
		{
			PX_ASSERT(destructibleCount == 1);	// All dynamic destructibles must be in their own structure
		}

		destructible->setStructure(this);
		PX_VERIFY(dscene->mDestructibles.useNextFree(destructible->getIDRef()));
		dscene->mDestructibles.direct(destructible->getID()) = destructible;

		//physx::PxVec3 destructibleScale = destructible->getScale();

		destructible->setFirstChunkIndex(chunkIndex);
		for (physx::PxU32 i = 0; i < destructible->getAsset()->getChunkCount(); ++i)
		{
			destructible->initializeChunk(i, chunks[chunkIndex++]);
		}
		// This should be done after all chunks are initialized, since it will traverse all chunks
		Chunk& root = chunks[destructible->getFirstChunkIndex()];
		PX_ASSERT(root.isDestroyed());    // ensure shape pointer is still NULL

		// Create a new static actor if this is the first actor
		if (destructibleIndex == 0)
		{
			// Disable visibility before creation
			root.state &= ~(physx::PxU8)ChunkVisible;
			NxActor* rootActor = dscene->createRoot(root, destructible->getInitialGlobalPose(), destructible->isInitiallyDynamic());
			if (!destructible->isInitiallyDynamic() || destructible->initializedFromState())
			{
				actorForStaticChunks = rootActor;
			}
		}
        // Otherwise, append the new destructible actor's root chunk shapes to the existing root chunk's actor
        //     If the new destructible is initialized from state, defer chunk loading to ChunkLoadState
		else if (!destructible->initializedFromState())
		{
			DestructibleActor* firstDestructible = dscene->mDestructibles.direct(chunks[0].destructibleID);
			physx::PxMat34Legacy relTM;
			relTM.multiplyInverseRTLeft(firstDestructible->getInitialGlobalPose(), destructible->getInitialGlobalPose());
			if (actorForStaticChunks)
			{
				dscene->appendShapes(root, destructible->isInitiallyDynamic(), &relTM, actorForStaticChunks);
				NiApexPhysXObjectDesc* objDesc = sdk->getGenericPhysXObjectInfo(actorForStaticChunks);
				PX_ASSERT(objDesc->mApexActors.find(destructible->getAPI()) == objDesc->mApexActors.end());
				objDesc->mApexActors.pushBack(destructible->getAPI());
				destructible->referencedByActor(actorForStaticChunks);
			}
			else
			{
				// Disable visibility before creation
				root.state &= ~(physx::PxU8)ChunkVisible;
				NxActor* rootActor = dscene->createRoot(root, destructible->getInitialGlobalPose(), destructible->isInitiallyDynamic());
				actorForStaticChunks = rootActor;
			}
		}

		destructible->setRelativeTMs();
		if (destructible->initializedFromState())
		{
			DestructibleScene::ChunkLoadState chunkLoadState(*dscene);
			dscene->forSubtree(root, chunkLoadState, false);
		}

		destructible->setChunkVisibility(0, destructible->getInitialChunkVisible(root.indexInAsset));
		// init bounds
		destructible->setRenderTMs();
	}

	dscene->setStructureSupportRebuild(this, true);

	return true;
}

struct Ptr_LT
{
	PX_INLINE bool operator()(void* a, void* b) const
	{
		return a < b;
	}
};

bool DestructibleStructure::removeActor(DestructibleActor* destructibleToRemove)
{
	PX_PROFILER_PERF_SCOPE("DestructibleRemoveActor");

	// use the scene lock to protect chunk data for
	// multi-threaded obbSweep calls on destructible actors
	// (different lock would be good, but mixing locks can cause deadlocks)
	SCOPED_PHYSX_LOCK_WRITE(*dscene->mApexScene);

	if (destructibleToRemove->getStructure() != this)
	{
		return false;	// Destructible not in structure
	}

	const physx::PxU32 oldDestructibleCount = destructibles.size();

	physx::PxU32 destructibleIndex = 0;
	for (; destructibleIndex < oldDestructibleCount; ++destructibleIndex)
	{
		if (destructibles[destructibleIndex] == destructibleToRemove)
		{
			break;
		}
	}

	if (destructibleIndex == oldDestructibleCount)
	{
		return false;	// Destructible not in structure
	}

	// Remove chunks (releases shapes) and ensure we dissociate from their actors.
	// Note: if we're not in a structure, releasing shapes will always release the actors, which releases the actorObjDesc, thus dissociating us.
	// But if we're in a structure, some other destructible might have shapes in the actor, and we won't get dissociated.  This is why we must
	// Go through the trouble of ensuring dissociation here.
	physx::Array<void*> dissociatedActors;
	physx::PxU32 visibleChunkCount = destructibleToRemove->getNumVisibleChunks();	// Do it this way in case removeChunk doesn't work for some reason
	while (visibleChunkCount--)
	{
		const physx::PxU16 chunkIndex = destructibleToRemove->getVisibleChunks()[destructibleToRemove->getNumVisibleChunks()-1];
		DestructibleStructure::Chunk& chunk = chunks[chunkIndex + destructibleToRemove->getFirstChunkIndex()];
		NxActor* actor = getChunkActor(chunk);
		if (actor != NULL)
		{
			dissociatedActors.pushBack(actor);
		}
		removeChunk(chunk);
	}

	// Now sort the actor pointers so we only deal with each once
	const physx::PxU32 dissociatedActorCount = dissociatedActors.size();
	if (dissociatedActorCount > 1)
	{
		physx::sort<void*, Ptr_LT>(&dissociatedActors[0], dissociatedActors.size(), Ptr_LT());
	}

	// Run through the sorted list
	void* lastActor = NULL;
	for (physx::PxU32 actorNum = 0; actorNum < dissociatedActorCount; ++actorNum)
	{
		void* nextActor = dissociatedActors[actorNum];
		if (nextActor != lastActor)
		{
			NiApexPhysXObjectDesc* actorObjDesc = dscene->mModule->mSdk->getGenericPhysXObjectInfo(nextActor);
			if (actorObjDesc != NULL)
			{
				for (physx::PxU32 i = actorObjDesc->mApexActors.size(); i--;)
				{
					if (actorObjDesc->mApexActors[i] == destructibleToRemove->getAPI())
					{
						actorObjDesc->mApexActors.replaceWithLast(i);
						destructibleToRemove->unreferencedByActor((NxActor*)nextActor);
					}
				}
				if (actorObjDesc->mApexActors.size() == 0)
				{
					dscene->releasePhysXActor(*static_cast<NxActor*>(nextActor));
				}
			}
			lastActor = nextActor;
		}
	}

	// Also check the actor for static chunks
	if (actorForStaticChunks != NULL)
	{
		NiApexPhysXObjectDesc* actorObjDesc = dscene->mModule->mSdk->getGenericPhysXObjectInfo(actorForStaticChunks);
		if (actorObjDesc != NULL)
		{
			for (physx::PxU32 i = actorObjDesc->mApexActors.size(); i--;)
			{
				if (actorObjDesc->mApexActors[i] == destructibleToRemove->getAPI())
				{
					actorObjDesc->mApexActors.replaceWithLast(i);
					destructibleToRemove->unreferencedByActor(actorForStaticChunks);
				}
			}
			if (actorObjDesc->mApexActors.size() == 0)
			{
				dscene->releasePhysXActor(*static_cast<NxActor*>(actorForStaticChunks));
			}
		}
	}

	const physx::PxU32 destructibleToRemoveFirstChunkIndex = destructibleToRemove->getFirstChunkIndex();
	const physx::PxU32 destructibleToRemoveChunkCount = destructibleToRemove->getAsset()->getChunkCount();
	const physx::PxU32 destructibleToRemoveChunkIndexStop = destructibleToRemoveFirstChunkIndex + destructibleToRemoveChunkCount;

	destructibleToRemove->setStructure(NULL);

	// Compact destructibles array
	for (; destructibleIndex < oldDestructibleCount-1; ++destructibleIndex)
	{
		destructibles[destructibleIndex] = destructibles[destructibleIndex+1];
		const physx::PxU32 firstChunkIndex = destructibles[destructibleIndex]->getFirstChunkIndex();
		PX_ASSERT(firstChunkIndex >= destructibleToRemoveChunkCount);
		destructibles[destructibleIndex]->setFirstChunkIndex(firstChunkIndex - destructibleToRemoveChunkCount);
	}
	destructibles.resize(oldDestructibleCount-1);

	// Compact chunks array
	chunks.removeRange(destructibleToRemoveFirstChunkIndex, destructibleToRemoveChunkCount);
	const physx::PxU32 chunkCount = chunks.size();

	NiApexSDK* sdk = dscene->mModule->mSdk;

	// Fix chunk pointers in shapes that may have been moved because of the removeRange() operation
	for (physx::PxU32 chunkIndex = destructibleToRemoveFirstChunkIndex; chunkIndex < chunkCount; ++chunkIndex)
	{
		Chunk& chunk = chunks[chunkIndex];
		if (chunk.visibleAncestorIndex != (physx::PxI32)InvalidChunkIndex)
		{
			chunk.visibleAncestorIndex -= destructibleToRemoveChunkCount;
		}
		for (physx::PxU32 i = chunk.shapes.size(); i--;)
		{
			NxShape* shape = chunk.shapes[i];
			if (shape != NULL)
			{
				NiApexPhysXObjectDesc* objDesc = sdk->getGenericPhysXObjectInfo(shape);
				if (objDesc != NULL)
				{
					objDesc->userData = &chunk;
				}
			}
		}
	}

	// Remove obsolete support info
	physx::PxU32 newSupportChunkNum = 0;
	PX_ASSERT(supportDepthChunksNotExternallySupportedCount <= supportDepthChunks.size());
	for (physx::PxU32 supportChunkNum = 0; supportChunkNum < supportDepthChunksNotExternallySupportedCount; ++supportChunkNum)
	{
		const physx::PxU32 supportChunkIndex = supportDepthChunks[supportChunkNum];
		if (supportChunkIndex < destructibleToRemoveFirstChunkIndex)
		{
			// This chunk index will not be affected
			supportDepthChunks[newSupportChunkNum++] = supportChunkIndex;
		}
		else
		if (supportChunkIndex >= destructibleToRemoveChunkIndexStop)
		{
			// This chunk index will be affected
			supportDepthChunks[newSupportChunkNum++] = supportChunkIndex - destructibleToRemoveChunkCount;
		}
	}

	const physx::PxU32 newSupportDepthChunksNotExternallySupportedCount = newSupportChunkNum;

	for (physx::PxU32 supportChunkNum = supportDepthChunksNotExternallySupportedCount; supportChunkNum < supportDepthChunks.size(); ++supportChunkNum)
	{
		const physx::PxU32 supportChunkIndex = supportDepthChunks[supportChunkNum];
		if (supportChunkIndex < destructibleToRemoveFirstChunkIndex)
		{
			// This chunk index will not be affected
			supportDepthChunks[newSupportChunkNum++] = supportChunkIndex;
		}
		else
		if (supportChunkIndex >= destructibleToRemoveChunkIndexStop)
		{
			// This chunk index will be affected
			supportDepthChunks[newSupportChunkNum++] = supportChunkIndex - destructibleToRemoveChunkCount;
		}
	}

	supportDepthChunksNotExternallySupportedCount = newSupportDepthChunksNotExternallySupportedCount;
	supportDepthChunks.resize(newSupportChunkNum);

	dscene->setStructureSupportRebuild(this, true);

	return true;
}


void DestructibleStructure::updateIslands()
{
	PX_PROFILER_PERF_SCOPE("DestructibleStructureUpdateIslands");

	// islandToActor is used within a single frame to reconstruct deserialized islands.
	//     After that frame, there are no guarantees on island recreation within
	//     a single structure between multiple actors.  Thus we reset it each frame.
	if (islandToActor.size())
	{
		islandToActor.clear();
	}

	if (supportInvalid)
	{
		separateUnsupportedIslands();
	}
}


void DestructibleStructure::tickStressSolver(physx::PxF32 deltaTime)
{
	PX_PROFILER_PERF_SCOPE("DestructibleStructureTickStressSolver");

	PX_ASSERT(stressSolver != NULL);
	if (stressSolver == NULL)
		return;

#if NX_SDK_VERSION_MAJOR == 3
	if(stressSolver->isPhysxBasedSim)
	{
		if(stressSolver->isCustomEnablesSimulating)
		{
			stressSolver->physcsBasedStressSolverTick();
		}
	}
	else
#endif
	{
		stressSolver->onTick(deltaTime);
		if (supportInvalid)
		{
			stressSolver->onResolve();
		}
	}
}

PX_INLINE physx::PxU32 colorFromUInt(physx::PxU32 x)
{
	x = (x % 26) + 1;
	physx::PxU32 rIndex = x / 9;
	x -= rIndex * 9;
	physx::PxU32 gIndex = x / 3;
	physx::PxU32 bIndex = x - gIndex * 3;
	physx::PxU32 r = (rIndex << 7) - (physx::PxU32)(rIndex != 0);
	physx::PxU32 g = (gIndex << 7) - (physx::PxU32)(gIndex != 0);
	physx::PxU32 b = (bIndex << 7) - (physx::PxU32)(bIndex != 0);
	return 0xFF000000 | r << 16 | g << 8 | b;
}

void DestructibleStructure::visualizeSupport(NiApexRenderDebug* debugRender)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(debugRender);
#else

	// apparently if this method is called before an actor is ever stepped bad things can happen
	if (firstOverlapIndices.size() == 0)
	{
		return;
	}

	for (physx::PxU32 i = 0; i < supportDepthChunks.size(); ++i)
	{
		physx::PxU32 chunkIndex = supportDepthChunks[i];
		Chunk& chunk = chunks[chunkIndex];
		NxActor* chunkActor = dscene->chunkIntact(chunk);
		if (chunkActor != NULL)
		{
			NxApexPhysXObjectDesc* actorObjDesc = (NxApexPhysXObjectDesc*) dscene->mModule->mSdk->getPhysXObjectInfo(chunkActor);
			physx::PxU32 color = colorFromUInt((physx::PxU32)(-(intptr_t)actorObjDesc->userData));
			debugRender->setCurrentColor(color);
#if NX_SDK_VERSION_MAJOR == 2
			NxVec3 actorCentroid = chunkActor->getGlobalPose() * chunk.localSphere.center;
#elif NX_SDK_VERSION_MAJOR == 3
			NxVec3 actorCentroid = (chunkActor->getGlobalPose() * PxTransform(chunk.localSphere.center)).p;
#endif
			const physx::PxVec3 chunkCentroid = !chunk.isDestroyed() ? getChunkWorldCentroid(chunk) : PXFROMNXVEC3(actorCentroid);
			const physx::PxU32 firstOverlapIndex = firstOverlapIndices[chunkIndex];
			const physx::PxU32 stopOverlapIndex = firstOverlapIndices[chunkIndex + 1];
			for (physx::PxU32 j = firstOverlapIndex; j < stopOverlapIndex; ++j)
			{
				physx::PxU32 overlapChunkIndex = overlaps[j];
				if (overlapChunkIndex > chunkIndex)
				{
					Chunk& overlapChunk = chunks[overlapChunkIndex];
					NxActor* overlapChunkActor = dscene->chunkIntact(overlapChunk);
					if (overlapChunkActor == chunkActor ||
					        (overlapChunkActor != NULL &&
					         chunkActor->readBodyFlag(NX_BF_KINEMATIC) != 0 &&
					         overlapChunkActor->readBodyFlag(NX_BF_KINEMATIC) != 0))
					{
#if NX_SDK_VERSION_MAJOR == 2
						NxVec3 actorCentroid = overlapChunkActor->getGlobalPose() * overlapChunk.localSphere.center;
#elif NX_SDK_VERSION_MAJOR == 3
						NxVec3 actorCentroid = (overlapChunkActor->getGlobalPose() * PxTransform(overlapChunk.localSphere.center)).p;
#endif
						const physx::PxVec3 overlapChunkCentroid = !overlapChunk.isDestroyed() ? getChunkWorldCentroid(overlapChunk) : PXFROMNXVEC3(actorCentroid);
						// Draw line from chunkCentroid to overlapChunkCentroid
						debugRender->debugLine(chunkCentroid, overlapChunkCentroid);
					}
				}
			}
			if (chunk.flags & ChunkExternallySupported)
			{
				// Draw mark at chunkCentroid
				DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
				if (destructible)
				{
					physx::PxBounds3 bounds = destructible->getAsset()->getChunkShapeLocalBounds(chunk.indexInAsset);
					physx::PxMat34Legacy tm;
					if (chunk.isDestroyed())
					{
						PxFromNxMat34(tm, chunkActor->getGlobalPose());
					}
					else
					{
						tm = getChunkGlobalPose(chunk);
					}
					tm.M.multiplyDiagonal(destructible->getScale());
					PxBounds3Transform(bounds, tm.M, tm.t);
					debugRender->debugBound(bounds.minimum, bounds.maximum);
				}
			}
		}
	}
#endif
}

physx::PxU32 DestructibleStructure::damageChunk(Chunk& chunk, const physx::PxVec3& position, const physx::PxVec3& direction, bool fromImpact, physx::PxF32 damage, physx::PxF32 damageRadius,
												physx::Array<FractureEvent> outputs[], physx::PxU32& possibleDeleteChunks, physx::PxF32& totalDeleteChunkRelativeDamage,
												physx::PxU32& maxDepth, physx::PxU32 depth, physx::PxU16 stopDepth, physx::PxF32 padding)
{
	if (depth > DamageEvent::MaxDepth || depth > (physx::PxU32)stopDepth)
	{
		return 0;
	}

	DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);

	if (depth > destructible->getLOD())
	{
		return 0;
	}

	physx::Array<FractureEvent>& output = outputs[depth];

	const DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];

	if (source.flags & DestructibleAsset::UnfracturableChunk)
	{
		return 0;
	}

	if (chunk.isDestroyed())
	{
		return 0;
	}

	const NxDestructibleParameters& destructibleParameters = destructible->getDestructibleParameters();

	const physx::PxU32 originalEventBufferSize = output.size();

	const bool hasChildren = source.numChildren != 0 && depth < destructible->getLOD();

	// Support legacy behavior
	const bool useLegacyChunkOverlap = destructible->getUseLegacyChunkBoundsTesting();
	const bool useLegacyDamageSpread = destructible->getUseLegacyDamageRadiusSpread();

	// Get behavior group
	const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroupImp(source.behaviorGroupIndex);

	// Get radial info from behavior group
	physx::PxF32 minRadius = 0.0f;
	physx::PxF32 maxRadius = 0.0f;
	physx::PxF32 falloff = 1.0f;
	if (!useLegacyDamageSpread)
	{
		// New behavior
		minRadius = behaviorGroup.damageSpread.minimumRadius;
		falloff = behaviorGroup.damageSpread.falloffExponent;

		if (!fromImpact)
		{
			maxRadius = minRadius + damageRadius*behaviorGroup.damageSpread.radiusMultiplier;
		}
		else
		{
			maxRadius = behaviorGroup.damageToRadius;
			if (behaviorGroup.damageThreshold > 0)
			{
				maxRadius *= damage / behaviorGroup.damageThreshold;
			}
		}
	}
	else
	{
		// Legacy behavior
		if (damageRadius == 0.0f)
		{
			maxRadius = destructible->getLinearSize() * behaviorGroup.damageToRadius;
			if (behaviorGroup.damageThreshold > 0)
			{
				maxRadius *= damage / behaviorGroup.damageThreshold;
			}
		}
		else
		{
			maxRadius = damageRadius;
		}
	}

	physx::PxF32 overlapDistance = PX_MAX_F32;
	if (!useLegacyChunkOverlap)
	{
		destructible->getAsset()->chunkAndSphereInProximity(chunk.indexInAsset, destructible->getChunkPose(chunk.indexInAsset), destructible->getScale(), position, maxRadius, padding, &overlapDistance);
	}
	else
	{
	physx::PxVec3 disp = getChunkWorldCentroid(chunk) - position;
	physx::PxF32 dist = disp.magnitude();
		overlapDistance = dist - maxRadius;
		padding = 0.0f;	// Note we're overriding the function parameter here, as it didn't exist in the old (legacy) code
		if (!hasChildren)	// Test against bounding sphere for smallest chunks
		{
			overlapDistance -= chunk.localSphere.radius;
		}
	}

	physx::PxU32 fractureCount = 0;

	const physx::PxF32 recipRadiusRange = maxRadius > minRadius ? 1.0f/(maxRadius - minRadius) : 0.0f;

	const bool canFracture =
		source.depth >= (physx::PxU16)destructibleParameters.minimumFractureDepth &&	// At or deeper than minimum fracture depth
		(source.flags & DestructibleAsset::DescendantUnfractureable) == 0 &&			// Some descendant cannot be fractured
		(source.flags & DestructibleAsset::UndamageableChunk) == 0 &&					// This chunk is not to be fractured (though a descendant may be)
		(useLegacyChunkOverlap || source.depth >= destructible->getSupportDepth());		// If we're using the new chunk overlap tests, we will only fracture at or deeper than the support depth

	const bool canCrumble = (destructibleParameters.flags & NxDestructibleParametersFlag::CRUMBLE_SMALLEST_CHUNKS) != 0 && (source.flags & DestructibleAsset::UncrumbleableChunk) == 0;

	const bool forceCrumbleOrRTFracture =
		source.depth < (physx::PxU16)destructibleParameters.minimumFractureDepth &&	// minimum fracture depth deeper than this chunk
		!hasChildren &&																// leaf chunk
		(canCrumble || (destructibleParameters.flags & NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE) != 0);	// set to crumble or RT fracture

	if (canFracture || forceCrumbleOrRTFracture)
	{
	physx::PxU32 virtualEventFlag = 0;
	for (physx::PxU32 it = 0; it < 2; ++it)	// Try once as a real event, then possibly as a virtual event
	{
		if (overlapDistance < padding)	// Overlap distance can change
		{
			physx::PxF32 damageFraction = 1;
			if (!useLegacyDamageSpread)
			{
				const physx::PxF32 radius = maxRadius + overlapDistance;
				if (radius > minRadius && recipRadiusRange > 0.0f)
				{
					if (radius < maxRadius)
					{
						damageFraction = physx::PxPow( (maxRadius - radius)*recipRadiusRange, falloff);
					}
					else
					{
						damageFraction = 0.0f;
					}
				}
			}
			else
			{
				if (falloff > 0.0f && maxRadius > 0.0f)
				{
					damageFraction -= physx::PxMax((maxRadius + overlapDistance) / maxRadius, 0.0f);
				}
			}
			const physx::PxF32 oldChunkDamage = chunk.damage;
			physx::PxF32 effectiveDamage = damageFraction * damage;
			chunk.damage += effectiveDamage;

			if ((chunk.damage >= behaviorGroup.damageThreshold) || (fromImpact && (behaviorGroup.materialStrength > 0)) || forceCrumbleOrRTFracture)
			{
					chunk.damage = behaviorGroup.damageThreshold;
				// Fracture
#if REDUCE_DAMAGE_TO_CHILD_CHUNKS
				if (it == 0)
				{
					damage -= chunk.damage - oldChunkDamage;
				}
#endif
				// this is an alternative deletion path (instead of using the delete flag on the fracture event)
				// this can allow us to skip some parts of damageChunk() and fractureChunk(), but it involves duplicating some code
				// we are still relying on the delete flag for now
				if(destructible->mInDeleteChunkMode && false)
				{
					removeChunk(chunks[chunk.indexInAsset]);
					physx::PxU16 parentIndex = source.parentIndex;

					Chunk* rootChunk = getRootChunk(chunk);
					const bool dynamic = rootChunk != NULL ? (rootChunk->state & ChunkDynamic) != 0 : false;
					if(!dynamic)
					{
						Chunk* parent = (parentIndex != DestructibleAsset::InvalidChunkIndex) ? &chunks[parentIndex + destructible->getFirstChunkIndex()] : NULL;
						const DestructibleAssetParametersNS::Chunk_Type* parentSource = parent ? (destructible->getAsset()->mParams->chunks.buf + parent->indexInAsset) : NULL;
						const physx::PxU32 startIndex = parentSource ? parentSource->firstChildIndex + destructible->getFirstChunkIndex() : chunk.indexInAsset /* + actor first index */;
						const physx::PxU32 stopIndex = parentSource ? startIndex + parentSource->numChildren : startIndex + 1;
						// Walk children of current parent (siblings)
						for (physx::PxU32 childIndex = startIndex; childIndex < stopIndex; ++childIndex)
						{
							Chunk& child = chunks[childIndex];
							if (child.isDestroyed())
							{
								continue;
							}
							bool belowSupportDepth = (child.flags & ChunkBelowSupportDepth) != 0;
							if (&child != &chunk && !belowSupportDepth)
							{
								// this is a sibling, which we can keep because we're at a supported depth or above
								if ((child.state & ChunkVisible) == 0)
								{
									// Make this chunk shape visible, attach to root bone
									if (dscene->appendShapes(child, dynamic))
									{
										destructible->setChunkVisibility(child.indexInAsset, true);
									}
								}
							}
						}
					}

					while(parentIndex != DestructibleAsset::InvalidChunkIndex)
					{
						Chunk * parent = &chunks[parentIndex + destructible->getFirstChunkIndex()];
						if(parent->state & ChunkVisible)
						{
							removeChunk(*parent);
						}
						PX_ASSERT(parent->indexInAsset == parentIndex + destructible->getFirstChunkIndex());
						parentIndex = destructible->getAsset()->mParams->chunks.buf[parent->indexInAsset].parentIndex;
					}
				}
				else
				{
					FractureEvent& fractureEvent = output.insert();
					fractureEvent.position = position;
					fractureEvent.impulse = physx::PxVec3(0.0f);
					fractureEvent.chunkIndexInAsset = chunk.indexInAsset;
					fractureEvent.destructibleID = destructible->getID();
					fractureEvent.flags = virtualEventFlag;
					fractureEvent.damageFraction = damageFraction;
					const bool normalCrumbleCondition = canCrumble && !hasChildren && damageFraction*damage >= behaviorGroup.damageThreshold;
					if (it == 0 && (normalCrumbleCondition || forceCrumbleOrRTFracture))
					{
						// Crumble
						fractureEvent.flags |= FractureEvent::CrumbleChunk;
					}
						fractureEvent.deletionWeight = 0.0f;
						if (destructibleParameters.debrisDepth >= 0 && source.depth >= (physx::PxU16)destructibleParameters.debrisDepth)
						{
							++possibleDeleteChunks;
//							const physx::PxF32 relativeDamage = behaviorGroup.damageThreshold > 0.0f ? effectiveDamage/behaviorGroup.damageThreshold : 1.0f;
							fractureEvent.deletionWeight = 1.0f;// - physx::PxExp(-relativeDamage);	// Flat probability for now
							totalDeleteChunkRelativeDamage += fractureEvent.deletionWeight;
						}
				}
			}
			if (!(destructibleParameters.flags & NxDestructibleParametersFlag::ACCUMULATE_DAMAGE))
			{
				chunk.damage = 0;
			}
			if (it == 1)
			{
				chunk.damage = oldChunkDamage;
			}
		}

		if (it == 1)
		{
			break;
		}

		fractureCount = output.size() - originalEventBufferSize;
		if (!hasChildren || fractureCount > 0)
		{
			break;
		}

			if (useLegacyChunkOverlap)
			{
				// Try treating this as a childless chunk.  
				overlapDistance -= chunk.localSphere.radius;
			}

			// If we have bounding sphere overlap, mark it as a virtual fracture event
		virtualEventFlag = FractureEvent::Virtual;
	}
	}
	else
	if (useLegacyChunkOverlap)
	{
		// Legacy system relied on this being done in the loop above (in the other branch of the conditional)
		overlapDistance -= chunk.localSphere.radius;
	}

	if (fractureCount > 0)
	{
		maxDepth = physx::PxMax(depth, maxDepth);
	}

	if (hasChildren && overlapDistance < padding && depth < (physx::PxU32)stopDepth)
	{
		// Recurse
		const physx::PxU32 startIndex = source.firstChildIndex + destructible->getFirstChunkIndex();
		const physx::PxU32 stopIndex = startIndex + source.numChildren;
		for (physx::PxU32 childIndex = startIndex; childIndex < stopIndex; ++childIndex)
		{
			fractureCount += damageChunk(chunks[childIndex], position, direction, fromImpact, damage, damageRadius, outputs,
										 possibleDeleteChunks, totalDeleteChunkRelativeDamage, maxDepth, depth + 1, stopDepth, padding);
		}
	}

	return fractureCount;
}


/*
	fractureChunk causes the chunk referenced in fractureEvent to become a dynamic, standalone chunk.

	If the chunk was invisible because it was an ancestor of a visible chunk, then the ancestor will desroyed,
	and appropriate descendants (including the chunk in question) will be made visible.

	Any chunks which become visible that are at, or deeper than, the support depth will also become standalone
	dynamic chunks.
*/

void DestructibleStructure::fractureChunk(const FractureEvent& fractureEvent)
{
	DestructibleActor* destructible = dscene->mDestructibles.direct(fractureEvent.destructibleID);
	if (!destructible)
	{
		return;
	}

	destructible->wakeForEvent();

	// ===SyncParams===
	if(0 != destructible->getSyncParams().getUserActorID())
	{
	destructible->evaluateForHitChunkList(fractureEvent);
	}

	Chunk& chunk = chunks[fractureEvent.chunkIndexInAsset + destructible->getFirstChunkIndex()];

	const physx::PxVec3& position = fractureEvent.position;
	const physx::PxVec3& impulse = fractureEvent.impulse;
	if(fractureEvent.flags & FractureEvent::CrumbleChunk)
	{
		chunk.flags |= ChunkCrumbled;
	}

	const DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];

	if ((fractureEvent.flags & FractureEvent::Forced) == 0 && (source.flags & DestructibleAsset::UnfracturableChunk) != 0)
	{
		return;
	}

	if (chunk.isDestroyed())
	{
		return;
	}

#if APEX_RUNTIME_FRACTURE
	// The runtime fracture flag pre-empts the crumble flag
	if (source.flags & DestructibleAsset::RuntimeFracturableChunk)
	{
		chunk.flags |= ChunkRuntime;
		chunk.flags &= ~(physx::PxU8)ChunkCrumbled;
	}
#endif

	bool loneDynamicShape = false;
	if ((chunk.state & ChunkVisible) != 0 && chunkIsSolitary(chunk) && (chunk.state & ChunkDynamic) != 0)
	{
		loneDynamicShape = true;
	}

	physx::PxU16 depth = source.depth;

	NxActor* actor = getChunkActor(chunk);
	PX_ASSERT(actor != NULL);

	bool isKinematic = actor->readBodyFlag(NX_BF_KINEMATIC);

	if (loneDynamicShape)
	{
		if(0 == (fractureEvent.flags & FractureEvent::Manual)) // if it is a manual fractureEvent, we do not consider giving it an impulse nor to crumble it
		{
			if (isKinematic)
			{
				NiApexPhysXObjectDesc* actorObjDesc = (NiApexPhysXObjectDesc*)dscene->mModule->mSdk->getPhysXObjectInfo(actor);
				if (actorObjDesc != NULL)
				{
					uintptr_t& cindex = (uintptr_t&)actorObjDesc->userData;
					if (cindex != 0)
					{
						// Dormant actor - turn dynamic
						const physx::PxU32 dormantActorIndex = (physx::PxU32)~cindex; 
						DormantActorEntry& dormantActorEntry = dscene->mDormantActors.direct(dormantActorIndex);
						dormantActorEntry.actor = NULL;
						dscene->mDormantActors.free(dormantActorIndex);
						cindex = (uintptr_t)0;
						actor->clearBodyFlag(NX_BF_KINEMATIC);
						dscene->addActor(*actorObjDesc, *actor, dormantActorEntry.unscaledMass,
							((dormantActorEntry.flags & ActorFIFOEntry::IsDebris) != 0));
						actor->wakeUp();
						isKinematic = false;
					}
				}
			}

			if (!isKinematic && impulse.magnitudeSquared() > 0.0f && ((chunk.flags & ChunkCrumbled) != 0 || (fractureEvent.flags & FractureEvent::DamageFromImpact) == 0))
			{
				addChunkImpluseForceAtPos(chunk, impulse, position);
			}
			if ((chunk.flags & ChunkCrumbled))
			{
				crumbleChunk(fractureEvent, chunk);
			}
#if APEX_RUNTIME_FRACTURE
			else if ((chunk.flags & ChunkRuntime))
			{
				runtimeFractureChunk(fractureEvent, chunk);
			}
#endif
			return;
		}
	}

	Chunk* rootChunk = getRootChunk(chunk);
	const bool dynamic = rootChunk != NULL ? (rootChunk->state & ChunkDynamic) != 0 : false;

	physx::PxMat34Legacy rootTM;
	physx::PxU16 parentIndex = source.parentIndex;
	physx::PxU32 chunkIndex = chunk.indexInAsset + destructible->getFirstChunkIndex();
#if NX_SDK_VERSION_MAJOR == 3
	//Sync the shadow scene if using physics based stress solver
	if(stressSolver&&stressSolver->isPhysxBasedSim)
	{
		stressSolver->removeChunkFromShadowScene(false,chunkIndex);
	}
#endif
	for (;;)
	{
		Chunk* parent = (parentIndex != DestructibleAsset::InvalidChunkIndex) ? &chunks[parentIndex + destructible->getFirstChunkIndex()] : NULL;
		const DestructibleAssetParametersNS::Chunk_Type* parentSource = parent ? (destructible->getAsset()->mParams->chunks.buf + parent->indexInAsset) : NULL;
		const physx::PxU32 startIndex = parentSource ? parentSource->firstChildIndex + destructible->getFirstChunkIndex() : chunkIndex;
		const physx::PxU32 stopIndex = parentSource ? startIndex + parentSource->numChildren : startIndex + 1;
		// Walk children of current parent (siblings)
		for (physx::PxU32 childIndex = startIndex; childIndex < stopIndex; ++childIndex)
		{
			Chunk& child = chunks[childIndex];
			if (child.isDestroyed())
			{
				continue;
			}

			bool belowSupportDepth = (child.flags & ChunkBelowSupportDepth) != 0;

			if (&child != &chunk && !belowSupportDepth)
			{
				// this is a sibling, which we can keep because we're at a supported depth or above
				if ((child.state & ChunkVisible) == 0)
				{
					// Make this chunk shape visible, attach to root bone
					dscene->appendShapes(child, dynamic);
					destructible->setChunkVisibility(child.indexInAsset, true);
				}
			}
			else if (child.flags & ChunkCrumbled)
			{
				// This chunk needs to be crumbled
				if (isKinematic)
				{
					crumbleChunk(fractureEvent, child, &impulse);
				}
				else
				{
					if (impulse.magnitudeSquared() > 0.0f)
					{
						addChunkImpluseForceAtPos(child, impulse, position);
					}
					crumbleChunk(fractureEvent, child);
				}
				if (parent)
				{
					parent->flags |= ChunkMissingChild;
				}
				setSupportInvalid(true);
			}
#if APEX_RUNTIME_FRACTURE
			else if (child.flags & ChunkRuntime)
			{
				runtimeFractureChunk(fractureEvent, child);
				if (parent)
				{
					parent->flags |= ChunkMissingChild;
				}
				setSupportInvalid(true);
			}
#endif
			else
			{
				// This is either the chunk or a sibling, which we are breaking off
				if ((fractureEvent.flags & FractureEvent::Silent) == 0 && dscene->mModule->m_chunkReport != NULL)
				{
					dscene->createChunkReportDataForFractureEvent(fractureEvent, destructible, child);
				}
				destructible->setChunkVisibility(child.indexInAsset, false);	// In case its mesh changes

				// if the fractureEvent is manually generated, we remove the chunk instead of simulating it
				if(0 != (fractureEvent.flags & FractureEvent::DeleteChunk))
				{
					removeChunk(child);
				}
				else
				{
					PX_ASSERT(0 == (fractureEvent.flags & FractureEvent::DeleteChunk));
					NxActor* oldActor = getChunkActor(child);
					NxActor* newActor = dscene->createRoot(child, getChunkGlobalPose(child), true);
					if (newActor != NULL && oldActor == actor)
					{
#if NX_SDK_VERSION_MAJOR == 2
						newActor->setLinearVelocity(actor->getPointVelocity(newActor->getCMassGlobalPosition()));
#elif NX_SDK_VERSION_MAJOR == 3
						physx::PxRigidDynamic* newRigidDynamic = newActor->isRigidDynamic();
						physx::PxVec3 childCenterOfMass = newRigidDynamic->getGlobalPose().transform( newRigidDynamic->getCMassLocalPose().p );
						newActor->setLinearVelocity(actor->getPointVelocity(childCenterOfMass));
#endif
						newActor->setAngularVelocity(actor->getAngularVelocity());
					}
				}

				PX_ASSERT(child.isDestroyed() == ((child.state & ChunkVisible) == 0));
				if (!child.isDestroyed())
				{
					destructible->setChunkVisibility(child.indexInAsset, true);
					if (&child == &chunk)
					{
						if (impulse.magnitudeSquared() > 0.0f)
						{
							// Bring position nearer to chunk for better effect
							PxVec3 newPosition = position;
							const PxVec3 c = getChunkWorldCentroid(child);
							const PxVec3 disp = newPosition - c;
							const PxF32 d2 = disp.magnitudeSquared();
							if (d2 > child.localSphere.radius * child.localSphere.radius)
							{
								newPosition = c + disp * (disp.magnitudeSquared() * PxRecipSqrt(d2));
							}
							addChunkImpluseForceAtPos(child, impulse, newPosition);
						}
					}
					addDust(child);
				}
				if (parent)
				{
					parent->flags |= ChunkMissingChild;
				}
				setSupportInvalid(true);
			}
		}
		if (!parent)
		{
			break;
		}
		if (parent->state & ChunkVisible)
		{
			removeChunk(*parent);
		}
		else if (parent->isDestroyed())
		{
			break;
		}
		parent->clearShapes();
		chunkIndex = parentIndex;
		parentIndex = parentSource->parentIndex;
		PX_ASSERT(depth > 0);
		if (depth > 0)
		{
			--depth;
		}
	}

	dscene->capDynamicActorCount();
}

void DestructibleStructure::crumbleChunk(const FractureEvent& fractureEvent, Chunk& chunk, const physx::PxVec3* impulse)
{
	if (chunk.isDestroyed())
	{
		return;
	}

	DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);

#if APEX_RUNTIME_FRACTURE
	if ((destructible->getDestructibleParameters().flags & NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE) != 0)
	{
		runtimeFractureChunk(fractureEvent, chunk);
		return;
	}
#else
	PX_UNUSED(fractureEvent);
#endif

	dscene->getChunkReportData(chunk, NxApexChunkFlag::DESTROYED_CRUMBLED);

	PX_ASSERT(chunk.reportID < dscene->mChunkReportHandles.size() || chunk.reportID == DestructibleScene::InvalidReportID);
	if (chunk.reportID < dscene->mChunkReportHandles.size())
	{
		IntPair& handle = dscene->mChunkReportHandles[chunk.reportID];
		if ( (physx::PxU32)handle.i0 < dscene->mDamageEventReportData.size() )
		{
			ApexDamageEventReportData& DamageEventReport = dscene->mDamageEventReportData[(physx::PxU32)handle.i0];

			DamageEventReport.impactDamageActor = fractureEvent.impactDamageActor;
			DamageEventReport.appliedDamageUserData = fractureEvent.appliedDamageUserData;
			DamageEventReport.hitPosition = fractureEvent.position;
			DamageEventReport.hitDirection = fractureEvent.hitDirection;
		}
	}

	NxApexEmitterActor* emitterToUse = destructible->getCrumbleEmitter() && destructible->isCrumbleEmitterEnabled() ? destructible->getCrumbleEmitter() : NULL;

	const physx::PxF32 crumbleParticleSpacing = destructible->getCrumbleParticleSpacing();

	if (crumbleParticleSpacing > 0.0f && (emitterToUse != NULL || dscene->mModule->m_chunkCrumbleReport != NULL))
	{
		physx::Array<physx::PxVec3> volumeFillPos;
		/// \todo expose jitter
		const physx::PxF32 jitter = 0.25f;
		for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
		{
			destructible->getAsset()->chunkConvexHulls[hullIndex].fill(volumeFillPos, getChunkGlobalPose(chunk), destructible->getScale(), destructible->getCrumbleParticleSpacing(), jitter, 250, true);
		}
		destructible->spawnParticles(emitterToUse, dscene->mModule->m_chunkCrumbleReport, chunk, volumeFillPos, true, impulse);
	}

	if (chunk.state & ChunkVisible)
	{
		removeChunk(chunk);
	}
}

#if APEX_RUNTIME_FRACTURE
void DestructibleStructure::runtimeFractureChunk(const FractureEvent& fractureEvent, Chunk& chunk)
{
	if (chunk.isDestroyed())
	{
		return;
	}

	DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
	PX_UNUSED(destructible);

	if (destructible->getRTActor() != NULL)
	{
		destructible->getRTActor()->patternFracture(fractureEvent,true);
	}

	if (chunk.state & ChunkVisible)
	{
		removeChunk(chunk);
	}
}
#endif

void DestructibleStructure::addDust(Chunk& chunk)
{
	PX_UNUSED(chunk);
#if 0	// Not implementing dust in 1.2.0
	if (chunk.isDestroyed())
	{
		return;
	}

	DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);

	NxApexEmitterActor* emitterToUse = destructible->getDustEmitter() && destructible->getDustEmitter() ? destructible->getDustEmitter() : NULL;

	const physx::PxF32 dustParticleSpacing = destructible->getDustParticleSpacing();

	if (dustParticleSpacing > 0.0f && (emitterToUse != NULL || dscene->mModule->m_chunkDustReport != NULL))
	{
		physx::Array<physx::PxVec3> tracePos;
		/// \todo expose jitter
		const physx::PxF32 jitter = 0.25f;
		destructible->getAsset()->traceSurfaceBoundary(tracePos, chunk.indexInAsset, getChunkGlobalPose(chunk), destructible->getScale(), dustParticleSpacing, jitter, 0.5f * dustParticleSpacing, 250);
		destructible->spawnParticles(emitterToUse, dscene->mModule->m_chunkDustReport, chunk, tracePos);
	}
#endif
}

void DestructibleStructure::removeChunk(Chunk& chunk)
{
	dscene->scheduleChunkShapesForDelete(chunk);
	DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
#if NX_SDK_VERSION_MAJOR == 3
	//Sync the shadow scene if using physics based stress solver
	if(stressSolver&&stressSolver->isPhysxBasedSim)
	{
		physx::PxU32 chunkIndex = chunk.indexInAsset + destructible->getFirstChunkIndex();
		stressSolver->removeChunkFromShadowScene(false,chunkIndex);
	}
#endif
	destructible->setChunkVisibility(chunk.indexInAsset, false);
	const DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];
	if (source.parentIndex != DestructibleAsset::InvalidChunkIndex)
	{
		Chunk& parent = chunks[source.parentIndex + destructible->getFirstChunkIndex()];
		parent.flags |= ChunkMissingChild;
	}
	// Remove from static roots list (this no-ops if the chunk was not in the static roots list)
	destructible->getStaticRoots().free(chunk.indexInAsset);
	// No more static chunks, make sure this actor field is cleared
	if (actorForStaticChunks != NULL && actorForStaticChunks->getNbShapes() == 0)
	{
		actorForStaticChunks = NULL;
	}
}

struct ChunkIsland
{
	ChunkIsland() : actor(NULL), flags(0), actorIslandDataIndex(0) {}

	physx::Array<physx::PxU32>	indices;
	NxActor*					actor;
	physx::PxU32				flags;
	physx::PxU32				actorIslandDataIndex;
};

struct ActorIslandData
{
	ActorIslandData() : islandCount(0), flags(0) {}

	enum Flag
	{
		SubmmittedForMassRecalc = 0x1
	};

	physx::PxU32	islandCount;
	physx::PxU32	flags;
};

void DestructibleStructure::separateUnsupportedIslands()
{
	// Find instances where an island broke away from an existing
	// island, but instead of two new islands there should actually
	// have been three (or more) because of discontinuities.  Fix it.

	PX_PROFILER_PERF_SCOPE("DestructibleSeparateUnsupportedIslands");
	physx::Array<physx::PxU32> chunksRemaining = supportDepthChunks;
	physx::Array<ChunkIsland> islands;

	// Instead of sorting to find islands that share actors, will merely count.
	// Since we expect the number of new islands to be small, we should be able
	// to eliminate most of the work this way.
	physx::Array<ActorIslandData> islandData(dscene->mActorFIFO.size() + dscene->mDormantActors.usedCount() + 1);	// Max size needed

	while (chunksRemaining.size())
	{
		physx::PxU32 chunkIndex = chunksRemaining.back();
		chunksRemaining.popBack();

		Chunk& chunk = chunks[chunkIndex];

		if ((chunk.state & ChunkTemp0) != 0)
		{
			continue;
		}
		NxActor* actor = dscene->chunkIntact(chunk);
		if (actor == NULL)
		{
			// we conveniently update and find recently broken links here since this loop is executed here
			if(NULL != stressSolver)
			{
#if NX_SDK_VERSION_MAJOR == 3
				if(!stressSolver->isPhysxBasedSim)
#endif
				{
					stressSolver->onUpdate(chunkIndex);
				}
			}
			
			continue;
		}
		ChunkIsland& island = islands.insert();
		island.indices.pushBack(chunkIndex);
		island.actor = actor;
		const bool actorStatic = (chunk.state & ChunkDynamic) == 0;
		NxApexPhysXObjectDesc* actorObjDesc = (NxApexPhysXObjectDesc*) dscene->mModule->mSdk->getPhysXObjectInfo(island.actor);
		physx::PxI32 index = -(physx::PxI32)(intptr_t)actorObjDesc->userData;
		if (index > 0 && actor->readBodyFlag(NX_BF_KINEMATIC))
		{
			index = physx::PxI32(dscene->mDormantActors.getRank((physx::PxU32)index-1) + dscene->mActorFIFO.size());	// Get to the dormant portion of the array
		}
		PX_ASSERT(index >= 0 && index < (physx::PxI32)islandData.size());

		island.actorIslandDataIndex = (physx::PxU32)index;

		++islandData[(physx::PxU32)index].islandCount;
		chunk.state |= ChunkTemp0;
		for (physx::PxU32 i = 0; i < island.indices.size(); ++i)
		{
			const physx::PxU32 chunkIndex = island.indices[i];
			Chunk& chunk = chunks[chunkIndex];
			island.flags |= chunk.flags & ChunkExternallySupported;
			DestructibleActor* dactor = dscene->mDestructibles.direct(chunk.destructibleID);
			const physx::PxU32 visibleAssetIndex = chunk.visibleAncestorIndex >= 0 ? (physx::PxU32)chunk.visibleAncestorIndex - dactor->getFirstChunkIndex() : chunk.indexInAsset;
			DestructibleAssetParametersNS::Chunk_Type& source = dactor->getAsset()->mParams->chunks.buf[visibleAssetIndex];
			if (source.flags & (DestructibleAsset::UnfracturableChunk | DestructibleAsset::DescendantUnfractureable))
			{
				island.flags |= ChunkExternallySupported;
			}
			const physx::PxU32 firstOverlapIndex = firstOverlapIndices[chunkIndex];
			const physx::PxU32 stopOverlapIndex = firstOverlapIndices[chunkIndex + 1];
			for (physx::PxU32 j = firstOverlapIndex; j < stopOverlapIndex; ++j)
			{
				const physx::PxU32 overlapChunkIndex = overlaps[j];
				Chunk& overlapChunk = chunks[overlapChunkIndex];
				if ((overlapChunk.state & ChunkTemp0) == 0)
				{
					NxActor* overlapActor = dscene->chunkIntact(overlapChunk);
					if (overlapActor != NULL)
					{
						if (overlapActor == actor ||
						        ((island.flags & ChunkExternallySupported) != 0 &&
						         actorStatic && (overlapChunk.state & ChunkDynamic) == 0))
						{
							island.indices.pushBack(overlapChunkIndex);
							overlapChunk.state |= (physx::PxU32)ChunkTemp0;
						}
					}
				}
			}
		}
	}

	physx::Array<NxActor*> dirtyActors;	// will need mass properties recalculated

	physx::Array<DestructibleActor*> destructiblesInIsland;

	for (physx::PxU32 islandNum = 0; islandNum < islands.size(); ++islandNum)
	{
		ChunkIsland& island = islands[islandNum];
		if (island.actor->getNbShapes() == 0)
		{
			// This can happen if this island's chunks have all been destroyed
			continue;
		}
		NiApexPhysXObjectDesc* actorObjDesc = dscene->mModule->mSdk->getGenericPhysXObjectInfo(island.actor);
		ActorIslandData& data = islandData[island.actorIslandDataIndex];
		PX_ASSERT(data.islandCount > 0);
		const bool dynamic = 0 == island.actor->readBodyFlag(NX_BF_KINEMATIC);
		if (!(island.flags & ChunkExternallySupported) || dynamic)
		{
			if (data.islandCount > 1)
			{
				--data.islandCount;
				createDynamicIsland(island.indices);
				actorObjDesc = dscene->mModule->mSdk->getGenericPhysXObjectInfo(island.actor);	// Need to get this again, since the pointer can change from createDynamicIsland()!
				if ((data.flags & ActorIslandData::SubmmittedForMassRecalc) == 0)
				{
					dirtyActors.pushBack(island.actor);
					data.flags |= ActorIslandData::SubmmittedForMassRecalc;
				}
				if (island.actorIslandDataIndex > 0 && island.actorIslandDataIndex <= (physx::PxU32)dscene->mActorFIFO.size())
				{
					dscene->mActorFIFO[island.actorIslandDataIndex-1].age = 0.0f;	// Reset the age of the original actor.  It will have a different set of chunks after this
				}
			}
			else
			{
				if (!dynamic)
				{
					createDynamicIsland(island.indices);
					actorObjDesc = dscene->mModule->mSdk->getGenericPhysXObjectInfo(island.actor);	// Need to get this again, since the pointer can change from createDynamicIsland()!
					// If we ever are able to simply clear the NX_BF_KINEMATIC flag, remember to recalculate the mass properties here.
				}
				else
				{
					// Recalculate list of destructibles on this dynamic NxActor island
					destructiblesInIsland.reset();
					for (physx::PxU32 i = 0; i < actorObjDesc->mApexActors.size(); ++i)
					{
						const NxDestructibleActor* dActor = static_cast<const NxDestructibleActor*>(actorObjDesc->mApexActors[i]);
						((DestructibleActorProxy*)dActor)->impl.unreferencedByActor(island.actor);
					}
					actorObjDesc->mApexActors.reset();
					bool isDebris = true;	// Until proven otherwise
					for (physx::PxU32 i = 0; i < island.indices.size(); ++i)
					{
						const physx::PxU32 chunkIndex = island.indices[i];
						Chunk& chunk = chunks[chunkIndex];
						DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
						const NxDestructibleParameters& parameters = destructible->getDestructibleParameters();
						DestructibleAssetParametersNS::Chunk_Type& assetChunk = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];
						if (parameters.debrisDepth >= 0)
						{
							if (assetChunk.depth < (island.indices.size() == 1 ? parameters.debrisDepth : (parameters.debrisDepth + 1)))	// If there is more than one chunk in an island, insist that all chunks must lie strictly _below_ the debris depth to be considered debris
							{
								isDebris = false;
							}
						}
						else
						{
							isDebris = false;
						}
						if ((destructible->getInternalFlags() & DestructibleActor::IslandMarker) == 0)
						{
							destructiblesInIsland.pushBack(destructible);
							PX_ASSERT(actorObjDesc->mApexActors.find(destructible->getAPI()) == actorObjDesc->mApexActors.end());
							actorObjDesc->mApexActors.pushBack(destructible->getAPI());
							destructible->referencedByActor(island.actor);
							destructible->getInternalFlags() |= DestructibleActor::IslandMarker;
						}
					}
					PX_ASSERT(island.actorIslandDataIndex > 0);
					if (isDebris && island.actorIslandDataIndex > 0 && island.actorIslandDataIndex <= (physx::PxU32)dscene->mActorFIFO.size())
					{
						dscene->mActorFIFO[island.actorIslandDataIndex-1].flags |= ActorFIFOEntry::IsDebris;
					}
					for (physx::PxU32 i = 0; i < actorObjDesc->mApexActors.size(); ++i)
					{
						destructiblesInIsland[i]->getInternalFlags() &= ~(physx::PxU16)DestructibleActor::IslandMarker;
					}
				}
			}
		}

		if (actorObjDesc->userData == 0) // initially static chunk
		{
			// Recalculate list of destructibles on the kinematic NxActor island
			for (physx::PxU32 i = actorObjDesc->mApexActors.size(); i--;)
			{
				DestructibleActorProxy* proxy = const_cast<DestructibleActorProxy*>(static_cast<const DestructibleActorProxy*>(actorObjDesc->mApexActors[i]));
				if (proxy->impl.getStaticRoots().usedCount() == 0)
				{
					proxy->impl.unreferencedByActor(island.actor);
					actorObjDesc->mApexActors.replaceWithLast(i);
				}
			}
		}

		if (!dynamic)
		{
			if (actorObjDesc->mApexActors.size() == 0)
			{
				// if all destructible references have been removed
				if (actorForStaticChunks == island.actor)
				{
					actorForStaticChunks = NULL;
				}
			}
			else
			{
				dirtyActors.pushBack(island.actor);
			}
		}
	}

	for (physx::PxU32 dirtyActorNum = 0; dirtyActorNum < dirtyActors.size(); ++dirtyActorNum)
	{
		NxActor* actor = (NxActor*)dirtyActors[dirtyActorNum];

		const physx::PxU32 nShapes = actor->getNbShapes();
		if (nShapes > 0)
		{
			physx::PxF32 mass = 0.0f;
			for (physx::PxU32 i = 0; i < nShapes; ++i)
			{
				NxShape* shape = getShape(*actor, i);
				DestructibleStructure::Chunk* chunk = dscene->getChunk(shape);
				if (chunk == NULL || !chunk->isFirstShape(shape))	// BRG OPTIMIZE
				{
					continue;
				}
				DestructibleActor* dactor = dscene->mDestructibles.direct(chunk->destructibleID);
				mass += dactor->getChunkMass(chunk->indexInAsset);
			}
			if (mass > 0.0f)
			{
				// Updating mass here, since it won't cause extra work for MOI calculator
#if NX_SDK_VERSION_MAJOR == 2
				actor->updateMassFromShapes(0.0f, dscene->scaleMass(mass));
#elif NX_SDK_VERSION_MAJOR == 3
				actor->updateMassFromShapes(0.0f, dscene->scaleMass(mass), true);	// Ensures that we count shapes that have been flagged as non-simulation shapes
#endif
			}
		}
	}

	for (physx::PxU32 supportChunkNum = 0; supportChunkNum < supportDepthChunks.size(); ++supportChunkNum)
	{
		chunks[supportDepthChunks[supportChunkNum]].state &= ~(physx::PxU32)ChunkTemp0;
	}

	setSupportInvalid(false);
}

#if NX_SDK_VERSION_MAJOR == 2
void DestructibleStructure::createDynamicIsland(const physx::Array<physx::PxU32>& indices)
{
	PX_PROFILER_PERF_SCOPE("DestructibleCreateDynamicIsland");

	if (indices.size() == 0)
	{
		return;
	}

	NxActor* actor = dscene->chunkIntact(chunks[indices[0]]);
	PX_ASSERT(actor != NULL);

	physx::Array<Chunk*> removedChunks;
	for (physx::PxU32 i = 0; i < indices.size(); ++i)
	{
		Chunk& chunk = chunks[indices[i]];
		if (!chunk.isDestroyed())
		{
			Chunk* rootChunk = getRootChunk(chunk);
			if (rootChunk && (rootChunk->state & ChunkTemp1) == 0)
			{
				rootChunk->state |= (physx::PxU32)ChunkTemp1;
				removedChunks.pushBack(rootChunk);
				getChunkActor(*rootChunk)->wakeUp();
			}
		}
		else
		{
			// This won't mark the found chunks with ChunkTemp1, but we should not
			// have any duplicates from this search
			DestructibleScene::CollectVisibleChunks chunkOp(removedChunks);
			dscene->forSubtree(chunk, chunkOp);
		}
	}
	if (removedChunks.size() == 0)
	{
		return;
	}

	// Eliminate chunks that are unfractureable.
	for (physx::PxU32 i = 0; i < removedChunks.size(); ++i)
	{
		Chunk& chunk = *removedChunks[i];
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
		destructible->getStaticRoots().free(chunk.indexInAsset);
		DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];
		if ((source.flags & DestructibleAsset::DescendantUnfractureable) != 0)
		{
			chunk.state &= ~(physx::PxU32)ChunkTemp1;
			removedChunks[i] = NULL;
			for (physx::PxU32 j = 0; j < source.numChildren; ++j)
			{
				const physx::PxU32 childIndex = destructible->getFirstChunkIndex() + source.firstChildIndex + j;
				Chunk& child = chunks[childIndex];
				removedChunks.pushBack(&child);
				destructible->setChunkVisibility(child.indexInAsset, false);	// In case its mesh changes
				dscene->appendShapes(child, true);
				destructible->setChunkVisibility(child.indexInAsset, true);
			}
			removeChunk(chunk);
		}
		if ((source.flags & DestructibleAsset::UnfracturableChunk) != 0)
		{
			if (actor->readBodyFlag(NX_BF_KINEMATIC))
			{
				chunk.state &= ~(physx::PxU32)ChunkTemp1;
				removedChunks[i] = NULL;
			}
		}
	}

	physx::PxU32 convexShapeCount = 0;
	physx::PxU32 chunkCount = 0;
	for (physx::PxU32 i = 0; i < removedChunks.size(); ++i)
	{
		Chunk* chunk = removedChunks[i];
		if (chunk == NULL)
		{
			continue;
		}
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk->destructibleID);
		removedChunks[chunkCount++] = chunk;
		convexShapeCount += destructible->getAsset()->getChunkHullCount(chunk->indexInAsset);
	}
	for (physx::PxU32 i = chunkCount; i < removedChunks.size(); ++i)
	{
		removedChunks[i] = NULL;
	}

	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	physx::Array<DestructibleActor*> destructiblesInIsland;

	physx::PxF32 mass = 0.0f;

	physx::PxU32* convexShapeCounts = (physx::PxU32*)PxAlloca(sizeof(physx::PxU32) * chunkCount);
	NxConvexShapeDesc* convexShapeDescs = (NxConvexShapeDesc*)PxAlloca(sizeof(NxConvexShapeDesc) * convexShapeCount);
	physx::PxU32 createdShapeCount = 0;
	bool isDebris = true;	// until proven otherwise
	physx::PxU32 minDepth = 0xFFFFFFFF;
	bool takesImpactDamage = false;
	PxF32 minContactReportThreshold = PX_MAX_F32;
	for (physx::PxU32 i = 0; i < chunkCount; ++i)
	{
		Chunk* chunk = removedChunks[i];
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk->destructibleID);
		convexShapeCounts[i] = destructible->getAsset()->getChunkHullCount(chunk->indexInAsset);
		if ((destructible->getInternalFlags() & DestructibleActor::IslandMarker) == 0)
		{
			destructiblesInIsland.pushBack(destructible);
			destructible->getInternalFlags() |= DestructibleActor::IslandMarker;
		}
		const NxDestructibleParameters& parameters = destructible->getDestructibleParameters();
		DestructibleAssetParametersNS::Chunk_Type& assetChunk = destructible->getAsset()->mParams->chunks.buf[chunk->indexInAsset];
		if (parameters.debrisDepth >= 0)
		{
			if (assetChunk.depth < (chunkCount == 1 ? parameters.debrisDepth : (parameters.debrisDepth + 1)))	// If there is more than one chunk in an island, insist that all chunks must lie strictly _below_ the debris depth to be considered debris
			{
				isDebris = false;
			}
		}
		else
		{
			isDebris = false;
		}
		if (assetChunk.depth < minDepth)
		{
			minDepth = assetChunk.depth;
		}
		if (destructible->takesImpactDamageAtDepth(assetChunk.depth))
		{
			takesImpactDamage = true;
			minContactReportThreshold = PxMin(minContactReportThreshold,
											  destructible->getContactReportThreshold(*chunk));
		}

		chunk->state &= ~(physx::PxU32)ChunkTemp1;
		mass += destructible->getChunkMass(chunk->indexInAsset);

		for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk->indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk->indexInAsset); ++hullIndex)
		{
			NxConvexShapeDesc& convexShapeDesc = convexShapeDescs[createdShapeCount];
			PX_PLACEMENT_NEW(&convexShapeDesc, NxConvexShapeDesc);

			// Need to get NxConvexMesh
			// Shape(s):
			destructible->getShapeTemplate(convexShapeDesc);
			NxFromPxMat34(convexShapeDesc.localPose, getChunkLocalPose(*chunk));

			convexShapeDesc.meshData = destructible->getConvexMesh(hullIndex);
			// Make sure we can get a collision mesh
			if (!convexShapeDesc.meshData)
			{
				PX_ALWAYS_ASSERT();
				return;
			}

			convexShapeDesc.shapeFlags &= ~(physx::PxU32)NX_SF_DISABLE_COLLISION;
			actorDesc.shapes.pushBack((NxShapeDesc*)&convexShapeDesc);
			++createdShapeCount;
		}

		dscene->scheduleChunkShapesForDelete(*chunk);
	}

	if (destructiblesInIsland.size() == 0)
	{
		return;
	}

	/*
		What if the different destructibles have different properties?  Which to choose for the island?  For now we'll just choose one.
	*/
	DestructibleActor* templateDestructible = destructiblesInIsland[0];

	NxConvexShapeDesc templateShapeDesc;
	templateDestructible->getShapeTemplate(templateShapeDesc);

	templateDestructible->getActorTemplate(actorDesc);
	templateDestructible->getBodyTemplate(bodyDesc);
	actorDesc.density = 0.0f;
	bodyDesc.flags &= ~(physx::PxU32)NX_BF_KINEMATIC;
	bodyDesc.mass = dscene->scaleMass(mass);
	bodyDesc.massSpaceInertia.set(0.0f);
	bodyDesc.massLocalPose.zero();
	actorDesc.body = &bodyDesc;
	actorDesc.globalPose = actor->getGlobalPose();

	if (takesImpactDamage)
	{
		// Set contact report threshold if the actor can take impact damage
#if (NX_SDK_VERSION_NUMBER >= 280)
		bodyDesc.contactReportThreshold = minContactReportThreshold;
		actorDesc.contactReportFlags =
		    NX_NOTIFY_FORCES |
		    NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD |
		    NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD;
#endif
	}

	NxActor* newActor = NULL;

	// Create actor
	PX_ASSERT(actorDesc.isValid());
	newActor = dscene->mPhysXScene->createActor(actorDesc);
	PX_ASSERT(newActor);

	if (templateDestructible->getDestructibleParameters().dynamicChunksDominanceGroup < 32)
	{
		newActor->setDominanceGroup(templateDestructible->getDestructibleParameters().dynamicChunksDominanceGroup);
	}

	// Use the templateDestructible to start the actorDesc, then add the other destructibles in the island
	NiApexPhysXObjectDesc* actorObjDesc = dscene->mModule->mSdk->createObjectDesc(templateDestructible->getAPI(), newActor);
	templateDestructible->referencedByActor(newActor);
	actorObjDesc->userData = 0;
	templateDestructible->setActorObjDescFlags(actorObjDesc, minDepth);
	// While we're at it, clear the marker flags here
	templateDestructible->getInternalFlags() &= ~(physx::PxU16)DestructibleActor::IslandMarker;
	for (physx::PxU32 i = 1; i < destructiblesInIsland.size(); ++i)
	{
		DestructibleActor* destructibleInIsland = destructiblesInIsland[i];
		PX_ASSERT(actorObjDesc->mApexActors.find(destructibleInIsland->getAPI()) == actorObjDesc->mApexActors.end());
		actorObjDesc->mApexActors.pushBack(destructibleInIsland->getAPI());
		destructibleInIsland->referencedByActor(newActor);
		destructibleInIsland->getInternalFlags() &= ~(physx::PxU16)DestructibleActor::IslandMarker;
	}

	dscene->addActor(*actorObjDesc, *newActor, mass, isDebris);
	newActor->wakeUp();
	physx::PxU32 newIslandID = newNxActorIslandReference(*removedChunks[0], *newActor);

	physx::PxU32 shapeStart = 0;
	for (physx::PxU32 i = 0; i < chunkCount; ++i)
	{
		Chunk& chunk = *removedChunks[i];
		chunk.islandID = newIslandID;
		physx::PxU32 shapeCount = convexShapeCounts[i];
		NxShape* const* newShapes = newActor->getShapes() + shapeStart;
		shapeStart += shapeCount;
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
		PX_ASSERT(this == destructible->getStructure());
		destructible->setChunkVisibility(chunk.indexInAsset, false);
		chunk.state |= (physx::PxU32)ChunkVisible | (physx::PxU32)ChunkDynamic;
		destructible->setChunkVisibility(chunk.indexInAsset, true);
		chunk.setShapes(newShapes, shapeCount);
		DestructibleScene::VisibleChunkSetDescendents chunkOp(chunk.indexInAsset + destructible->getFirstChunkIndex(), true);
		dscene->forSubtree(chunk, chunkOp, true);
		for (physx::PxU32 j = 0; j < shapeCount; ++j)
		{
			NiApexPhysXObjectDesc* shapeObjDesc = dscene->mModule->mSdk->createObjectDesc(destructible->getAPI(), newShapes[j]);
			shapeObjDesc->userData = &chunk;
		}
	}

	if (newActor && dscene->getModule()->m_destructiblePhysXActorReport != NULL)
	{
		dscene->getModule()->m_destructiblePhysXActorReport->onPhysXActorCreate(*newActor);
	}
}

#elif NX_SDK_VERSION_MAJOR == 3
void DestructibleStructure::createDynamicIsland(const physx::Array<physx::PxU32>& indices)
{
	PX_PROFILER_PERF_SCOPE("DestructibleCreateDynamicIsland");

	if (indices.size() == 0)
	{
		return;
	}

	NxActor* actor = dscene->chunkIntact(chunks[indices[0]]);
	PX_ASSERT(actor != NULL);

	physx::Array<Chunk*> removedChunks;
	for (physx::PxU32 i = 0; i < indices.size(); ++i)
	{
		Chunk& chunk = chunks[indices[i]];
		if (!chunk.isDestroyed())
		{
			Chunk* rootChunk = getRootChunk(chunk);
			if (rootChunk && (rootChunk->state & ChunkTemp1) == 0)
			{
				rootChunk->state |= (physx::PxU32)ChunkTemp1;
				removedChunks.pushBack(rootChunk);
				if(stressSolver && stressSolver->isPhysxBasedSim)
				{
					stressSolver->removeChunkFromIsland(indices[i]);
				}
				getChunkActor(*rootChunk)->wakeUp();
			}
		}
		else
		{
			// This won't mark the found chunks with ChunkTemp1, but we should not
			// have any duplicates from this search
			DestructibleScene::CollectVisibleChunks chunkOp(removedChunks);
			dscene->forSubtree(chunk, chunkOp);
		}
	}
	if (removedChunks.size() == 0)
	{
		return;
	}

	// Eliminate chunks that are unfractureable.
	for (physx::PxU32 i = 0; i < removedChunks.size(); ++i)
	{
		Chunk& chunk = *removedChunks[i];
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
		destructible->getStaticRoots().free(chunk.indexInAsset);
		DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];
		if ((source.flags & DestructibleAsset::DescendantUnfractureable) != 0)
		{
			chunk.state &= ~(physx::PxU32)ChunkTemp1;
			removedChunks[i] = NULL;
			for (physx::PxU32 j = 0; j < source.numChildren; ++j)
			{
				const physx::PxU32 childIndex = destructible->getFirstChunkIndex() + source.firstChildIndex + j;
				Chunk& child = chunks[childIndex];
				removedChunks.pushBack(&child);
				destructible->setChunkVisibility(child.indexInAsset, false);	// In case its mesh changes
				dscene->appendShapes(child, true);
				destructible->setChunkVisibility(child.indexInAsset, true);
			}
			removeChunk(chunk);
		}
		if ((source.flags & DestructibleAsset::UnfracturableChunk) != 0)
		{
			if (actor->readBodyFlag(NX_BF_KINEMATIC))
			{
				chunk.state &= ~(physx::PxU32)ChunkTemp1;
				removedChunks[i] = NULL;
			}
		}
	}

	physx::PxU32 convexShapeCount = 0;
	physx::PxU32 chunkCount = 0;
	for (physx::PxU32 i = 0; i < removedChunks.size(); ++i)
	{
		Chunk* chunk = removedChunks[i];
		if (chunk == NULL)
		{
			continue;
		}
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk->destructibleID);
		removedChunks[chunkCount++] = chunk;

		convexShapeCount += destructible->getAsset()->getChunkHullCount(chunk->indexInAsset);
	}
	for (physx::PxU32 i = chunkCount; i < removedChunks.size(); ++i)
	{
		removedChunks[i] = NULL;
	}

	PxRigidDynamic* newActor = NULL;

	physx::Array<DestructibleActor*> destructiblesInIsland;

	physx::PxF32 mass = 0.0f;

	physx::PxU32* convexShapeCounts = (physx::PxU32*)PxAlloca(sizeof(physx::PxU32) * chunkCount);
	PxConvexMeshGeometry* convexShapeDescs = (PxConvexMeshGeometry*)PxAlloca(sizeof(PxConvexMeshGeometry) * convexShapeCount);
	physx::PxU32 createdShapeCount = 0;
	bool isDebris = true;	// until proven otherwise
	physx::PxU32 minDepth = 0xFFFFFFFF;
	bool takesImpactDamage = false;
	PxF32 minContactReportThreshold = PX_MAX_F32;
	PhysX3DescTemplate physX3Template;

	for (physx::PxU32 i = 0; i < chunkCount; ++i)
	{
		Chunk* chunk = removedChunks[i];

		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk->destructibleID);
		destructible->getPhysX3Template(physX3Template);
		if ((destructible->getInternalFlags() & DestructibleActor::IslandMarker) == 0)
		{
			destructiblesInIsland.pushBack(destructible);
			destructible->getInternalFlags() |= DestructibleActor::IslandMarker;

			if (!newActor)
			{
				newActor = dscene->mPhysXScene->getPhysics().createRigidDynamic(actor->getGlobalPose());

				PX_ASSERT(newActor);
				physX3Template.apply(newActor);

				if (destructible->getDestructibleParameters().dynamicChunksDominanceGroup < 32)
				{
					newActor->setDominanceGroup(destructible->getDestructibleParameters().dynamicChunksDominanceGroup);
				}

				newActor->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
				const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = destructible->getBehaviorGroup(chunk->indexInAsset);
				newActor->setMaxDepenetrationVelocity(behaviorGroup.maxDepenetrationVelocity);
			}
		}
		NxDestructibleParameters& parameters = destructible->getDestructibleParameters();
		DestructibleAssetParametersNS::Chunk_Type& assetChunk = destructible->getAsset()->mParams->chunks.buf[chunk->indexInAsset];
		if (parameters.debrisDepth >= 0)
		{
			if (assetChunk.depth < (chunkCount == 1 ? parameters.debrisDepth : (parameters.debrisDepth + 1)))	// If there is more than one chunk in an island, insist that all chunks must lie strictly _below_ the debris depth to be considered debris
			{
				isDebris = false;
			}
		}
		else
		{
			isDebris = false;
		}
		if (assetChunk.depth < minDepth)
		{
			minDepth = assetChunk.depth;
		}
		if (destructible->takesImpactDamageAtDepth(assetChunk.depth))
		{
			takesImpactDamage = true;
			minContactReportThreshold = PxMin(minContactReportThreshold,
											  destructible->getContactReportThreshold(*chunk));
		}

		chunk->state &= ~(physx::PxU32)ChunkTemp1;
		mass += destructible->getChunkMass(chunk->indexInAsset);

		const physx::PxU32 oldShapeCount = newActor->getNbShapes();
		for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk->indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk->indexInAsset); ++hullIndex)
		{
			PxConvexMeshGeometry& convexShapeDesc = convexShapeDescs[createdShapeCount];
			PX_PLACEMENT_NEW(&convexShapeDesc, PxConvexMeshGeometry);

			// Need to get NxConvexMesh
			// Shape(s):
			PxTransform	localPose	= getChunkLocalPose(*chunk).toPxTransform();

			convexShapeDesc.convexMesh = destructible->getConvexMesh(hullIndex);
			// Make sure we can get a collision mesh
			if (!convexShapeDesc.convexMesh)
			{
				PX_ALWAYS_ASSERT();
				return;
			}
			convexShapeDesc.scale.scale = destructible->getScale();
			// Divide out the cooking scale to get the proper scaling
			const physx::PxVec3 cookingScale = dscene->getModule()->getChunkCollisionHullCookingScale();
			convexShapeDesc.scale.scale.x /= cookingScale.x;
			convexShapeDesc.scale.scale.y /= cookingScale.y;
			convexShapeDesc.scale.scale.z /= cookingScale.z;
			if ((convexShapeDesc.scale.scale - physx::PxVec3(1.0f)).abs().maxElement() < 10*PX_EPS_F32)
			{
				convexShapeDesc.scale.scale = physx::PxVec3(1.0f);
			}

			PxShape*	shape;

			shape = newActor->createShape(convexShapeDesc, 
				physX3Template.materials.begin(), 
				static_cast<physx::PxU16>(physX3Template.materials.size()),
				static_cast<PxShapeFlags>(physX3Template.shapeFlags));
			shape->setLocalPose(localPose);
			physX3Template.apply(shape);

			PxPairFlags	pairFlag	= (PxPairFlags)physX3Template.contactReportFlags;
			if (takesImpactDamage)
			{
				pairFlag	/* |= PxPairFlag::eNOTIFY_CONTACT_FORCES */
							|=  PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS
							|  PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND
							|  PxPairFlag::eNOTIFY_CONTACT_POINTS;
			}

			if (destructible->getBehaviorGroup(chunk->indexInAsset).materialStrength > 0.0f)
			{
				pairFlag |= PxPairFlag::eMODIFY_CONTACTS;
			}

			if (dscene->mApexScene->getApexPhysX3Interface())
				dscene->mApexScene->getApexPhysX3Interface()->setContactReportFlags(shape, pairFlag, destructible->getAPI(), chunk->indexInAsset);


			++createdShapeCount;
		}
		dscene->scheduleChunkShapesForDelete(*chunk);
		convexShapeCounts[i] = newActor->getNbShapes() - oldShapeCount;
	}

	if (destructiblesInIsland.size() == 0)
	{
		return;
	}

	/*
		What if the different destructibles have different properties?  Which to choose for the island?  For now we'll just choose one.
	*/
	DestructibleActor* templateDestructible = destructiblesInIsland[0];

	newActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false);
	if (takesImpactDamage)
	{
		// Set contact report threshold if the actor can take impact damage
		newActor->setContactReportThreshold(minContactReportThreshold);
	}
	PxRigidBodyExt::setMassAndUpdateInertia(*newActor, dscene->scaleMass(mass));

	{
		dscene->mActorsToAddIndexMap[newActor] = dscene->mActorsToAdd.size();
		dscene->mActorsToAdd.pushBack(newActor);

		PxReal initWakeCounter = dscene->mPhysXScene->getWakeCounterResetValue();
		newActor->setWakeCounter(initWakeCounter);
	}

	// Use the templateDestructible to start the actorDesc, then add the other destructibles in the island
	NiApexPhysXObjectDesc* actorObjDesc = dscene->mModule->mSdk->createObjectDesc(templateDestructible->getAPI(), newActor);
	actorObjDesc->userData = 0;
	templateDestructible->setActorObjDescFlags(actorObjDesc, minDepth);
	templateDestructible->referencedByActor((NxActor*)newActor); // needs to be called after setActorObjDescFlags
	// While we're at it, clear the marker flags here
	templateDestructible->getInternalFlags() &= ~(physx::PxU16)DestructibleActor::IslandMarker;
	for (physx::PxU32 i = 1; i < destructiblesInIsland.size(); ++i)
	{
		DestructibleActor* destructibleInIsland = destructiblesInIsland[i];
		PX_ASSERT(actorObjDesc->mApexActors.find(destructibleInIsland->getAPI()) == actorObjDesc->mApexActors.end());
		actorObjDesc->mApexActors.pushBack(destructibleInIsland->getAPI());
		destructibleInIsland->referencedByActor((NxActor*)newActor);
		destructibleInIsland->getInternalFlags() &= ~(physx::PxU16)DestructibleActor::IslandMarker;
	}

	dscene->addActor(*actorObjDesc, *(NxActor*)newActor, mass, isDebris);

	physx::PxU32 newIslandID = newNxActorIslandReference(*removedChunks[0], *(NxActor*)newActor);
	physx::PxU32 shapeStart = 0;
	const physx::PxU32 actorShapeCount = newActor->getNbShapes();
	PX_ALLOCA(shapeArray, physx::PxShape*, actorShapeCount);
	PxRigidActor* rigidActor = newActor->isRigidActor();
	rigidActor->getShapes(shapeArray, actorShapeCount);
	for (physx::PxU32 i = 0; i < chunkCount; ++i)
	{
		Chunk& chunk = *removedChunks[i];
		chunk.islandID = newIslandID;
		physx::PxU32 shapeCount = convexShapeCounts[i];
		NxShape* const* newShapes = shapeArray + shapeStart;
		shapeStart += shapeCount;
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
		PX_ASSERT(this == destructible->getStructure());
		destructible->setChunkVisibility(chunk.indexInAsset, false);
		chunk.state |= (physx::PxU32)ChunkVisible | (physx::PxU32)ChunkDynamic;
		destructible->setChunkVisibility(chunk.indexInAsset, true);
		chunk.setShapes(newShapes, shapeCount);
		DestructibleScene::VisibleChunkSetDescendents chunkOp(chunk.indexInAsset+destructible->getFirstChunkIndex(), true);
		dscene->forSubtree(chunk, chunkOp, true);
		for (physx::PxU32 j = 0; j < shapeCount; ++j)
		{
			NiApexPhysXObjectDesc* shapeObjDesc = dscene->mModule->mSdk->createObjectDesc(destructible->getAPI(), newShapes[j]);
			shapeObjDesc->userData = &chunk;
		}
	}

	if (newActor && dscene->getModule()->m_destructiblePhysXActorReport != NULL)
	{
		dscene->getModule()->m_destructiblePhysXActorReport->onPhysXActorCreate(*newActor);
	}

	//===SyncParams===
	evaluateForHitChunkList(indices);
}


#endif

void DestructibleStructure::calculateExternalSupportChunks()
{
	const physx::PxU32 chunkCount = chunks.size();
	PX_UNUSED(chunkCount);
	PX_PROFILER_PERF_DSCOPE("DestructibleStructureCalculateExternalSupportChunks", chunkCount);

	supportDepthChunks.resize(0);
	supportDepthChunksNotExternallySupportedCount = 0;

	// iterate all chunks, one destructible after the other
	for (physx::PxU32 i = 0; i < destructibles.size(); ++i)
	{
		DestructibleActor* destructible = destructibles[i];
		if (destructible == NULL)
			continue;

		DestructibleAsset* destructibleAsset = destructible->getAsset();
		const physx::PxF32 paddingFactor = destructibleAsset->mParams->neighborPadding;
		const physx::PxF32 padding = paddingFactor * (destructible->getOriginalBounds().maximum - destructible->getOriginalBounds().minimum).magnitude();
		const DestructibleAssetParametersNS::Chunk_Type* destructibleAssetChunks = destructibleAsset->mParams->chunks.buf;

		const PxU32 supportDepth = destructible->getSupportDepth();
		const bool useAssetDefinedSupport = destructible->useAssetDefinedSupport();
		const bool useWorldSupport = destructible->useWorldSupport();

		const PxU32 firstChunkIndex = destructible->getFirstChunkIndex();
		const PxU32 destructibleChunkCount = destructible->getChunkCount();
		for (PxU32 chunkIndex = firstChunkIndex; chunkIndex < firstChunkIndex + destructibleChunkCount; ++chunkIndex)
		{
			Chunk& chunk = chunks[chunkIndex];
			const DestructibleAssetParametersNS::Chunk_Type& source = destructibleAssetChunks[chunk.indexInAsset];

			// init chunks flags (this is why all chunks need to be iterated)
			chunk.flags &= ~((physx::PxU8)ChunkBelowSupportDepth | (physx::PxU8)ChunkExternallySupported | (physx::PxU8)ChunkWorldSupported);

			if (source.depth > supportDepth)
			{
				chunk.flags |= ChunkBelowSupportDepth;
			}
			else if (source.depth == supportDepth)
			{	
				// keep list of all support depth chunks
				supportDepthChunks.pushBack(chunkIndex);

				if (!destructible->isInitiallyDynamic())
				{
					// Only static destructibles can be externally supported
					if (useAssetDefinedSupport)
					{
						if ((source.flags & DestructibleAsset::SupportChunk) != 0)
						{
							chunk.flags |= ChunkExternallySupported;
						}
					}
					if (useWorldSupport)	// Test even if ChunkExternallySupported flag is already set, since we want to set the ChunkWorldSupported flag correctly
					{
	#if NX_SDK_VERSION_MAJOR == 2
						NxPlaneShapeDesc shapeDesc;
						destructible->getShapeTemplate(shapeDesc);
						for (physx::PxU32 hullIndex = destructibleAsset->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructibleAsset->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
						{
							physx::PxMat44 globalPose = destructible->getInitialGlobalPose();
							globalPose.setPosition(globalPose.getPosition() + globalPose.rotate(destructible->getScale().multiply(destructibleAsset->getChunkPositionOffset(chunk.indexInAsset))));
							if (dscene->testWorldOverlap(destructibleAsset->chunkConvexHulls[hullIndex], globalPose, destructible->getScale(), padding, &shapeDesc.groupsMask))
							{
								chunk.flags |= ChunkExternallySupported | ChunkWorldSupported;
								break;
							}
						}
	#elif NX_SDK_VERSION_MAJOR == 3
						PhysX3DescTemplate physX3Template;
						destructible->getPhysX3Template(physX3Template);
						for (physx::PxU32 hullIndex = destructibleAsset->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructibleAsset->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
						{
							physx::PxMat44 globalPose = destructible->getInitialGlobalPose();
							globalPose.setPosition(globalPose.getPosition() + globalPose.rotate(destructible->getScale().multiply(destructibleAsset->getChunkPositionOffset(chunk.indexInAsset))));
							if (dscene->testWorldOverlap(destructibleAsset->chunkConvexHulls[hullIndex], globalPose, destructible->getScale(), padding, &physX3Template.queryFilterData))
							{
								chunk.flags |= ChunkExternallySupported | ChunkWorldSupported;
								break;
							}
						}
	#endif
					}

					if (	((chunk.flags & ChunkExternallySupported) == 0) &&
						supportDepthChunks.size() > supportDepthChunksNotExternallySupportedCount + 1)
					{
						// Will sort the supportDepthChunks so that externally supported ones are all at the end of the array
						physx::swap(supportDepthChunks.back(), supportDepthChunks[supportDepthChunksNotExternallySupportedCount]);
						++supportDepthChunksNotExternallySupportedCount;
					}
				}
			}
		}
	}
}

void DestructibleStructure::invalidateBounds(const physx::PxBounds3* bounds, physx::PxU32 boundsCount)
{
	for (physx::PxU32 chunkNum = supportDepthChunks.size(); chunkNum-- > supportDepthChunksNotExternallySupportedCount;)
	{
		const physx::PxU32 chunkIndex = supportDepthChunks[chunkNum];
		Chunk& chunk = chunks[chunkIndex];
		if (chunk.state & ChunkDynamic)
		{
			continue;	// Only affect still-static chunks
		}
		DestructibleActor* destructible = dscene->mDestructibles.direct(chunk.destructibleID);
		const physx::PxF32 paddingFactor = destructible->getAsset()->mParams->neighborPadding;
		const physx::PxF32 padding = paddingFactor * (destructible->getOriginalBounds().maximum - destructible->getOriginalBounds().minimum).magnitude();
		const DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk.indexInAsset];
		if (destructible->useAssetDefinedSupport() && (source.flags & DestructibleAsset::SupportChunk) != 0)
		{
			continue;	// Asset-defined support.  This is not affected by external actors.
		}
		physx::PxBounds3 chunkBounds = destructible->getChunkBounds(chunk.indexInAsset);
		chunkBounds.fattenFast(2.0f*padding);
		bool affectedByInvalidBounds = false;
		for (physx::PxU32 boundsNum = 0; boundsNum < boundsCount; ++boundsNum)
		{
			if (bounds[boundsNum].intersects(chunkBounds))
			{
				affectedByInvalidBounds = true;
				break;
			}
		}
		if (!affectedByInvalidBounds)
		{
			continue;
		}
		bool isWorldSupported = false;	// until proven otherwise
#if NX_SDK_VERSION_MAJOR == 2
		NxPlaneShapeDesc shapeDesc;
		destructible->getShapeTemplate(shapeDesc);
		for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
		{
			physx::PxMat44 globalPose = destructible->getInitialGlobalPose();
			globalPose.setPosition(globalPose.getPosition() + globalPose.rotate(destructible->getScale().multiply(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset))));
			if (dscene->testWorldOverlap(destructible->getAsset()->chunkConvexHulls[hullIndex], globalPose, destructible->getScale(), padding, &shapeDesc.groupsMask))
			{
				isWorldSupported = true;
				break;
			}
		}
#elif NX_SDK_VERSION_MAJOR == 3
		PhysX3DescTemplate physX3Template;
		destructible->getPhysX3Template(physX3Template);
		for (physx::PxU32 hullIndex = destructible->getAsset()->getChunkHullIndexStart(chunk.indexInAsset); hullIndex < destructible->getAsset()->getChunkHullIndexStop(chunk.indexInAsset); ++hullIndex)
		{
			physx::PxMat44 globalPose = destructible->getInitialGlobalPose();
			globalPose.setPosition(globalPose.getPosition() + globalPose.rotate(destructible->getScale().multiply(destructible->getAsset()->getChunkPositionOffset(chunk.indexInAsset))));
			if (dscene->testWorldOverlap(destructible->getAsset()->chunkConvexHulls[hullIndex], globalPose, destructible->getScale(), padding, &physX3Template.queryFilterData))
			{
				isWorldSupported = true;
				break;
			}
		}
#endif
		if (!isWorldSupported)
		{
			chunk.flags &= ~(physx::PxU8)ChunkExternallySupported;
			swap(supportDepthChunks[supportDepthChunksNotExternallySupportedCount++], supportDepthChunks[chunkNum++]);
			supportInvalid = true;
		}
	}
}

void DestructibleStructure::buildSupportGraph()
{
	PX_PROFILER_PERF_SCOPE("DestructibleStructureBuildSupportGraph");

	// First ensure external support data is up-to-date
	calculateExternalSupportChunks();

	// Now create graph

	if (destructibles.size() > 1)
	{
		// First record cached overlaps within each destructible
		physx::Array<IntPair> cachedOverlapPairs;
		for (physx::PxU32 i = 0; i < destructibles.size(); ++i)
		{
			DestructibleActor* destructible = destructibles[i];
			if (destructible != NULL)
			{
				DestructibleAsset* asset = destructibles[i]->getAsset();
				PX_ASSERT(asset);
				CachedOverlapsNS::IntPair_DynamicArray1D_Type* internalOverlaps = asset->getOverlapsAtDepth(destructible->getSupportDepth());
				cachedOverlapPairs.reserve(cachedOverlapPairs.size() + internalOverlaps->arraySizes[0]);
				for (int j = 0; j < internalOverlaps->arraySizes[0]; ++j)
				{
					CachedOverlapsNS::IntPair_Type& internalPair = internalOverlaps->buf[j];
					IntPair& pair = cachedOverlapPairs.insert();
					pair.i0 = internalPair.i0 + (physx::PxI32)destructible->getFirstChunkIndex();
					pair.i1 = internalPair.i1 + (physx::PxI32)destructible->getFirstChunkIndex();
				}
			}
		}

		physx::Array<IntPair> overlapPairs;
		physx::PxU32 overlapCount = overlapPairs.size();

		// Now find overlaps between destructibles.  Start by creating a destructible overlap list
		physx::Array<BoundsRep> destructibleBoundsReps;
		destructibleBoundsReps.reserve(destructibles.size());
		for (physx::PxU32 i = 0; i < destructibles.size(); ++i)
		{
			DestructibleActor* destructible = destructibles[i];
			BoundsRep& boundsRep = destructibleBoundsReps.insert();
			if (destructible != NULL)
			{
				boundsRep.aabb = destructible->getOriginalBounds();
			}
			else
			{
				boundsRep.type = 1;	// Will not test 1-0 or 1-1 overlaps
			}
			const physx::PxF32 paddingFactor = destructible->getAsset()->mParams->neighborPadding;
			const physx::PxF32 padding = paddingFactor * (destructible->getOriginalBounds().maximum - destructible->getOriginalBounds().minimum).magnitude();
			PX_ASSERT(!boundsRep.aabb.isEmpty());
			boundsRep.aabb.fattenFast(padding);
		}
		physx::Array<IntPair> destructiblePairs;
		{
			PX_PROFILER_PERF_DSCOPE("DestructibleStructureBoundsCalculateOverlaps", destructibleBoundsReps.size());
			BoundsInteractions interactions(false);
			interactions.set(0, 0, true);	// Only test 0-0 overlaps
			PX_ASSERT(destructibleBoundsReps.size() > 1);
			boundsCalculateOverlaps(destructiblePairs, Bounds3XYZ, &destructibleBoundsReps[0], destructibleBoundsReps.size(), sizeof(destructibleBoundsReps[0]), interactions);
		}

		// Test for chunk overlaps between overlapping destructibles
		for (physx::PxU32 overlapIndex = 0; overlapIndex < destructiblePairs.size(); ++overlapIndex)
		{
			const IntPair& destructibleOverlap = destructiblePairs[overlapIndex];

			DestructibleActor* destructible0 = destructibles[(physx::PxU32)destructibleOverlap.i0];
			const physx::PxU32 startChunkAssetIndex0 = destructible0->getAsset()->mParams->firstChunkAtDepth.buf[destructible0->getSupportDepth()];
			const physx::PxU32 stopChunkAssetIndex0 = destructible0->getAsset()->mParams->firstChunkAtDepth.buf[destructible0->getSupportDepth() + 1];
			const physx::PxF32 paddingFactor0 = destructible0->getAsset()->mParams->neighborPadding;
			const physx::PxF32 padding0 = paddingFactor0 * (destructible0->getOriginalBounds().maximum - destructible0->getOriginalBounds().minimum).magnitude();

			DestructibleActor* destructible1 = destructibles[(physx::PxU32)destructibleOverlap.i1];
			const physx::PxU32 startChunkAssetIndex1 = destructible1->getAsset()->mParams->firstChunkAtDepth.buf[destructible1->getSupportDepth()];
			const physx::PxU32 stopChunkAssetIndex1 = destructible1->getAsset()->mParams->firstChunkAtDepth.buf[destructible1->getSupportDepth() + 1];
			const physx::PxF32 paddingFactor1 = destructible1->getAsset()->mParams->neighborPadding;
			const physx::PxF32 padding1 = paddingFactor1 * (destructible1->getOriginalBounds().maximum - destructible1->getOriginalBounds().minimum).magnitude();

			// Find AABB overlaps

			// chunkBoundsReps contains bounds of all support chunks for both destructibles (first part destructible0, second part destructible1)
			physx::Array<BoundsRep> chunkBoundsReps;
			chunkBoundsReps.reserve((stopChunkAssetIndex0 - startChunkAssetIndex0) + (stopChunkAssetIndex1 - startChunkAssetIndex1));

			// Destructible 0 bounds
			for (physx::PxU32 chunkAssetIndex = startChunkAssetIndex0; chunkAssetIndex < stopChunkAssetIndex0; ++chunkAssetIndex)
			{
				BoundsRep& bounds = chunkBoundsReps.insert();
				physx::PxMat34Legacy scaledTM = destructible0->getInitialGlobalPose();
				scaledTM.M.multiplyDiagonal(destructible0->getScale());
				bounds.aabb = destructible0->getAsset()->getChunkActorLocalBounds(chunkAssetIndex);
				PxBounds3Transform(bounds.aabb, scaledTM.M, scaledTM.t);
				PX_ASSERT(!bounds.aabb.isEmpty());
				bounds.aabb.fattenFast(padding0);
				bounds.type = 0;
			}

			// Destructible 1 bounds
			for (physx::PxU32 chunkAssetIndex = startChunkAssetIndex1; chunkAssetIndex < stopChunkAssetIndex1; ++chunkAssetIndex)
			{
				BoundsRep& bounds = chunkBoundsReps.insert();
				physx::PxMat34Legacy scaledTM = destructible1->getInitialGlobalPose();
				scaledTM.M.multiplyDiagonal(destructible1->getScale());
				bounds.aabb = destructible1->getAsset()->getChunkActorLocalBounds(chunkAssetIndex);
				PxBounds3Transform(bounds.aabb, scaledTM.M, scaledTM.t);
				PX_ASSERT(!bounds.aabb.isEmpty());
				bounds.aabb.fattenFast(padding1);
				bounds.type = 1;
			}

			{
				PX_PROFILER_PERF_SCOPE("DestructibleStructureBoundsCalculateOverlaps");
				BoundsInteractions interactions(false);
				interactions.set(0, 1, true);	// Only test 0-1 overlaps
				if (chunkBoundsReps.size() > 0)
				{
					// this adds overlaps into overlapPairs with indices into the chunkBoundsReps array
					boundsCalculateOverlaps(overlapPairs, Bounds3XYZ, &chunkBoundsReps[0], chunkBoundsReps.size(), sizeof(chunkBoundsReps[0]), interactions, true);
				}
			}

			// Now do detailed overlap test
			const bool performDetailedOverlapTest = destructible0->performDetailedOverlapTestForExtendedStructures() || destructible1->performDetailedOverlapTestForExtendedStructures();
			{
				const physx::PxF32 padding = padding0 + padding1;

				// upperOffset transforms from chunkBoundsReps indices to asset chunk indices of the second destructible
				const physx::PxI32 upperOffset = physx::PxI32(startChunkAssetIndex1 - (stopChunkAssetIndex0 - startChunkAssetIndex0));

				PX_PROFILER_PERF_SCOPE("DestructibleStructureDetailedOverlapTest");
				const physx::PxU32 oldSize = overlapCount;
				for (physx::PxU32 overlapIndex = oldSize; overlapIndex < overlapPairs.size(); ++overlapIndex)
				{
					IntPair& AABBOverlap = overlapPairs[overlapIndex];
					// The new overlaps will be indexed incorrectly.  Fix the indexing.
					if (AABBOverlap.i0 > AABBOverlap.i1)
					{
						// Ensures i0 corresponds to destructible0, i1 to destructible 1
						// because lower chunkBoundsReps indices belong to destructible0
						physx::swap(AABBOverlap.i0, AABBOverlap.i1);
					}

					// transform chunkBoundsReps indices to asset chunk indices
					AABBOverlap.i0 += startChunkAssetIndex0;
					AABBOverlap.i1 += upperOffset;
					if (!performDetailedOverlapTest || DestructibleAsset::chunksInProximity(*destructible0->getAsset(), (physx::PxU16)AABBOverlap.i0, destructible0->getInitialGlobalPose(), destructible0->getScale(),
							*destructible1->getAsset(), (physx::PxU16)AABBOverlap.i1, destructible1->getInitialGlobalPose(), destructible1->getScale(), padding))
					{
						// Record overlap with global indexing
						IntPair& overlap = overlapPairs[overlapCount++];
						overlap.i0 = AABBOverlap.i0 + (physx::PxI32)destructible0->getFirstChunkIndex();
						overlap.i1 = AABBOverlap.i1 + (physx::PxI32)destructible1->getFirstChunkIndex();
					}
				}
			}
			overlapPairs.resize(overlapCount);
		}

		// We now have all chunk overlaps in the structure.
		// Make 'overlapPairs' symmetric - i.e. if (A,B) is in the array, then so is (B,A).
		overlapPairs.resize(2 * overlapCount);
		for (physx::PxU32 i = 0; i < overlapCount; ++i)
		{
			IntPair& overlap = overlapPairs[i];
			IntPair& recipOverlap = overlapPairs[i + overlapCount];
			recipOverlap.i0 = overlap.i1;
			recipOverlap.i1 = overlap.i0;
		}
		overlapCount *= 2;

		// Now sort overlapPairs by index0 and index1 (symmetric, so it doesn't matter which we sort by first)
		qsort(overlapPairs.begin(), overlapCount, sizeof(IntPair), IntPair::compare);


		// merge new overlaps with cached ones
		physx::Array<IntPair> allOverlapPairs(overlapPairs.size() + cachedOverlapPairs.size());
		ApexMerge(overlapPairs.begin(), overlapPairs.size(), cachedOverlapPairs.begin(), cachedOverlapPairs.size(), allOverlapPairs.begin(), allOverlapPairs.size(), IntPair::compare);
		overlapCount = allOverlapPairs.size();

		// Create the structure's chunk overlap list
		createIndexStartLookup(firstOverlapIndices, 0, chunks.size(), &allOverlapPairs.begin()->i0, overlapCount, sizeof(IntPair));
		overlaps.resize(overlapCount);
		for (physx::PxU32 overlapIndex = 0; overlapIndex < overlapCount; ++overlapIndex)
		{
			overlaps[overlapIndex] = (physx::PxU32)allOverlapPairs[overlapIndex].i1;
		}
	}
	else if (destructibles.size() == 1 && destructibles[0] != NULL)
	{
		// fast path for case of structures with only 1 destructible

		DestructibleAsset* asset = destructibles[0]->getAsset();
		CachedOverlapsNS::IntPair_DynamicArray1D_Type* internalOverlaps = asset->getOverlapsAtDepth(destructibles[0]->getSupportDepth());

		PxU32 numOverlaps = (physx::PxU32)internalOverlaps->arraySizes[0];

		// Create the structure's chunk overlap list
		createIndexStartLookup(firstOverlapIndices, 0, chunks.size(), &internalOverlaps->buf[0].i0, numOverlaps, sizeof(IntPair));

		overlaps.resize(numOverlaps);
		for (physx::PxU32 overlapIndex = 0; overlapIndex < numOverlaps; ++overlapIndex)
		{
			overlaps[overlapIndex] = (physx::PxU32)internalOverlaps->buf[overlapIndex].i1;
		}
	}

	// embed this call within DestructibleStructure, rather than exposing the StressSolver to other classes
	postBuildSupportGraph();
}

void DestructibleStructure::postBuildSupportGraph()
{
	if(NULL != stressSolver)
	{
		PX_DELETE(stressSolver);
		stressSolver = NULL;
		dscene->setStressSolverTick(this, false);
	}

	{
		// instantiate a stress solver if the user wants to use it
		for(physx::Array<DestructibleActor*>::ConstIterator kIter = destructibles.begin(); kIter != destructibles.end(); ++kIter)
		{
			const DestructibleActor & currentDestructibleActor = *(*kIter);
			PX_ASSERT(NULL != &currentDestructibleActor);
			if(currentDestructibleActor.useStressSolver())
			{
#if NX_SDK_VERSION_MAJOR == 3
				stressSolver = PX_NEW(DestructibleStructureStressSolver)(*this,currentDestructibleActor.getDestructibleParameters().supportStrength);
				if (!stressSolver->isPhysxBasedSim || stressSolver->isCustomEnablesSimulating)
				{
					dscene->setStressSolverTick(this, true);
				}
#elif NX_SDK_VERSION_MAJOR == 2
				stressSolver = PX_NEW(DestructibleStructureStressSolver)(*this);
				dscene->setStressSolverTick(this, true);
#endif
				break;
			}
		}
	}
}

void DestructibleStructure::evaluateForHitChunkList(const physx::Array<physx::PxU32> & chunkIndices) const
{
	PX_ASSERT(NULL != &chunkIndices);
	FractureEvent fractureEvent;
	::memset(static_cast<void *>(&fractureEvent), 0x00, 1 * sizeof(FractureEvent));
	for(physx::Array<physx::PxU32>::ConstIterator iter = chunkIndices.begin(); iter != chunkIndices.end(); ++iter)
	{
		DestructibleActor & destructibleActor = *(dscene->mDestructibles.direct(chunks[*iter].destructibleID));
		PX_ASSERT(NULL != &destructibleActor);
		if(0 != destructibleActor.getSyncParams().getUserActorID())
		{
			fractureEvent.chunkIndexInAsset = *iter - destructibleActor.getFirstChunkIndex();
			PX_ASSERT(fractureEvent.chunkIndexInAsset < destructibleActor.getChunkCount());
			destructibleActor.evaluateForHitChunkList(fractureEvent);
		}
	}
}

void DestructibleStructure::addChunkImpluseForceAtPos(Chunk& chunk, const physx::PxVec3& impulse, const physx::PxVec3& position, bool wakeup)
{
	physx::Array<NxShape*>& shapes = getChunkShapes(chunk);
	PX_ASSERT(!shapes.empty());
	if (!shapes.empty())
	{
		// All shapes should have the same actor
#if NX_SDK_VERSION_MAJOR == 2
		shapes[0]->getActor().addForceAtPos(NXFROMPXVEC3(impulse), NXFROMPXVEC3(position), NX_IMPULSE, wakeup);
#elif NX_SDK_VERSION_MAJOR == 3

		PxRigidActor* actor = shapes[0]->getActor();
		if (actor->getScene())
		{
			PxRigidDynamic* rigidDynamic = actor->isRigidDynamic();
			if (rigidDynamic && !(rigidDynamic->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC))
			{
                PxRigidBodyExt::addForceAtPos(*actor->isRigidBody(), impulse, position, PxForceMode::eIMPULSE, wakeup);
			}
		}
		else // the actor hasn't been added to the scene yet, so store the forces to apply later.
		{
			ActorForceAtPosition forceToAdd(impulse, position, PxForceMode::eIMPULSE, wakeup, true);
			dscene->addForceToAddActorsMap(actor, forceToAdd);
		}
#endif
	}
}

// http://en.wikipedia.org/wiki/Jenkins_hash_function
//   Fast and convenient hash for arbitrary multi-byte keys
physx::PxU32 jenkinsHash(const void *data, size_t sizeInBytes)
{
	physx::PxU32 hash, i;
	const char* key = reinterpret_cast<const char*>(data);
	for (hash = i = 0; i < sizeInBytes; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

physx::PxU32 DestructibleStructure::newNxActorIslandReference(Chunk& chunk, NxActor& nxActor)
{
	physx::PxU32 islandID = jenkinsHash(&chunk, sizeof(Chunk));

	// Prevent hash collisions
	while (NULL != islandToActor.find(islandID))
	{
		++islandID;
	}

	PX_ASSERT(islandID != InvalidID);

	islandToActor[islandID] = &nxActor;
	actorToIsland[&nxActor] = islandID;

	dscene->setStructureUpdate(this, true);	// islandToActor needs to be cleared in DestructibleStructure::tick

	return islandID;
}

void DestructibleStructure::removeNxActorIslandReferences(NxActor& nxActor) const
{
	islandToActor.erase(actorToIsland[&nxActor]);
	actorToIsland.erase(&nxActor);
}

NxActor* DestructibleStructure::getChunkActor(Chunk& chunk)
{
	physx::Array<NxShape*>& shapes = getChunkShapes(chunk);
#if NX_SDK_VERSION_MAJOR == 2
	return !shapes.empty() ? (NxActor*)&shapes[0]->getActor() : NULL;
#elif NX_SDK_VERSION_MAJOR == 3
	NxActor* actor;
	SCOPED_PHYSX_LOCK_READ(*dscene->mApexScene);
	actor = !shapes.empty() ? (NxActor*)shapes[0]->getActor() : NULL;
	return actor;
#endif
}

const NxActor* DestructibleStructure::getChunkActor(const Chunk& chunk) const
{
	const physx::Array<NxShape*>& shapes = getChunkShapes(chunk);
#if NX_SDK_VERSION_MAJOR == 2
	return !shapes.empty() ? (NxActor*)&shapes[0]->getActor() : NULL;
#elif NX_SDK_VERSION_MAJOR == 3
	NxActor* actor;
	SCOPED_PHYSX_LOCK_READ(*dscene->mApexScene);
	actor = !shapes.empty() ? (NxActor*)shapes[0]->getActor() : NULL;
	return actor;
#endif
}

physx::PxMat34Legacy DestructibleStructure::getChunkLocalPose(const Chunk& chunk) const
{
	const physx::Array<NxShape*>* shapes;
	physx::PxVec3 offset;
	if (chunk.visibleAncestorIndex == (physx::PxI32)InvalidChunkIndex)
	{
		shapes = &chunk.shapes;
		offset = physx::PxVec3(0.0f);
	}
	else
	{
		shapes = &chunks[(physx::PxU32)chunk.visibleAncestorIndex].shapes;
		offset = chunk.localOffset - chunks[(physx::PxU32)chunk.visibleAncestorIndex].localOffset;
	}

	physx::PxMat34Legacy pose;
	if (!shapes->empty() && NULL != (*shapes)[0])
	{
#if NX_SDK_VERSION_MAJOR == 3
		SCOPED_PHYSX3_LOCK_READ(dscene->getModulePhysXScene());
#endif
		// All shapes should have the same global pose
		PxFromNxMat34(pose, (*shapes)[0]->getLocalPose());
	}
	else
	{
		pose.setIdentity();
	}
	pose.t += offset;
	return pose;
}

physx::PxMat34Legacy DestructibleStructure::getChunkActorPose(const Chunk& chunk) const
{
	const physx::Array<NxShape*>& shapes = getChunkShapes(chunk);
	physx::PxMat34Legacy pose;
	if (!shapes.empty() && NULL != shapes[0])
	{
		// All shapes should have the same actor
#if NX_SDK_VERSION_MAJOR == 2
		PxFromNxMat34(pose, shapes[0]->getActor().getGlobalPose());
#elif NX_SDK_VERSION_MAJOR == 3
		SCOPED_PHYSX3_LOCK_READ(dscene->getModulePhysXScene());
		PxFromNxMat34(pose, shapes[0]->getActor()->getGlobalPose());
#endif
	}
	else
	{
		pose.setIdentity();
	}
	return pose;
}

void DestructibleStructure::setSupportInvalid(bool supportIsInvalid)
{
	supportInvalid = supportIsInvalid;
	dscene->setStructureUpdate(this, supportIsInvalid);
}

}
}
} // end namespace physx::apex

#endif

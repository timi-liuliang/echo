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

#include "PxvConfig.h"
#include "PxsAABBManager.h"
#include "PxsBroadPhaseConfig.h"
#include "GuRevisitedRadixBuffered.h"
#include "PsUtilities.h"
#include "PsAllocator.h"
#include "CmTmpMem.h"
#include "PxsBroadPhaseSap.h"
#include "PxsRigidBody.h"
#include "CmEventProfiler.h"
#include "PxProfileEventId.h"

using namespace physx;

#ifdef PX_PS3
	#include "PS3Support.h"
	#include "CellSPUProfiling.h"
	#include "CellTimerMarker.h"
#endif //PX_PS3

#define DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY 64

///////////////////////////////////////////////////

/*
	- what happens to bitmaps when a subpart gets deleted? (i.e. "nb" changes)
	Very tedious to handle this case, all bitmaps from all pairs containing the aggregate must be upgraded

	Different cases:

	I) object gets added to an existing aggregate. "nb++", and the new element becomes the *head* of the
	linked list, so the mapping between the bitmaps and the objects changes.

		1) The aggregate's "selfCollBitmap" must be reallocated with (nb+1)*(nb+1) bits and the previous
		(nb)*(nb) bitmap must be transferred to the larger one. Tedious but not very difficult.

		2) all "aggregateCollBitmap" of all AggregatePair involving the modified aggregate must be updated.
		This requires parsing the mAggregatePairs array, as in 'purgeAggregatePairs'. Each bitmap of each
		pair involving the modified aggregate must be upgraded.

			a) if the pair is aggregate-aggregate, the bitmap must be upgraded from (nb0)*(nb1) to (nb0+1)*(nb1)
			b) if the pair is single-aggregate, the bitmap must be upgraded from (nb0) to (nb0+1)

	II) object gets removed from an existing aggregate. "nb--" and the removed element can be anywhere in the
	linked list. We need to know the element's position in order to upgrade the bitmaps correctly. Contrary
	to the previous case where the new element was always inserted in the same position (the head), the hole
	left by the removed element can be anywhere, and the remapping code depends on the hole index.

		1) The aggregate's "selfCollBitmap" must be reallocated with (nb-1)*(nb-1) bits and the previous
		(nb)*(nb) bitmap must be transferred to the smaller one.

		2) all "aggregateCollBitmap" of all AggregatePair involving the modified aggregate must be updated.
		This requires parsing the mAggregatePairs array, as in 'purgeAggregatePairs'. Each bitmap of each
		pair involving the modified aggregate must be upgraded.

			a) if the pair is aggregate-aggregate, the bitmap must be upgraded from (nb0)*(nb1) to (nb0-1)*(nb1)
			b) if the pair is single-aggregate, the bitmap must be upgraded from (nb0) to (nb0-1)

	So in short, this is hideous.

	Moreover:
	- we want to minimize the remaps I guess. If we remove 2 objects from a aggregate, do we do the remap twice
	(more expensive), or do we do the remap once, taking into account both objects (more complicated)?

	Notes:
	- for removals, maybe we can avoid "filling the hole". We may simply invalidate the linked list entry without
	actually deleting it, and just keep the bitmap until the aggregate is deleted (self CD bitmap) or until the
	pair separates. After all, removing an object from one of those dynamic aggregate (e.g. ragdoll) shouldn't
	happen. When it does, the aggregate parts might go far away from each other, in which case we have bigger issues
	(e.g. the aggregate bounds become very large and the whole thing becomes very inefficient).
	- maybe we should just ban the whole operation.

	--------------------------------------------------------------------------

	- mix of 2 solutions? pass aggregate ID down to Sim creation, then save it in ActorSim

	- reoptimize away useless imul/idiv (if any) introduced by SPU changes + useless CMPs in IDToElem
	- "aggregate" viz is confusing
	- test bitmap promotion
	- new design:
		- refiltering pain
		- serial
		- self collisions bitmap
		- arrays of aggregates in npscene or something
	- redo aggregate allocation/management
	- use a bitmap iterator?
	- refactor small pieces of code like bitmap allocation, etc
	- pass integer bounds as-is to underlying BP
	- optimize:
		- skip self-cd etc also when new bounds are "the same" as before?
		- use timestamps for deletion?
	- use box pruning?
	- remove linked lists, do a shared sorted buffer
	- better memory buffer usage. When it's all done, compare memory usage to normal code.
	- fix debug colors!
*/

// PT: one non-inlined resize function for everybody please

///////////////////////////////////////////////////////////////////////////////

PxsAABBManager::PxsAABBManager(Cm::EventProfiler& eventProfiler, PxcScratchAllocator& scratchAllocator) 
:   mScratchAllocator(scratchAllocator),
	mEventProfiler(eventProfiler),
    mBPUpdatedElemIds(&scratchAllocator),
    mBPUpdatedElemIdsSize(0),
    mAggregateUpdatedElemIds(&scratchAllocator),
    mAggregateUpdatedElemIdsSize(0),
#ifdef PX_PS3
    mBPUpdatedElemWordStarts(&scratchAllocator),
    mBPUpdatedElemWordEnds(&scratchAllocator),
    mBPUpdatedElemWordCount	(0),
    mAggregateUpdatedElemWordStarts(&scratchAllocator),
    mAggregateUpdatedElemWordEnds(&scratchAllocator),
    mAggregateUpdatedElemWordCount(0),
#endif
	mAggregateActorShapeIds(&scratchAllocator),
	mDirtyActorAggregates(&scratchAllocator),
	mNumDirtyActorAggregates(0),
	mAggregatesToSort(&scratchAllocator),
	mNumAggregatesToSort(0),
	mAggregatePairsToOverlap(&scratchAllocator),
	mNumAggregatePairsToOverlap(0),
	mBP						(NULL),
	mCreatedPairs			(NULL),
	mCreatedPairsSize		(0),
	mCreatedPairsCapacity	(0),
	mDeletedPairs			(NULL),
	mDeletedPairsSize		(0),
	mDeletedPairsCapacity	(0),
	mAggregatePairs			(NULL),
	mAggregatePairsSize		(0),
	mAggregatePairsCapacity	(0)
{
#if defined(PX_OSX)
	//workaround for DE9738:
	//clang-503.0.40 shipped with Apple LLVM version 5.1
	//seems to exhibit a bug when generating sse code.
	//The code fails only for the last four bitshifts.
	for(PxU32 i=0;i<28;i++)
	{
		mBitmasks[i] = PxU32(1<<i);
	}
	mBitmasks[28] = PxU32(0x100000000);
	mBitmasks[29] = PxU32(0x200000000);
	mBitmasks[30] = PxU32(0x400000000);
	mBitmasks[31] = PxU32(0x800000000);
#else
	for(PxU32 i=0;i<32;i++)
	{
		mBitmasks[i] = PxU32(1<<i);
	}
#endif
	
	preAllocate(PX_DEFAULT_BOX_ARRAY_CAPACITY, PX_DEFAULT_BOX_ARRAY_CAPACITY, PX_DEFAULT_AGGREGATE_CAPACITY);
}

PxsAABBManager::~PxsAABBManager()
{
	for(PxU32 i=0;i<mAggregatePairsSize;i++)
	{
		if(mAggregatePairs[i].aggregateCollBitmap)
		{
			PX_DELETE(mAggregatePairs[i].aggregateCollBitmap);
			mAggregatePairs[i].aggregateCollBitmap=NULL;
		}
	}

	mCreatedPairs = NULL;
	mDeletedPairs = NULL;
	PX_FREE(mAggregatePairs);

	mBPUpdatedElemIds.free();
	mAggregateUpdatedElemIds.free();

#ifdef PX_PS3
	mBPUpdatedElemWordStarts.free();
	mBPUpdatedElemWordEnds.free();
	mAggregateUpdatedElemWordStarts.free();
	mAggregateUpdatedElemWordEnds.free();
#endif

	mAggregatesToSort.free();
	mAggregatePairsToOverlap.free();
}

void PxsAABBManager::init(PxvBroadPhase* bp)
{
	mBP = bp;
}

void PxsAABBManager::preAllocate(const PxU32 nbStaticShapes, const PxU32 nbDynamicShapes, const PxU32 numAggregates)
{
	const PxU32 nbStaticShapes32 = ((nbStaticShapes  + 31) & ~31);
	const PxU32 nbDynamicShapes32 = ((nbDynamicShapes  + 31) & ~31);

	const PxU32 nbElems = nbStaticShapes32 + nbDynamicShapes32;

	if(!nbElems && !numAggregates)
		return;

	if(mBPElems.getCapacity() < nbElems)
	{
		mBPElems.setDefaultCapacity(nbStaticShapes32, nbDynamicShapes32);

		//these arrays needs to at least track the number of bp elems
		mBPUpdatedElems.setDefaultElemsCapacity(nbElems);
		mBPCreatedElems.setDefaultElemsCapacity(nbElems);
		mBPRemovedElems.setDefaultElemsCapacity(nbElems);
		mBPUpdatedElems.growBitmap(nbElems);
		mBPCreatedElems.growBitmap(nbElems);
		mBPRemovedElems.growBitmap(nbElems);
	}

	if(mAggregateElems.getCapacity() < nbElems)
	{
		mAggregateElems.resize(nbElems);
	}

	if(mSingleManager.getCapacity()<nbElems)
	{
		mSingleManager.resize(nbElems);
	}

	if(mAggregateManager.getAggregatesCapacity() < numAggregates)
	{
		//Round up numAggregates to a nice multiple of 4.
		const PxU32 numAggregates4 = (numAggregates + 3) & ~3;
		mAggregateManager.resize(numAggregates4);
		mAggregatesUpdated.growBitmap(numAggregates4);
	}
}

void PxsAABBManager::destroyV()
{
	mBP->destroy();
	delete this;
}

void PxsAABBManager::freeCreatedOverlaps()
{
	if(mCreatedPairs) mScratchAllocator.free(mCreatedPairs);
	mCreatedPairs = NULL;
	mCreatedPairsCapacity = 0;
	mCreatedPairsSize = 0;
}

void PxsAABBManager::freeDestroyedOverlaps()
{
	if(mDeletedPairs) mScratchAllocator.free(mDeletedPairs);
	mDeletedPairs = NULL;
	mDeletedPairsCapacity = 0;
	mDeletedPairsSize = 0;
}

///////////////////////////////////////////////////////////////////////////////

#include "CmRenderOutput.h"
void PxsAABBManager::visualize(Cm::RenderOutput& out)
{
	PxTransform idt = PxTransform(PxIdentity);
	out << idt;

	const PxU32 N=mAggregateManager.getAggregatesCapacity();
	for(PxU32 i=0;i<N;i++)
	{
		const Aggregate* aggregate=mAggregateManager.getAggregate(i);
		if(aggregate->nbElems)
		{
			if(!mAggregatesUpdated.isInList((PxcBpHandle)i))
			{
				out << PxU32(PxDebugColor::eARGB_GREEN);
			}
			else
			{
				out << PxU32(PxDebugColor::eARGB_RED);
			}

			PxBounds3 decoded;
			const IntegerAABB iaabb=mBPElems.getAABB(aggregate->bpElemId);
			iaabb.decode(decoded);

			out << Cm::DebugBox(decoded, true);
			PxU32 elem = aggregate->elemHeadID;
			while(PX_INVALID_BP_HANDLE!=elem)
			{
				out << PxU32(PxDebugColor::eARGB_CYAN);
				const IntegerAABB elemBounds = mAggregateElems.getAABB(elem);
				elemBounds.decode(decoded);
				out << Cm::DebugBox(decoded, true);
				elem = mAggregateElems.getNextId(elem);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

PxU32 PxsAABBManager::createAggregate(void* userData, const bool selfCollisions)
{
	//Create an aggregate and return the encoded aggregate id.
	const PxU32 oldCapacity=mAggregateManager.getAggregatesCapacity();
	const PxcBpHandle aggregateId = mAggregateManager.createAggregate(userData, selfCollisions);
	const PxU32 newCapacity=mAggregateManager.getAggregatesCapacity();
	if(newCapacity>oldCapacity)
	{
		mAggregatesUpdated.growBitmap(newCapacity);
	}
	return encodeAggregateForClient(aggregateId);
}

void PxsAABBManager::deleteAggregate(const PxU32 encodedAggregateId)
{
	PX_ASSERT(isClientVolumeAggregate((PxcBpHandle)encodedAggregateId));
	const PxcBpHandle aggregateId=decodeAggregateFromClient((PxcBpHandle)encodedAggregateId);
	PX_ASSERT(0==mAggregateManager.getAggregate(aggregateId)->nbElems);
	PX_ASSERT(PX_INVALID_BP_HANDLE==mAggregateManager.getAggregate(aggregateId)->elemHeadID);
	PX_ASSERT(PX_INVALID_BP_HANDLE==mAggregateManager.getAggregate(aggregateId)->bpElemId);
	PX_ASSERT(PX_INVALID_BP_HANDLE!=mAggregateManager.getAggregate(aggregateId)->group);
	Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);
	mAggregateActorManager.freeActors(aggregate->actorHeadID);
	mAggregateManager.reuseAggregate(aggregateId);
}

AABBMgrId PxsAABBManager::createVolume(const PxU32 encodedAggregateId, const PxcBpHandle encodedActorHandle, const PxU32 group, void* userdata, const PxBounds3& bounds)
{
	if(encodedAggregateId==PX_INVALID_U32)
	{
		//The incoming object should be handled as a singleton ("fast" path).

		//Encode the bounds.
		IntegerAABB iaabb;
		iaabb.encode(bounds);

		//Get the bp elem id.
		const PxcBpHandle bpElemId = createBPElem();
		PX_ASSERT(bpElemId < mBPElems.getCapacity());

		//Add the created bp elem id to the list of created elems.
		mBPCreatedElems.addElem(bpElemId);

		//Set up the bp elem as a single.
		mBPElems.initAsSingle(bpElemId, userdata, (PxcBpHandle)group, iaabb); 

		//Connect the bp elem to the structure that manages all bp elems shared by the same rigid body.
		PxcBpHandle actorHandle=PX_INVALID_BP_HANDLE;
		if(PX_INVALID_BP_HANDLE==encodedActorHandle)
		{
			//This is the first bp entry of a rigid body (a rigid body can have multiple shapes).
			actorHandle=mSingleManager.createSingle();
			Single* single=mSingleManager.getSingle(actorHandle);
			mBPElems.setSingleOwnerId(bpElemId,actorHandle);
			mBPElems.setNextId(bpElemId, PX_INVALID_BP_HANDLE);
			single->headID=bpElemId;
		}
		else 
		{
			//This isn't the first bp entry of a rigid body.  
			PX_ASSERT(!isClientVolumeAggregate(encodedActorHandle));
			actorHandle=decodeSingleFromClient(encodedActorHandle);
			Single* single=mSingleManager.getSingle(actorHandle);
			mBPElems.setSingleOwnerId(bpElemId,actorHandle);
			const PxcBpHandle headId=single->headID;
			mBPElems.setNextId(bpElemId, headId);
			single->headID=bpElemId;
		}

		//Return an encoded id (internal id + isAggregate).
		if(canEncodeForClient(bpElemId) && canEncodeForClient(actorHandle))
		{
			return AABBMgrId(encodeSingleForClient(bpElemId), encodeSingleForClient(actorHandle));
		}
		else
		{
			return AABBMgrId(PX_INVALID_BP_HANDLE, PX_INVALID_BP_HANDLE);
		}
	}

	PX_ASSERT(isClientVolumeAggregate((PxcBpHandle)encodedAggregateId));
	const PxcBpHandle aggregateId = decodeAggregateFromClient((PxcBpHandle)encodedAggregateId);
	Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);

	if(PX_INVALID_BP_HANDLE == aggregate->elemHeadID)
	{
		//Incoming object is a aggregate but we've not yet added an element to the aggregate.
		//Alternatively we have added to the aggregate then removed all aggregate elements and are now adding again to an empty aggregate.

		//Encode the bounds (we need this later to initialise the bp elem bounds and to set the aggregate elem bounds).
		IntegerAABB encoded;
		encoded.encode(bounds);

		//Sanity test the aggregate.
		PX_ASSERT(0 == aggregate->nbActive);
		PX_ASSERT(PX_INVALID_BP_HANDLE == aggregate->bpElemId);
		PX_ASSERT(PX_INVALID_BP_HANDLE != aggregate->group);

		//Create a bp elem for the aggregate and set it up so the aggregate
		//knows the bp elem id and the bp elem knows the aggregate id.
		//Also set the bounds of the bp elem.
		const PxcBpHandle bpElemId = createBPElem();
		aggregate->bpElemId=bpElemId;
		mBPElems.setAggregateOwnerId(bpElemId, aggregateId);
		mBPElems.setGroup(bpElemId, aggregate->group);
		mBPElems.initAsAggregate(bpElemId,encoded);

		//Add to the created list of bp elems.
		mBPCreatedElems.addElem(bpElemId);

		//Create a aggregate elem for the aabb and set it up.
		const PxcBpHandle aggregateElemID = createAggregateElem();
		PX_ASSERT(aggregateElemID < mAggregateElems.getCapacity());
		mAggregateElems.init(aggregateElemID, userdata, (PxcBpHandle)group, encoded, PX_INVALID_BP_HANDLE);

		//Set up the aggregate.
		if(!mAggregatesUpdated.isInList(aggregateId))
		{
			mAggregatesUpdated.addElem(aggregateId);
		}
		aggregate->elemHeadID = aggregateElemID;
		aggregate->nbElems++;
		aggregate->nbActive++;

		//Create an actor for the aggregate if necessary
		PX_ASSERT((PX_INVALID_BP_HANDLE != aggregate->actorHeadID) || (PX_INVALID_BP_HANDLE == encodedActorHandle));
		PxcBpHandle actorId = PX_INVALID_BP_HANDLE;
		if(PX_INVALID_BP_HANDLE == encodedActorHandle)
		{
			actorId = mAggregateActorManager.getAvailableElem();
			mAggregateActorManager.setAggregateId(actorId, aggregateId);
			if(PX_INVALID_BP_HANDLE == aggregate->actorHeadID)
			{
				mAggregateActorManager.setNextId(actorId, PX_INVALID_BP_HANDLE);
				aggregate->actorHeadID = actorId;
			}
			else
			{
				mAggregateActorManager.setNextId(actorId, aggregate->actorHeadID);
				aggregate->actorHeadID = actorId;
			}
		}
		else
		{
			actorId = decodeAggregateFromClient(encodedActorHandle);
		}
		mAggregateElems.setAggregateActorId(aggregateElemID, actorId);
		mAggregateActorManager.incrementShapeCount(actorId);

		//Set the actor dirty but only if the aggregate has no self-collision
		//(we don't care about the actor bounds if the actor has self-collision
		//because we will compute aggregate bounds from shape bounds in that case.)
		if(!aggregate->selfCollide) mAggregateActorManager.setDirty(actorId);

		if(canEncodeForClient(aggregateId) && canEncodeForClient(actorId))
		{
			return AABBMgrId(encodeAggregateForClient(aggregateElemID), encodeAggregateForClient(actorId));
		}
		else
		{
			return AABBMgrId(PX_INVALID_BP_HANDLE, PX_INVALID_BP_HANDLE);
		}
	}
	else 
	{
		IntegerAABB encoded;
		encoded.encode(bounds);

		//Create a aggregate elem for the aabb.
		PxcBpHandle aggregateElemID;
		PxU32 internalIndex=mAggregateManager.getAvailableElem(aggregateId);
		if(PX_INVALID_BP_HANDLE != internalIndex)
		{
			//Remember that internalIndex was computed by counting 
			//backwards from the end of the list because the start of the 
			//list can change if we add an extra element to the aggreagate.
			//We need to first get a list of all elements in the aggregate
			//so we can get the reusable element by counting backwards from the end.
			PxcBpHandle componentElemIds[MAX_AGGREGATE_BOUND_SIZE];
			aggregateElemID = aggregate->elemHeadID;
			PxU32 index=0;
			while(aggregateElemID!=PX_INVALID_BP_HANDLE)
			{
				componentElemIds[index]=aggregateElemID;
				index++;
				aggregateElemID = mAggregateElems.getNextId(aggregateElemID);
			}
			PX_ASSERT(index == aggregate->nbElems);

			//Select the one that can be reused.
			const PxU32 componentElemIndex = aggregate->nbElems - 1 - internalIndex;
			PX_ASSERT(componentElemIndex < MAX_AGGREGATE_BOUND_SIZE);
			aggregateElemID = componentElemIds[componentElemIndex];

			//Reuse it.
			mAggregateElems.reinit(aggregateElemID, userdata, (PxcBpHandle)group, encoded);

			//Set up the aggregate.
			aggregate->nbActive++;

			//Add the aggregate to the updated list.
			if(!mAggregatesUpdated.isInList(aggregateId))
			{
				mAggregatesUpdated.addElem(aggregateId);
			}
		}
		else if(aggregate->nbElems < MAX_AGGREGATE_BOUND_SIZE)
		{
			promoteBitmaps(aggregate);

			aggregateElemID = createAggregateElem();
			mAggregateElems.init(aggregateElemID, userdata, (PxcBpHandle)group, encoded, aggregate->elemHeadID);

			//Set up the aggregate.
			aggregate->elemHeadID = aggregateElemID;
			aggregate->nbElems++;
			aggregate->nbActive++;

			//Add the aggregate to the updated list.
			if(!mAggregatesUpdated.isInList(aggregateId))
			{
				mAggregatesUpdated.addElem(aggregateId);
			}
		}
		else
		{
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "A PxAggregate has exceeded the limit of 128 PxShapes. Not all shapes of the aggregate will be added to the broapdhase");
			return AABBMgrId();
		}

		// Merge/update bounds in the BP
		const PxcBpHandle bpElemId=aggregate->bpElemId;
		PX_ASSERT(bpElemId < mBPElems.getCapacity());

		IntegerAABB iaabb=mBPElems.getAABB(bpElemId);
		if(!encoded.isInside(iaabb))
		{
			iaabb.include(encoded);
			mBPElems.setAABB(bpElemId,iaabb);

			//If the element is not in the created list then we need to think about adding it to the updated list.
			//If it is in the created list then just leave it there.
			if(!mBPCreatedElems.isInList(bpElemId))
			{
				if(!mBPUpdatedElems.isInList(bpElemId))
				{
					mBPUpdatedElems.addElem(bpElemId);
				}
			}
		}

		//Make an actor if necessary
		PxcBpHandle actorId = PX_INVALID_BP_HANDLE;
		if(PX_INVALID_BP_HANDLE == encodedActorHandle)
		{
			actorId = mAggregateActorManager.getAvailableElem();
			mAggregateActorManager.setAggregateId(actorId, aggregateId);
			if(PX_INVALID_BP_HANDLE == aggregate->actorHeadID)
			{
				mAggregateActorManager.setNextId(actorId, PX_INVALID_BP_HANDLE);
				aggregate->actorHeadID = actorId;
			}
			else
			{
				mAggregateActorManager.setNextId(actorId, aggregate->actorHeadID);
				aggregate->actorHeadID = actorId;
			}
		}
		else
		{
			actorId = decodeAggregateFromClient(encodedActorHandle);
		}
		mAggregateElems.setAggregateActorId(aggregateElemID, actorId);
		mAggregateActorManager.incrementShapeCount(actorId);

		//Set the actor dirty but only if the aggregate has no self-collision
		//(we don't care about the actor bounds if the actor has self-collision
		//because we will compute aggregate bounds from shape bounds in that case.)
		if(!aggregate->selfCollide) mAggregateActorManager.setDirty(actorId);

		if(canEncodeForClient(aggregateElemID) && canEncodeForClient(actorId))
		{
			return AABBMgrId(encodeAggregateForClient(aggregateElemID),encodeAggregateForClient(actorId));
		}
		else
		{
			return AABBMgrId(PX_INVALID_BP_HANDLE, PX_INVALID_BP_HANDLE);
		}
	}
}

void PxsAABBManager::setStaticAABBData(const PxcBpHandle shapeHandle, const PxcAABBDataStatic& aabbData)
{
	if(!isClientVolumeAggregate(shapeHandle))
	{
		mBPElems.setStaticAABBData(decodeSingleFromClient(shapeHandle), aabbData);
	}
	else
	{
		const PxcBpHandle shapeId = decodeAggregateFromClient(shapeHandle);
		mAggregateElems.setStaticAABBData(shapeId, aabbData);
	}
}

void PxsAABBManager::setDynamicAABBData(const PxcBpHandle shapeHandle, const PxcAABBDataDynamic& aabbData)
{
	if(!isClientVolumeAggregate(shapeHandle))
	{
		mBPElems.setDynamicAABBData(decodeSingleFromClient(shapeHandle), aabbData);
	}
	else
	{
		const PxcBpHandle shapeId = decodeAggregateFromClient(shapeHandle);
		mAggregateElems.setDynamicAABBData(shapeId, aabbData);
	}
}

bool PxsAABBManager::releaseVolume(const PxcBpHandle shapeHandle)
{
	bool removingLastShape = false;
	if(!isClientVolumeAggregate(shapeHandle))
	{
		const PxcBpHandle bpElemId = decodeSingleFromClient(shapeHandle);

		purgeAggregatePairs(bpElemId);

		//If the volume is in the updated list then remove it.
		if(mBPUpdatedElems.isInList(bpElemId))
		{
			mBPUpdatedElems.removeElem(bpElemId);
		}

		//If the volume is in the created list then remove it.
		//If the volume wasn't in the created list then add it to the removed list.
		//Remember that we only need to remove elements from the bp that have already been added and sorted.
		//If the element has never been inserted into the bp then don't add it to the list to be removed from the bp.
		bool reuseBpElemId = false;
		if(mBPCreatedElems.isInList(bpElemId))
		{
			reuseBpElemId = true;
			mBPCreatedElems.removeElem(bpElemId);
		}
		else
		{
			mBPRemovedElems.addElem(bpElemId);
		}

		//Patch up the linked list of singles with common ownership.
		const PxcBpHandle singleId = mBPElems.getSingleOwnerId(bpElemId);
		Single* single=mSingleManager.getSingle(singleId);
		PxcBpHandle idPrev=single->headID;
		PX_ASSERT(idPrev!=PX_INVALID_BP_HANDLE);
		PxcBpHandle idCurr=mBPElems.getNextId(idPrev);
		if(bpElemId==idPrev)
		{
			//The first element in the linked list is to be removed.
			if(PX_INVALID_BP_HANDLE==idCurr)
			{
				//The linked list has only a single entry so the list will be empty after removing the entry.
				mSingleManager.clearSingle(singleId);
				mSingleManager.reuseSingle(singleId);
				mBPElems.setNextId(idPrev, PX_INVALID_BP_HANDLE);
				removingLastShape = true;
			}
			else
			{
				//Just set the headId to be the second entry in the linked list.
				single->headID=idCurr;
			}
		}
		else
		{
			//Find the entry in the list that is to be removed.
			while(idCurr!=bpElemId)
			{
				idPrev=idCurr;
				idCurr=mBPElems.getNextId(idCurr);
			}
			//Connect the previous entry to the next entry.
			mBPElems.setNextId(idPrev, mBPElems.getNextId(idCurr));
			//Invalidate the removed entry.
			mBPElems.setNextId(idCurr, PX_INVALID_BP_HANDLE);
		}

		//NOTE: Don't free the shape id yet because it could then be immediately reused, leading to confusion
		//		because the same id would appear in the created and removed lists.
		//		Only reuse the id when the next simulate is called during updateAABBsAndBP.
		//NOTE: If the id was added and immediately removed then we can reuse the id immediately.
		if(reuseBpElemId)
		{
			mBPElems.freeElem(bpElemId);
		}
	}
	else
	{
		//Get the aggregate elem id.
		//Get the aggregate actor id from the aggregate elem id.
		//Get the aggregate id from the aggregate actor id.
		//Get the aggregate from the aggregate id.
		const PxcBpHandle aggregateElemId = decodeAggregateFromClient(shapeHandle);
		const PxcBpHandle actorId = mAggregateElems.getAggregateActorId(aggregateElemId);
		const PxcBpHandle aggregateId = mAggregateActorManager.getAggregateId(actorId);
		Aggregate* PX_RESTRICT aggregate = mAggregateManager.getAggregate(aggregateId);

		//Set the actor dirty but only if the aggregate has no self-collision
		//(we don't care about the actor bounds if the actor has self-collision
		//because we will compute aggregate bounds from shape bounds in that case.)
		if(!aggregate->selfCollide) mAggregateActorManager.setDirty(actorId);
		//Decrement the shape count by 1 so we can ignore actors with zero shapes.
		removingLastShape = mAggregateActorManager.decrementShapeCount(actorId);

		//Decrement nbActive because we lost a shape.
		aggregate->nbActive--;	
		//Add aggregate to bitmap of aggregates that need updated.
		if(!mAggregatesUpdated.isInList(aggregateId))
		{
			mAggregatesUpdated.addElem(aggregateId);
		}

		//We need to preserve the position of all the elements in the linked list to preserve the meaning of the overlap bitmaps.
		//This means we need to keep the removed element in the linked list but flag it so that in the future we neglect it when 
		//updating bp bounds, overlapping single-aggregate pairs etc.
		//We obviously can't reuse the element we're trying to remove because we need to preserve the flag that marks it to be neglected.
		//Mark the element to be removed with an invalid group id.
		//Set the aabb to empty just to help us with debugging.
		mAggregateElems.setGroup(aggregateElemId, PX_INVALID_BP_HANDLE);	
		mAggregateElems.setEmptyAABB(aggregateElemId);	

		//Mark the element so that it can be reused after all bitmaps have been cleared.
		//Because we keep changing the head id when we add new elements to the aggregate
		//we need to count backwards from the end of the linked list rather than forwards
		//from the start.  As a consequence we need to mark (aggregate->nbElems - 1 - internalIndex)
		//rather than just internalIndex.  
		PxcBpHandle headId = aggregate->elemHeadID;
		PxU32 internalIndex = 0;
		while(headId != aggregateElemId)
		{
			internalIndex++;
			headId = mAggregateElems.getNextId(headId);
		}
		mAggregateManager.releaseElem(aggregateId, (PxcBpHandle)(aggregate->nbElems - 1 - internalIndex));

		//NOTE: Don't free the shape id yet because it could then be immediately reused, leading to confusion
		//		because the same id would appear in the created and removed lists.
		//		Only reuse the id when the next simulate is called during updateAABBsAndBP.

		//If there are no active elements remaining in the aggregate then take action by removing the aggregate from the bp.
		//We can now free all the aggregate's elems.
		if(!aggregate->nbActive)
		{
			//Get the bp elem id of the aggregate.
			const PxcBpHandle bpElemId=aggregate->bpElemId;

			//Remove all overlap pairs involving the bpElem
			purgeAggregatePairs(bpElemId);

			//If the aggregate was in the updated list then remove it from the updated list
			if(mAggregatesUpdated.isInList(aggregateId))
			{
				mAggregatesUpdated.removeElem(aggregateId);
			}

			//If the bp element is in the updated list then remove it from the updated list.
			if(mBPUpdatedElems.isInList(bpElemId))
			{
				mBPUpdatedElems.removeElem(bpElemId);
			}

			//If the volume is in the created list then remove it from the created list.
			//If the volume wasn't in the created list then add it to the removed list.
			//Remember that we only need to remove elements from the bp that have already been added and sorted.
			//If the element has never been inserted into the bp then don't add it to the list to be removed from the bp.
			bool reuseBpElemId = false;
			if(mBPCreatedElems.isInList(bpElemId))
			{
				reuseBpElemId = true;
				mBPCreatedElems.removeElem(bpElemId);
			}
			else
			{
				mBPRemovedElems.addElem(bpElemId);
			}

			//Now free all the elements of the aggregate (including the volume of the aggregate that is being freed).
			PxcBpHandle elemId=aggregate->elemHeadID;
			while(PX_INVALID_BP_HANDLE!=elemId)
			{
				const PxcBpHandle nextElemId=mAggregateElems.getNextId(elemId);
				mAggregateElems.freeElem(elemId);
				PX_ASSERT(PX_INVALID_BP_HANDLE==mAggregateElems.getNextId(elemId));
				elemId=nextElemId;
			}

			//Now clear the aggregate itself.
			//Don't reuse the aggregate yet because we only do this in deleteAggregate.
			mAggregateManager.clearAggregate(aggregateId);

			//NOTE: Don't free the actor ids until the aggregate is removed because
			//		actors persist without shapes.
			//NOTE: If the bp elem id was added and immediately removed then we can reuse the id immediately.
			if(reuseBpElemId)
			{
				mBPElems.freeElem(bpElemId);
			}
		}
	}

	return removingLastShape;
}

void PxsAABBManager::setBPElemVolumeBounds(const PxcBpHandle singleID, const IntegerAABB& iaabb)
{
	mBPElems.setAABB(singleID,iaabb);

	if(!mBPCreatedElems.isInList(singleID) && !mBPUpdatedElems.isInList(singleID))
	{
		mBPUpdatedElems.addElem(singleID);
	}
}

void PxsAABBManager::setAggregateElemVolumeBounds(const PxcBpHandle elemId, const IntegerAABB& iaabb)
{
	mAggregateElems.setAABB(elemId,iaabb);

	const PxcBpHandle aggregateId = mAggregateActorManager.getAggregateId(mAggregateElems.getAggregateActorId(elemId));
	if(!mAggregatesUpdated.isInList(aggregateId))
	{
		mAggregatesUpdated.addElem(aggregateId);
	}
}

void PxsAABBManager::setVolumeBounds(const PxcBpHandle shapeHandle, const PxBounds3& bounds)
{
	IntegerAABB iaabb;
	iaabb.encode(bounds);

	if(!isClientVolumeAggregate(shapeHandle))
	{
		const PxcBpHandle singleID = decodeSingleFromClient(shapeHandle);
		setBPElemVolumeBounds(singleID, iaabb);
	}
	else
	{
		const PxcBpHandle elemId = decodeAggregateFromClient(shapeHandle);
		setAggregateElemVolumeBounds(elemId, iaabb);
	}
}

void PxsAABBManager::setActorDirty(const PxcBpHandle actorHandle)
{
	if(isClientVolumeAggregate(actorHandle))
	{
		const PxcBpHandle actorId = decodeAggregateFromClient(actorHandle);
		const PxcBpHandle aggregateId = mAggregateActorManager.getAggregateId(actorId);
		const Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);
		//Set the actor dirty but only if the aggregate has no self-collision
		//(we don't care about the actor bounds if the actor has self-collision
		//because we compute aggregate bounds from shape bounds)
		if(!aggregate->selfCollide) mAggregateActorManager.setDirty(actorId);
	}
}

PxBounds3 PxsAABBManager::getBPBounds(const PxcBpHandle shapeHandle) const
{
	PxBounds3 ret;
	const IntegerAABB iaabb = mBPElems.getAABB(shapeHandle);
	iaabb.decode(ret);
	return ret;
}


///////////////////////////////////////////////////////////////////////////////

// The goal here is to update the list of overlapping shape pairs.
// 
// Each shape is owned by an actor, and each actor may be in an aggregate. 
// Each broad phase entry corresponds to either a shape or an aggregate. 
// 
// We say an aggregate is *lazy* for this frame if it has no self-collision (SC) and was not involved in a broad phase overlap
// last frame. All other aggregates are *eager*
//
// Since the output of this function is a list of shape pairs, we may ultimately need to calculate the bounds of all shapes.
// Shape bounds are stored persistently.
// 
// The key optimization is to defer update of shape bounds of lazy aggregates (think: spaceships.) To do this we persistently store
// local space bounds for each actor in each no-SC aggregate. We assume the local-space bounds rarely change. The typical use-case 
// is an aggregate containing one or a few actors each having many shapes.
//
// This function 
// * updates broad phase bounds that have changed since last frame
// * runs the broad phase to find overlaps between pairs of shapes and/or aggregates
// * reduces all overlaps to overlaps between shape pairs. 

//The update order is as follows:

//1.  Iterate over all aggregate actors that have been marked as dirty (removed/added shape, modified shape local pose, 
//    modified shape geometry).
//    * add each affected aggregate to a list. Mark it as dirty and its corresponding broadphase entry as dirty.
//    
//    Implemented by computeAABBUpdateLists. No parallelisation is employed.

//2.  Iterate over all actors that have a new transform.
//    * for each actor in an aggregate, mark the aggregate and corresponding broadphase entry as dirty.
//    * for each actor not in an aggregate, mark each shape and its corresponding broadphase entry as dirty.
//    
//    Implemented by computeAABBUpdateLists. No parallelisation is employed.

//3.  For each aggregate with dirty actors (see 1),
//    * update each dirty actor with new local-space bounds (= the union of the transformed local space bounds of all its shapes).
//    
//    Implemented by ActorAABBTask.  On all platforms this may be performed in parallel with multiple tasks.
//    On ps3 this is performed on ppu only.

//4.  Update the world-space bounds for shapes whose actors have moved or which have been freshly added (see 2).
//	  * for shapes not in aggregates, update the bounds.
//	  * for shapes in eager aggregates, update the bounds.
//
//    Implemented by SingleAABBTask.  On ps3 only shapes of dynamic actors are updated in parallel on spu.  Shapes of static actors are updated on ppu.  
//    (The assumption is that shapes of static actors rarely require a bounds update.)
//    On all other platforms all shapes are updated in parallel with multiple tasks.

//4.  Update the world-space bounds of all non-aggregate shapes whose actors have moved or which have been freshly added (see 2).
//     Update the world-space bounds of all aggregate shapes for active aggregates

//5.  Update the world-space bounds of all aggregates that are dirty (see 1) or have been freshly added or have an actor that has moved (see 2).
//    
//     a) For eager aggregates, compute the union of the world-space bounds of each shape in the aggregate
//     b) For lazy aggregates, compute the union of the transformed local space bounds of each actor in the aggregate
// 
//     In case (b), if an actor has only a single shape then the world space bounds of the actor are equal to the world space bounds of the shape.
//    So we save the shape bounds here in case we need them after broad phase.
//
//     Implemented by AggregateAABBTask. On ps3 this is performed on ppu only. On all platforms this is performed in parallel with multiple tasks.

//6.  Run the broadphase with the updated world-space bounds of all broadphase entries (see 4 for single shape bp entries, see 5 for aggregate bp entries).
//    See implementation of chosen broadphase method for more details of parallelism.
//
//    Implemented by BPWorkTask

//7.  Process the results of the broadphase.  
//    * process any deleted/created pair involving single shapes immediately.  
//    * process any deleted pair involving a shape in an aggregate immediately
//    * add created pairs involving an aggregate to a list for later processing.
//    
//    Implemented by ProcessBPResultsTask. No parallelisation is employed.

//8.  Gather a list of all individual aggregate shapes that need updated.
//    Mark shapes which need to be updated as dirty. These are:
//    * shapes in lazy aggregates that appear in the overlap list (so the aggregate will be eager next frame)
//    * where the shape is part of a multi-shape actor (so that the bounds were not computed and stored in 5)
// 
//    Implemented by ProcessBPResultsTask. No parallelisation is employed.

//9.  Update the world-space bounds of all dirty shapes (see 8)
//    
//    Implemented by SingleAABBTask. The same platform & parallelism notes apply here as step 4

//10. Perform shape overlaps for all aggregate-single overlaps, aggregate-aggregate overlaps and aggregate self-collision.
//    
//    Implemented by AggregateOverlapTask. No parallelisation is employed.

//11. Release any allocated memory that is no longer required.
//    
//    Implemented by AggregateOverlapTask  (ok, this doesn't belong in there but lets keep the number of tasks to a minimum).

//12. Reset the bitmap of eager aggregates.
//    
//    Implemented by AggregateOverlapTask. No parallelisation is employed.

void PxsAABBManager::updateAABBsAndBP
(const PxU32 numCpuTasks, const PxU32 numSpusAABB, const PxU32 numSpusBP, 
 PxBaseTask* continuation,
 PxU32* PX_RESTRICT changedActorWords, const PxU32 changedActorWordCount, 						
 const bool secondBroadPhase, PxI32* numFastMovingShapes)
{
#ifdef PX_PROFILE
	Cm::EventProfiler* profiler = &mEventProfiler;
	CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::GetcomputeAABBUpdateLists());
#ifdef PX_PS3
	startTimerMarker(eCOMPUTE_AABB_UPDATE_LISTS);
#endif
#endif

	const PxsComputeAABBParams params = {numCpuTasks, numSpusAABB, numSpusBP, secondBroadPhase, numFastMovingShapes};

	//Resize arrays of known size.
	mBPUpdatedElemIds.resize(mBPElems.getCapacity());
	mBPUpdatedElemIdsSize = 0;
	mAggregateUpdatedElemIds.resize(mAggregateElems.getCapacity());
	mAggregateUpdatedElemIdsSize = 0;
	mAggregateActorShapeIds.resize(mAggregateActorManager.getCapacity());
	mDirtyActorAggregates.resize(mAggregateManager.getAggregatesCapacity());
	mNumDirtyActorAggregates = 0;
#ifdef PX_PS3
	mBPUpdatedElemWordCount = ((mBPElems.getCapacity() + 31) & ~31) >> 5;
	mBPUpdatedElemWordStarts.resize(mBPUpdatedElemWordCount);
	mBPUpdatedElemWordEnds.resize(mBPUpdatedElemWordCount);
	mAggregateUpdatedElemWordCount = ((mAggregateElems.getCapacity() + 31) & ~31) >> 5;
	mAggregateUpdatedElemWordStarts.resize(mAggregateUpdatedElemWordCount);
	mAggregateUpdatedElemWordEnds.resize(mAggregateUpdatedElemWordCount);
#endif

	//Resize arrays of unknown size to a default size
	mCreatedPairs = (PxvBroadPhaseOverlap*)mScratchAllocator.alloc(sizeof(PxvBroadPhaseOverlap)*DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY, true);
	mDeletedPairs = (PxvBroadPhaseOverlap*)mScratchAllocator.alloc(sizeof(PxvBroadPhaseOverlap)*DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY, true);

	//Need to compute the update lists before we release the handles.
	computeAABBUpdateLists(changedActorWords, changedActorWordCount, params);				

	//Remove all actors with zero shapes.
	removeEmptyActorsFromAggregates();
	mAggregateActorManager.resetEmptyBitmap();

	//The removed ids can be reused now.
	const PxcBpHandle* removedHandles=mBPRemovedElems.getElems();
	const PxU32 numRemovedHandles=mBPRemovedElems.getElemsSize();
	for(PxU32 i=0;i<numRemovedHandles;i++)
	{
		mBPElems.freeElem(removedHandles[i]);
	}

#ifdef PX_PROFILE
	CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::GetcomputeAABBUpdateLists());
#ifdef PX_PS3
	stopTimerMarker(eCOMPUTE_AABB_UPDATE_LISTS);
#endif
#endif

	//Set up the task chain.

	mSingleShapeAABBTask.setUpdateType(SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_SINGLE | SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_AGGREGATE);
	mAggregateShapeAABBTask.setUpdateType(SingleAABBUpdateTask::eUPDATE_TYPE_SHAPE_AGGREGATE);

	mFinishTask.setAABBMgr(this);
	mAggregateOverlapTask.setAABBMgr(this);
	mAggregateShapeAABBTask.setAABBMgr(this);
	mProcessBPResultsTask.setAABBMgr(this);
	mBPWorkTask.setAABBMgr(this);
	mAggregateAABBTask.setAABBMgr(this);
	mSingleShapeAABBTask.setAABBMgr(this);
	mActorAABBTask.setAABBMgr(this);

	mFinishTask.set(params);
	mAggregateOverlapTask.set(params);
	mAggregateShapeAABBTask.set(params);
	mProcessBPResultsTask.set(params);
	mBPWorkTask.set(params);
	mAggregateAABBTask.set(params);
	mSingleShapeAABBTask.set(params);
	mActorAABBTask.set(params);

	mFinishTask.setContinuation(continuation);
	mAggregateOverlapTask.setContinuation(&mFinishTask);
	mAggregateShapeAABBTask.setContinuation(&mAggregateOverlapTask);
	mProcessBPResultsTask.setContinuation(&mAggregateShapeAABBTask);
	mBPWorkTask.setContinuation(&mProcessBPResultsTask);
	mAggregateAABBTask.setContinuation(&mBPWorkTask);
	mSingleShapeAABBTask.setContinuation(&mAggregateAABBTask);
	mActorAABBTask.setContinuation(&mSingleShapeAABBTask);

	mFinishTask.removeReference();
	mAggregateOverlapTask.removeReference();
	mAggregateShapeAABBTask.removeReference();
	mProcessBPResultsTask.removeReference();
	mBPWorkTask.removeReference();
	mAggregateAABBTask.removeReference();
	mSingleShapeAABBTask.removeReference();
	mActorAABBTask.removeReference();
}

void PxsAABBManager::removeEmptyActorsFromAggregates()
{
	//Remove all empty actors (actors with zero shapes)
	const Cm::BitMap& emptyAggregateActors = mAggregateActorManager.getEmptyBitMap();
	if(emptyAggregateActors.getWords())
	{
		//Bitmap of aggregates with actors with zero shapes.  Compute this from all actors with zero shapes.
		InlineBitmap<MAX_NUM_BP_SPU_SAP_AABB> aggregatesWithEmptyActors(mAggregateManager.getAggregatesCapacity(), &mScratchAllocator);
		{
			Cm::BitMap::Iterator it(emptyAggregateActors);
			PxU32 index = it.getNext();
			while(index != Cm::BitMap::Iterator::DONE)
			{
				//Get the actor id.
				const PxcBpHandle actorId = (PxcBpHandle)index;
				PX_ASSERT(!mAggregateActorManager.getShapeCount(actorId));

				//Get the aggregate id
				const PxcBpHandle aggregateId = mAggregateActorManager.getAggregateId(actorId);

				//Set the bitmap.
				aggregatesWithEmptyActors.set(aggregateId);

				//Ready for the next.
				index = it.getNext();
			}//index
		}

		//Iterate over all aggregates with actors with zero shapes.
		//Remove all empty actors from the actor list for each aggregate.
		//Free the actor ids so they may be reused.
		{
			Cm::BitMap::Iterator it(aggregatesWithEmptyActors.mBitmap);
			PxU32 index = it.getNext();
			while(index != Cm::BitMap::Iterator::DONE)
			{
				const PxcBpHandle aggregateId = (PxcBpHandle)index;
				Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);

				//Remove all actors with zero shapes.
				PxcBpHandle actorId = aggregate->actorHeadID;
				while((actorId != PX_INVALID_BP_HANDLE) && (0==mAggregateActorManager.getShapeCount(actorId)))
				{
					const PxcBpHandle nextActorId = mAggregateActorManager.getNextId(actorId);
					mAggregateActorManager.freeElem(actorId);
					actorId = nextActorId;
				}
				aggregate->actorHeadID = actorId;
				PX_ASSERT((PX_INVALID_BP_HANDLE == actorId) || (mAggregateActorManager.getShapeCount(actorId) > 0));
				while(actorId != PX_INVALID_BP_HANDLE)
				{
					PxcBpHandle nextActorId = mAggregateActorManager.getNextId(actorId);
					while(nextActorId != PX_INVALID_BP_HANDLE && 0==mAggregateActorManager.getShapeCount(nextActorId))
					{
						const PxcBpHandle nextNextActorId = mAggregateActorManager.getNextId(nextActorId);
						mAggregateActorManager.freeElem(nextActorId);
						nextActorId = nextNextActorId;
					}
					PX_ASSERT((PX_INVALID_BP_HANDLE == nextActorId) || (mAggregateActorManager.getShapeCount(nextActorId) > 0));
					mAggregateActorManager.setNextId(actorId, nextActorId);
					actorId = nextActorId;
				}

				//Ready for the next.
				index = it.getNext();
			}//index
		}
	}
}

#ifdef PX_PS3
PxU32 computeUpdatedElemWords
(const PxsComputeAABBParams& params,
 const Elems& elems, const Cm::BitMap& updatedElemsBitmap,
 PxcBpHandle* updatedElemIds, IntegerAABB* iaabbs, PxcBpHandle* updatedElemWordStarts, PxcBpHandle* updatedElemWordEnds)
{
	PX_UNUSED(params);
	const PxcBpHandle* PX_RESTRICT groupIds=elems.getGroups();

	PxU32 updatedElemIdsSize = 0;
	for(PxU32 w = 0; w <= updatedElemsBitmap.findLast() >> 5; ++w)
	{
		updatedElemWordStarts[w] = updatedElemIdsSize;
		for(PxU32 b = updatedElemsBitmap.getWords()[w]; b; b &= b-1)
		{
			const PxU32 index = (w<<5|Ps::lowestSetBit(b));
			if(0!=groupIds[index])
			{
				updatedElemIds[updatedElemIdsSize]=index;
				updatedElemIdsSize++;
			}
			else
			{
				PxBounds3 bounds;
				const PxcAABBDataStatic& staticAABBData=elems.getStaticAABBData(index);
				PxsComputeAABB(staticAABBData, bounds);
				IntegerAABB iaabb;
				iaabb.encode(bounds);
				iaabbs[index] = iaabb;
			}
		}
		updatedElemWordEnds[w] = updatedElemIdsSize;
	}

	return updatedElemIdsSize;
}
#endif

void PxsAABBManager::computeAABBUpdateLists(PxU32* PX_RESTRICT changedActorWords, const PxU32 changedActorWordCount, const PxsComputeAABBParams& params)
{
	PX_ASSERT(changedActorWords);

#ifndef PX_PS3
	PX_UNUSED(params);
#endif

	//Compute a list of all aggregates with actors marked dirty from 
	//(i) shape insertion, (ii) shape removal (iii) local pose change (iv) geometry change
	//This only affects aggregates without self-collision.
	//[NOTE: 
	//1. Aggregates without self-collision and no record of external collision will have an aggregate bound computed from the 
	//union of actor bounds.  As a consequence, the actor local-space bounds need updated.
	//2. Aggregates without self-collision but with a record of external collision will have the aggregate bound computed from the 
	//union of shape bounds.  Despite this, the actor local-space bounds still need updated because in future updates there may be no 
	//record of external collision.  To anticipate this we update the actor local-space bounds now even though they aren't used this
	//frame.
	//3.  Aggregates with self-collision are always computed from the union of shape bounds.  The actor local-space bounds are 
	//not required now or in the future.
	//]
	{
		const Cm::BitMap& dirtyAggregateActors = mAggregateActorManager.getDirtyBitmap();
		PX_ASSERT(!mNumDirtyActorAggregates);
		if(dirtyAggregateActors.getWords())
		{
			//Record a list of aggregates that have a dirty actor.
			InlineBitmap<1024> dirtyActorAggregatesBitmap(mAggregateManager.getAggregatesCapacity(), &mScratchAllocator);
			PxcBpHandle* PX_RESTRICT dirtyActorAggregates = mDirtyActorAggregates.mBuffer;
			PxU32 NumDirtyActorAggregates = 0;

			Cm::BitMap::Iterator it(dirtyAggregateActors);
			PxU32 index = it.getNext();
			while(index != Cm::BitMap::Iterator::DONE)
			{
				//Navigate from the actor id to the aggregate.
				const PxcBpHandle actorId = (PxcBpHandle)index;
				const PxcBpHandle aggregateId = mAggregateActorManager.getAggregateId(actorId);
				const Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);

				//Only need to worry about aggregates with shapes. Aggregates with no 
				//shapes do not have a broadphase entry.
				if(aggregate->nbElems>0)
				{	
					//We should never get here with self-colliding aggregates.
					PX_ASSERT(!aggregate->selfCollide);

					//Add the aggregate to the list of aggregates that have a dirty actor.
					if(!dirtyActorAggregatesBitmap.test(aggregateId))
					{
						dirtyActorAggregatesBitmap.set(aggregateId);
						dirtyActorAggregates[NumDirtyActorAggregates] = aggregateId;
						NumDirtyActorAggregates++;
					}

					//The aggregate has changed so mark it as having changed.
					//Add to the list of aggregates that need updated.
					if(!mAggregatesUpdated.isInList(aggregateId))
					{
						mAggregatesUpdated.addElem(aggregateId);
					}

					//The bp entry needs updated too.
					//Add to the list of bp elems that will have been updated after we've updated the aggregates.
					const PxcBpHandle bpElemId = aggregate->bpElemId;
					if(!mBPCreatedElems.isInList(bpElemId) && !mBPUpdatedElems.isInList(bpElemId))
					{
						mBPUpdatedElems.addElem(bpElemId);
					}
				}

				//Ready for the next.
				index = it.getNext();
			}//index

			mNumDirtyActorAggregates = NumDirtyActorAggregates;
		}
	}

	//Compute a list of single shapes that need updated before the broadphase due to updated actor transforms.
	//Mark all affected aggregates that need updated before the broadphase due to updated actor transforms.
	{
		//Gather lists of all bp elems that need updated and of all aggregates that need updated.
		PxU32 BPUpdatedElemIdsSize = 0;
		PxcBpHandle* PX_RESTRICT BPUpdatedElemIds = mBPUpdatedElemIds.mBuffer;
		Cm::BitMap changedActorsBitmap;
		changedActorsBitmap.setWords(changedActorWords, changedActorWordCount);
		Cm::BitMap::Iterator it(changedActorsBitmap);
		PxU32 index = it.getNext();
		while(index != Cm::BitMap::Iterator::DONE)
		{
			const PxcBpHandle actorHandle = (PxcBpHandle)index;
			if (actorHandle == PX_INVALID_BP_HANDLE)
				continue;

			if(!isClientVolumeAggregate(actorHandle))
			{
				const PxcBpHandle singleId=decodeSingleFromClient(actorHandle);
				const Single* single=mSingleManager.getSingle(singleId);
				const PxcBpHandle headId=single->headID;
				PxcBpHandle id=headId;
				while(id!=PX_INVALID_BP_HANDLE)
				{
					BPUpdatedElemIds[BPUpdatedElemIdsSize]=id;
					BPUpdatedElemIdsSize++;
					if(!mBPCreatedElems.isInList(id) && !mBPUpdatedElems.isInList(id))
					{
						mBPUpdatedElems.addElem(id);
					}
					id=mBPElems.getNextId(id);
				}
			}
			else
			{
				//Get the aggregate id.
				const PxcBpHandle aggregateActorId=decodeAggregateFromClient(actorHandle);
				PX_ASSERT(aggregateActorId != PX_INVALID_BP_HANDLE);
				const PxcBpHandle aggregateId=mAggregateActorManager.getAggregateId(aggregateActorId);
				PX_ASSERT(aggregateId != PX_INVALID_BP_HANDLE);

				//Only consider aggregates that have elements.
				Aggregate* aggregate=mAggregateManager.getAggregate(aggregateId);
				if(aggregate->nbElems>0)
				{	
					if(!mAggregatesUpdated.isInList(aggregateId))
					{
						mAggregatesUpdated.addElem(aggregateId);
					}

					//Add to the list of bp elems that will have been updated after we've updated the aggregates.
					const PxcBpHandle bpElemId = aggregate->bpElemId;
					if(!mBPCreatedElems.isInList(bpElemId) && !mBPUpdatedElems.isInList(bpElemId))
					{
						mBPUpdatedElems.addElem(bpElemId);
					}
				}
			}
			index = it.getNext();
		}//index
		mBPUpdatedElemIdsSize = BPUpdatedElemIdsSize;

#ifdef PX_PS3
		//Extra sorting on ps3 to help parallelize on spu.
		{
			InlineBitmap<MAX_NUM_BP_SPU_SAP_AABB> updatedElemsBitmap(mBPElems.getCapacity(), &mScratchAllocator);
			for(PxU32 i = 0; i < BPUpdatedElemIdsSize; i++)
			{
				updatedElemsBitmap.set(BPUpdatedElemIds[i]);
			}
			mBPUpdatedElemIdsSize = computeUpdatedElemWords(
				params, mBPElems, updatedElemsBitmap.mBitmap, 
				BPUpdatedElemIds, mBPElems.getBounds(), mBPUpdatedElemWordStarts.mBuffer, mBPUpdatedElemWordEnds.mBuffer);
		}
#endif //PX_PS3
	}

	//Iterate over all updated aggregates and compile a list of all shapes that need their bounds updated.
	//1. If an aggregate is self-colliding then we always need to update the individual shape bounds
	//because they are needed for aggregate self-collision.  It makes sense to compute the shape bounds
	//and compute the aggregate bounds from the union of all shape bounds.
	//2. If an aggregate is not self-colliding but has a record of broadphase overlaps then we assume that 
	//it will remain in broadphase overlap. This being the case, we will need the individual shape bounds
	//to perform aggregate-aggregate or aggregate-single overlaps.  With this assumption, it makes sense to 
	//compute the shape bounds and compute the aggregate bounds from the union of all shape bounds.
	//3.  If an aggregate is not self-colliding and has no record of broadphase overlaps then we assume 
	//that it will remain without broadphase overlap.  This being the case, we don't need the individual 
	//shape bounds and can more optimally compute the broadphase bounds from the actor bounds.
	{
		mAggregatesUpdated.computeList();

		PxU32 AggregateUpdatedElemIdsSize = 0;
		PxcBpHandle* AggregateUpdatedElemIds = mAggregateUpdatedElemIds.mBuffer;

		const PxU32 numUpdatedAggregates = mAggregatesUpdated.getElemsSize();
		const PxcBpHandle* updatedAggregates = mAggregatesUpdated.getElems();

		for(PxU32 k = 0; k < numUpdatedAggregates; k++)
		{
			const PxcBpHandle aggregateId = updatedAggregates[k];
			const Aggregate* aggregate= mAggregateManager.getAggregate(aggregateId);
			if(aggregate->selfCollide || mAggregateManager.getHasBroadphaseOverlap(aggregateId))
			{
				PxcBpHandle elemId = aggregate->elemHeadID;
				while(elemId != PX_INVALID_BP_HANDLE)
				{
					if(mAggregateElems.getGroup(elemId) != PX_INVALID_BP_HANDLE)
					{
						AggregateUpdatedElemIds[AggregateUpdatedElemIdsSize] = elemId;
						AggregateUpdatedElemIdsSize++;
					}
					elemId = mAggregateElems.getNextId(elemId);
				}

				//Add to the list of bp elems that will have been updated after we've updated the aggregates.
				const PxcBpHandle bpElemId = aggregate->bpElemId;
				if(!mBPCreatedElems.isInList(bpElemId) && !mBPUpdatedElems.isInList(bpElemId))
				{
					mBPUpdatedElems.addElem(bpElemId);
				}
			}
		}
		mAggregateUpdatedElemIdsSize = AggregateUpdatedElemIdsSize;


#ifdef PX_PS3
		{
			InlineBitmap<MAX_NUM_BP_SPU_SAP_AABB> updatedElemsBitmap(mAggregateElems.getCapacity(), &mScratchAllocator);
			for(PxU32 i = 0; i < AggregateUpdatedElemIdsSize; i++)
			{
				updatedElemsBitmap.set(AggregateUpdatedElemIds[i]);
			}

			mAggregateUpdatedElemIdsSize = computeUpdatedElemWords(
				params, mAggregateElems, updatedElemsBitmap.mBitmap, 
				AggregateUpdatedElemIds, mAggregateElems.getBounds(), mAggregateUpdatedElemWordStarts.mBuffer, mAggregateUpdatedElemWordEnds.mBuffer);

		}
#endif //PX_PS3
	}

	mBPUpdatedElems.computeList();
	mBPCreatedElems.computeList();
	mBPRemovedElems.computeList();
}


void PxsAABBManager::prepareBP()
{
	mSingleShapeAABBTask.updateNumFastMovingShapes();
	mAggregateAABBTask.updateNumFastMovingShapes();
}

void PxsAABBManager::processBPResults(const PxsComputeAABBParams& params)
{
#ifndef PX_PS3
	PX_UNUSED(params);
#endif

	// Handle out-of-bounds objects
	{
		PxU32 nb = mBP->getNumOutOfBoundsObjects();
		const PxU32* PX_RESTRICT handles = mBP->getOutOfBoundsObjects();
		while(nb--)
		{
			const PxcBpHandle h = (PxcBpHandle)(*handles++);
			if(mBPElems.isValid(h))	// PT: can return false when object has been removed from scene
			{
				if(mBPElems.isSingle(h))
				{
					void* userData = mBPElems.getUserData(h);
					mOutOfBoundsObjects.pushBack(userData);
				}
				else
				{
					Aggregate* aggregate = mAggregateManager.getAggregate(mBPElems.getAggregateOwnerId(h));
					mOutOfBoundsAggregates.pushBack(aggregate->userData);
				}
			}
		}
	}

	// Reset the number of created and deleted pairs.
	mCreatedPairsSize = 0;
	mDeletedPairsSize = 0;

	// Now for the main chunk of work: take all the pairs reported by the bp 
	// and add all the pairs from single-single single-aggregate and aggregate-aggregate overlaps.

	const PxU32 nbCreatedPairs = mBP->getNumCreatedPairs();
	const PxU32 nbDeletedPairs = mBP->getNumDeletedPairs();
	PxcBroadPhasePair* createdPairs = mBP->getCreatedPairs();
	PxcBroadPhasePair* deletedPairs = mBP->getDeletedPairs();

	// Directly add all freshly created single-single pairs.
	// Add all created single-aggregate or aggregate-aggregate pairs to the bp created pair array.
	PxU32 numCreatedAggregatePairs = 0;
	PxcBroadPhasePair* createdAggregatePairs = createdPairs;
	for(PxU32 i=0; i<nbCreatedPairs; i++)
	{
		// Get the bp elem ids.
		const PxcBpHandle volA = createdPairs[i].mVolA;
		const PxcBpHandle volB = createdPairs[i].mVolB;

		// Work out if the bp elems are singles or aggregates.
		const bool s0 = mBPElems.isSingle(volA);
		const bool s1 = mBPElems.isSingle(volB);

		// If both are singles then we can add them as a created pair immediately.
		// If either is a aggregate then store in an array for later use cos we need to consider the aggregate elems.
		if(s0 && s1)
		{
			addCreatedPair
				(&mScratchAllocator, 
				 mBPElems.getUserData(volA), mBPElems.getUserData(volB), 
				 mCreatedPairs, mCreatedPairsSize, mCreatedPairsCapacity);
		}
		else 
		{
			PX_ASSERT(numCreatedAggregatePairs<=i);
			createdAggregatePairs[numCreatedAggregatePairs].mVolA = volA;
			createdAggregatePairs[numCreatedAggregatePairs].mVolB = volB;
			numCreatedAggregatePairs++;
		}
	}

	// Directly add all freshly deleted single-single pairs.
	// Add all deleted single-aggregate or aggregate-aggregate pairs to the bp deleted pair array.
	PxU32 numDeletedAggregatePairs = 0;
	PxcBroadPhasePair* deletedAggregatePairs = deletedPairs;
	for(PxU32 i=0;i<nbDeletedPairs;i++)
	{
		// Get the bp elem ids.
		const PxcBpHandle volA = deletedPairs[i].mVolA;
		const PxcBpHandle volB = deletedPairs[i].mVolB;

		// Work out if the bp elems are singles or aggregates.
		const bool s0 = mBPElems.isSingle(volA);
		const bool s1 = mBPElems.isSingle(volB);

		// If both are singles then we can add them as a deleted pair immediately.
		// If either is a aggregate then store in an array for later use cos we need to consider the aggregate elems.
		if(s0 && s1)
		{
			addDeletedPair
				(&mScratchAllocator, 
				 mBPElems.getUserData(volA), mBPElems.getUserData(volB), 
				 mDeletedPairs, mDeletedPairsSize, mDeletedPairsCapacity);		
		}
		else
		{
			PX_ASSERT(numDeletedAggregatePairs<=i);
			deletedAggregatePairs[numDeletedAggregatePairs].mVolA = volA;
			deletedAggregatePairs[numDeletedAggregatePairs].mVolB = volB;
			numDeletedAggregatePairs++;
		}
	}

	// Handle the deleted single-aggregate and aggregate-aggregate pairs.
	// Do this before considering the added pairs to avoid unnecessarily growing the arrays.
	for(PxU32 i=0;i<numDeletedAggregatePairs;i++)
	{
		//Get the bp elem ids.
		const PxcBpHandle volA = deletedAggregatePairs[i].mVolA;
		const PxcBpHandle volB = deletedAggregatePairs[i].mVolB;
		removeAggregatePair(volA, volB);
	}

	// Now add the freshly created single-aggregate and aggregate-aggregate pairs
	// to a special array of aggregate pairs.
	for(PxU32 i=0;i<numCreatedAggregatePairs;i++)
	{
		//Get the bp elem ids.
		const PxcBpHandle volA = createdAggregatePairs[i].mVolA;
		const PxcBpHandle volB = createdAggregatePairs[i].mVolB;
		addAggregatePair(volA, volB);
	}

	//Compile a list of aggregate shapes that need their bounds updated.
	//Compile a list of aggregates that need their shapes sorted.
	PX_ASSERT(!mNumAggregatesToSort);
	PX_ASSERT(!mNumAggregatePairsToOverlap);
	if(mAggregateManager.getAggregatesCapacity() > 0)
	{
		//Bitmap of aggregates that need their shapes updated.
		InlineBitmap<MAX_NUM_BP_SPU_SAP_AABB> aggregatesNeedingShapesUpdated(mAggregateManager.getAggregatesCapacity(), &mScratchAllocator);
		aggregatesNeedingShapesUpdated.resetAll();

		//Bitmap of aggregates that need their bounds sorted prior to aggregate-aggregate overlaps.
		InlineBitmap<MAX_NUM_BP_SPU_SAP_AABB> aggregatesNeedingSorted(mAggregateManager.getAggregatesCapacity(), &mScratchAllocator);
		aggregatesNeedingSorted.resetAll();
		PxU32 NumAggregatesToSort = 0;

		//Array of all pairs that need overlaps recomputed.
		mAggregatePairsToOverlap.resize(mAggregatePairsSize);
		PxcBpHandle* AggregatePairsToOverlap = mAggregatePairsToOverlap.mBuffer;
		PxU32 NumAggregatePairsToOverlap = 0;

		//Compute all shapes that need updated.
		//Compute all aggregate pairs that need overlaps recomputed
		//Compute aggregates that need sorted for overlap re-computation.
		{
			//Add all updated aggregates with self-collision to the list of aggregates needing sorted shape bounds.
			//All updated aggregates whose bounds were computed from the union of actor bounds are marked as dirty 
			//because their shape bounds are not in sync with the actor transforms.  We only need to update individual shape
			//bounds for aggregates that are dirty and have a bp overlap.
			{
				const PxU32 N = mAggregatesUpdated.getElemsSize();
				const PxcBpHandle* updatedAggregates = mAggregatesUpdated.getElems();
				for(PxU32 i = 0; i < N; i++)
				{
					const PxcBpHandle aggregateId = updatedAggregates[i];
					const Aggregate& aggregate = *mAggregateManager.getAggregate(aggregateId);
					if(aggregate.performSelfCollision())
					{
						aggregatesNeedingSorted.set(aggregateId);
						NumAggregatesToSort++;
					}
					else if(!mAggregateManager.getHasBroadphaseOverlap(aggregateId) && aggregate.nbActive > 0)
					{
						PX_ASSERT(!aggregate.selfCollide);
						mAggregateManager.setDirty(aggregateId);
					}
				}
			}

			//Add all dirty aggregates with bp overlaps to the list of aggregates needing updated shape bounds.
			//Aggregates with self-collision will already be in the list (see above).
			//Aggregates without self-collision that are dirty need to be in the list if any bp overlap pair 
			//featuring a dirty aggregate has an updated bp aabb.
			{
				const PxU32 N = mAggregatePairsSize;
				for(PxU32 i = 0; i < N; i++)
				{
					const AggregatePair& cp = mAggregatePairs[i];
					const PxcBpHandle bpElemId0 = cp.mBPElemId0;
					const PxcBpHandle bpElemId1 = cp.mBPElemId1;

					//If neither has been updated we can defer updating the individual shapes until one of them does move.
					bool eitherUpdated = false;
					if(mBPElems.isSingle(bpElemId0) && (mBPUpdatedElems.isInList(bpElemId0) || mBPCreatedElems.isInList(bpElemId0)))
					{
						eitherUpdated = true;
					}
					else if(!mBPElems.isSingle(bpElemId0) && (mAggregatesUpdated.isInList(mBPElems.getAggregateOwnerId(bpElemId0)) || mBPCreatedElems.isInList(bpElemId0)))
					{
						eitherUpdated = true;
					}
					if(mBPElems.isSingle(bpElemId1) && (mBPUpdatedElems.isInList(bpElemId1) || mBPCreatedElems.isInList(bpElemId1)))
					{
						eitherUpdated = true;
					}
					else if(!mBPElems.isSingle(bpElemId1) && (mAggregatesUpdated.isInList(mBPElems.getAggregateOwnerId(bpElemId1)) || mBPCreatedElems.isInList(bpElemId1)))
					{
						eitherUpdated = true;
					}

					if(eitherUpdated)
					{		
						PxcBpHandle aggregateId0 = PX_INVALID_BP_HANDLE;
						PxcBpHandle aggregateId1 = PX_INVALID_BP_HANDLE;

						//If it is an aggregate and has self-collision the shapes are guaranteed to be in sync with the actor transforms
						//because self-colliding aggregates update their shape bounds every time the aggregate is marked as updated.
						//If it is an aggregate and has no self-collision the shapes might not be in sync with the actor transforms because 
						//it might have had no overlaps when it was last updated.  Any dirty aggregates need updated.
						if(!mBPElems.isSingle(bpElemId0))
						{
							aggregateId0 = mBPElems.getAggregateOwnerId(bpElemId0);
							if(!aggregatesNeedingShapesUpdated.test(aggregateId0) && mAggregateManager.isDirty(aggregateId0))
							{
								mAggregateManager.setClean(aggregateId0);
								aggregatesNeedingShapesUpdated.set(aggregateId0);
							}
						}

						//If it is an aggregate and has self-collision the shapes are guaranteed to be in sync with the actor transforms
						//because self-colliding aggregates update their shape bounds every time the aggregate is marked as updated.
						//If it is an aggregate and has no self-collision the shapes might not be in sync with the actor transforms because 
						//it might have had no overlaps when it was last updated.  Any dirty aggregates need updated.
						if(!mBPElems.isSingle(bpElemId1))
						{
							aggregateId1 = mBPElems.getAggregateOwnerId(bpElemId1);
							if(!aggregatesNeedingShapesUpdated.test(aggregateId1) && mAggregateManager.isDirty(aggregateId1))
							{
								mAggregateManager.setClean(aggregateId1);
								aggregatesNeedingShapesUpdated.set(aggregateId1);
							}
						}

						//Sort any aggregates
						if(PX_INVALID_BP_HANDLE != aggregateId0 && !aggregatesNeedingSorted.test(aggregateId0))
						{
							aggregatesNeedingSorted.set(aggregateId0);
							NumAggregatesToSort++;
						}
						if(PX_INVALID_BP_HANDLE != aggregateId1 && !aggregatesNeedingSorted.test(aggregateId1))
						{
							aggregatesNeedingSorted.set(aggregateId1);
							NumAggregatesToSort++;
						}

						AggregatePairsToOverlap[NumAggregatePairsToOverlap] = (PxcBpHandle)i;
						NumAggregatePairsToOverlap++;
					}
				}
			}
		}
		mNumAggregatePairsToOverlap = NumAggregatePairsToOverlap;

		//Iterate over all aggregates that need sorted and store them in an array.
		mAggregatesToSort.resize(NumAggregatesToSort);
		PxcBpHandle* AggregatesToSort = mAggregatesToSort.mBuffer;
		{
			if(NumAggregatesToSort > 0)
			{
				//Record the aggregates that need sorted.
				NumAggregatesToSort = 0;
				Cm::BitMap::Iterator it(aggregatesNeedingSorted.mBitmap);
				PxU32 index = it.getNext();
				while(index != Cm::BitMap::Iterator::DONE)
				{
					AggregatesToSort[NumAggregatesToSort] = (PxcBpHandle)index;
					NumAggregatesToSort++;
					index = it.getNext();
				}
			}
		}
		mNumAggregatesToSort = NumAggregatesToSort;

		//Iterate over all aggregates in the bitmap and add all individual shapes to the updated elem list.
		{
			PxcBpHandle* PX_RESTRICT AggregateUpdatedElemIds = mAggregateUpdatedElemIds.mBuffer;
			PxU32 AggregateUpdatedElemIdsSize = 0;
			Cm::BitMap::Iterator it(aggregatesNeedingShapesUpdated.mBitmap);
			PxU32 index = it.getNext();
			while(index != Cm::BitMap::Iterator::DONE)
			{
				//Get the aggregate id.
				const PxcBpHandle aggregateId = (PxcBpHandle)index;

				//Get the aggregate
				const Aggregate& aggregate = *mAggregateManager.getAggregate(aggregateId);
				const Ps::IntBool isInAggregateUpdatedList = mAggregatesUpdated.isInList(aggregateId);

				if(!isInAggregateUpdatedList)
				{
					//Iterate over all shapes of the aggregate
					PxcBpHandle elemId = aggregate.elemHeadID;
					while(elemId != PX_INVALID_BP_HANDLE)
					{
						if(mAggregateElems.getGroup(elemId) != PX_INVALID_BP_HANDLE)
						{
							AggregateUpdatedElemIds[AggregateUpdatedElemIdsSize] = elemId;
							AggregateUpdatedElemIdsSize++;
						}
						elemId = mAggregateElems.getNextId(elemId);
					}
				}
				else
				{
					//Iterate over all shapes of the aggregate and discard those with a single shape.
					//Shapes of single-shape actors in aggregates had their bounds updated in AggregateAABBTask.
					PxcBpHandle elemId = aggregate.elemHeadID;
					while(elemId != PX_INVALID_BP_HANDLE)
					{
						if(mAggregateElems.getGroup(elemId) != PX_INVALID_BP_HANDLE)
						{
							const PxcBpHandle actorId = mAggregateElems.getAggregateActorId(elemId);
							const PxU8 shapeCount = mAggregateActorManager.getShapeCount(actorId);
							PX_ASSERT(shapeCount > 0);
							if(shapeCount > 1)
							{
								//Add shape to update list.
								AggregateUpdatedElemIds[AggregateUpdatedElemIdsSize] = elemId;
								AggregateUpdatedElemIdsSize++;
							}
						}
						elemId = mAggregateElems.getNextId(elemId);
					}
				}
				
				//Ready for the next.
				index = it.getNext();
			}//index
			mAggregateUpdatedElemIdsSize = AggregateUpdatedElemIdsSize;

#ifdef PX_PS3
			if(AggregateUpdatedElemIdsSize > 0)
			{
				InlineBitmap<MAX_NUM_BP_SPU_SAP_AABB> updatedElemsBitmap(mAggregateElems.getCapacity(), &mScratchAllocator);
				for(PxU32 i = 0; i < AggregateUpdatedElemIdsSize; i++)
				{
					updatedElemsBitmap.set(AggregateUpdatedElemIds[i]);
				}

				mAggregateUpdatedElemIdsSize = computeUpdatedElemWords(
					params, mAggregateElems, updatedElemsBitmap.mBitmap, 
					AggregateUpdatedElemIds, mAggregateElems.getBounds(), mAggregateUpdatedElemWordStarts.mBuffer, mAggregateUpdatedElemWordEnds.mBuffer);
			}
#endif
		}
	}
}

void PxsAABBManager::updateAggregateBPOverlapRecords()
{
	mAggregateManager.clearBroadPhaseOverlapBitMap();
	for(PxU32 i = 0; i < mAggregatePairsSize; i++)
	{
		const AggregatePair& pair = mAggregatePairs[i];
		if(mBPElems.isOwnerAggregate(pair.mBPElemId0))
		{
			const PxcBpHandle aggregateId = mBPElems.getAggregateOwnerId(pair.mBPElemId0);
			mAggregateManager.setBroadphaseOverlap(aggregateId);
		}
		if(mBPElems.isOwnerAggregate(pair.mBPElemId1))
		{
			const PxcBpHandle aggregateId = mBPElems.getAggregateOwnerId(pair.mBPElemId1);
			mAggregateManager.setBroadphaseOverlap(aggregateId);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

PxcBpHandle PxsAABBManager::createBPElem()
{
	PxcBpHandle elemId = PX_INVALID_BP_HANDLE;

	if(mBPElems.getFirstFreeElem() != PX_INVALID_BP_HANDLE)
	{
		elemId = mBPElems.useFirstFreeElem();
	}
	else
	{
		{
			const PxU32 newCapacity = mBPElems.getCapacity() ? mBPElems.getCapacity()*2 : 32;
			mBPElems.grow(newCapacity);

			//these array needs to track the number of bp elems
			mBPUpdatedElems.growBitmap(newCapacity);
			mBPCreatedElems.growBitmap(newCapacity);
			mBPRemovedElems.growBitmap(newCapacity);
		}

		PX_ASSERT(mBPElems.getFirstFreeElem()!=PX_INVALID_BP_HANDLE);
		elemId = mBPElems.useFirstFreeElem();
	}

	PX_ASSERT(elemId != PX_INVALID_BP_HANDLE);
	return elemId;
}

PxcBpHandle PxsAABBManager::createAggregateElem()
{
	PxcBpHandle elemId = PX_INVALID_BP_HANDLE;

	if(mAggregateElems.getFirstFreeElem() != PX_INVALID_BP_HANDLE)
	{
		elemId = mAggregateElems.useFirstFreeElem();
	}
	else
	{
		{
			const PxU32 newCapacity = mAggregateElems.getCapacity() ? mAggregateElems.getCapacity()*2 : 32;
			mAggregateElems.grow(newCapacity);
			mAggregatesUpdated.growBitmap(newCapacity);
		}

		PX_ASSERT(mAggregateElems.getFirstFreeElem()!=PX_INVALID_BP_HANDLE);
		elemId = mAggregateElems.useFirstFreeElem();
	}

	PX_ASSERT(elemId != PX_INVALID_BP_HANDLE);
	return elemId;
}

///////////////////////////////////////////////////////////////////////////////

AggregatePair& PxsAABBManager::addAggregatePair(const PxcBpHandle bpElemId0, const PxcBpHandle bpElemId1)
{
	if(mAggregatePairsSize == mAggregatePairsCapacity)
	{
		const PxU32 oldCapacity = mAggregatePairsCapacity;
		const PxU32 newCapacity = mAggregatePairsCapacity ? mAggregatePairsCapacity*2 : 32;
		mAggregatePairs = resizePODArray<AggregatePair>(oldCapacity, newCapacity, mAggregatePairs);
		mAggregatePairsCapacity = newCapacity;
	}

	AggregatePair& cp = mAggregatePairs[mAggregatePairsSize++];
	cp.mBPElemId0 = PxMin(bpElemId0, bpElemId1);
	cp.mBPElemId1 = PxMax(bpElemId0, bpElemId1);
	cp.aggregateCollBitmap	= NULL;

	return cp;
}

bool PxsAABBManager::removeAggregatePair(const PxcBpHandle vol0, const PxcBpHandle vol1)
{
	const PxcBpHandle bpElemId0 = PxMin(vol0,vol1);
	const PxcBpHandle bpElemId1 = PxMax(vol0,vol1);

#ifdef PX_DEBUG
	bool aggregateOutOfBounds = false;
	PxU32 nb = mBP->getNumOutOfBoundsObjects();
	const PxU32* PX_RESTRICT handles = mBP->getOutOfBoundsObjects();
	while(nb--)
	{
		const PxcBpHandle h = (PxcBpHandle)(*handles++);
		if(!mBPElems.isSingle(h))
		{
			if(h==bpElemId0 || h==bpElemId1)
				aggregateOutOfBounds = true;
		}
	}
#endif

	for(PxU32 i=0;i<mAggregatePairsSize;i++)
	{
		if(mAggregatePairs[i].mBPElemId0==bpElemId0 && mAggregatePairs[i].mBPElemId1==bpElemId1)
		{
			if(mAggregatePairs[i].aggregateCollBitmap)
			{
				PX_ASSERT(aggregateOutOfBounds || mBPRemovedElems.isInList(bpElemId0) || false == mBPElems.getAABB(bpElemId0).intersectsOrTouches(mBPElems.getAABB(bpElemId1)));

				// PT: the correct function could be looked up and stored inside "AggregatePair"...
				Aggregate* c0 = (mBPElems.isSingle(bpElemId0)) ? NULL : mAggregateManager.getAggregate(mBPElems.getAggregateOwnerId(bpElemId0));
				Aggregate* c1 = (mBPElems.isSingle(bpElemId1)) ? NULL : mAggregateManager.getAggregate(mBPElems.getAggregateOwnerId(bpElemId1));

				if(c0)
				{
					if(c1)
					{
						// aggregate-aggregate
						removeAggregateAggregatePair(c0, c1, mAggregatePairs[i].aggregateCollBitmap);
					}
					else
					{
						// aggregate-single
						removeAggregateSinglePair(bpElemId1, c0, mAggregatePairs[i].aggregateCollBitmap);
					}
				}
				else
				{
					if(c1)
					{
						// Single-aggregate
						removeAggregateSinglePair(bpElemId0, c1, mAggregatePairs[i].aggregateCollBitmap);
					}
					else
					{
						// Single-single, shouldn't happen
						PX_ASSERT(0);
					}
				}

				PX_DELETE(mAggregatePairs[i].aggregateCollBitmap);
				mAggregatePairs[i].aggregateCollBitmap=NULL;
			}

			mAggregatePairs[i] = mAggregatePairs[--mAggregatePairsSize];
			return true;
		}
	}
	PX_ASSERT(0);
	return false;
}


// PT: TODO: try to use timestamps instead of this
void PxsAABBManager::purgeAggregatePairs(const PxcBpHandle bpElemId)
{
	PxU32 nb = mAggregatePairsSize;
	PxU32 i = 0;
	while(nb--)
	{
		// PT: TODO: try to use one test only
		if(bpElemId==mAggregatePairs[i].mBPElemId0 || bpElemId==mAggregatePairs[i].mBPElemId1)
		{
			// PT: we don't issue "deleted pairs" here because the PhysX broadphase also doesn't. But in theory we should.
			if(mAggregatePairs[i].aggregateCollBitmap)
			{
				PX_DELETE(mAggregatePairs[i].aggregateCollBitmap);
				mAggregatePairs[i].aggregateCollBitmap=NULL;
			}

			mAggregatePairs[i] = mAggregatePairs[--mAggregatePairsSize];
		}
		else i++;
	}
}

///////////////////////////////////////////////////////////////////////////////

void PxsAABBManager::addCreatedPair
	(PxcScratchAllocator* scratchAllocator, 
	void* userdata0, void* userdata1, 
	PxvBroadPhaseOverlap*& createdPairs, PxU32& createdPairsSize, PxU32& createdPairsCapacity)
{
	if(createdPairsSize == createdPairsCapacity)
	{
		const PxU32 oldCapacity = createdPairsCapacity;
		const PxU32 newCapacity = createdPairsCapacity ? createdPairsCapacity*2 : 32;
		createdPairs = resizePODArray<PxvBroadPhaseOverlap>(oldCapacity, newCapacity, scratchAllocator, createdPairs);
		createdPairsCapacity = newCapacity;
	}

	PxvBroadPhaseOverlap& pair = createdPairs[createdPairsSize++];
	pair.userdata0 = userdata0;
	pair.userdata1 = userdata1;

}

void PxsAABBManager::addDeletedPair
	(PxcScratchAllocator* scratchAllocator, void* userdata0, void* userdata1, PxvBroadPhaseOverlap*& deletedPairs, PxU32& deletedPairsSize, PxU32& deletedPairsCapacity)
{
	if(deletedPairsSize==deletedPairsCapacity)
	{
		const PxU32 oldCapacity = deletedPairsCapacity;
		const PxU32 newCapacity = deletedPairsCapacity ? deletedPairsCapacity*2 : 32;
		deletedPairs = resizePODArray<PxvBroadPhaseOverlap>(oldCapacity, newCapacity, scratchAllocator, deletedPairs);
		deletedPairsCapacity = newCapacity;
	}

	PxvBroadPhaseOverlap& pair = deletedPairs[deletedPairsSize++];
	pair.userdata0 = userdata0;
	pair.userdata1 = userdata1;
}


///////////////////////////////////////////////////////////////////////////////

void PxsAABBManager::removeAggregateAggregatePair(Aggregate* PX_RESTRICT c0, Aggregate* PX_RESTRICT c1, Cm::BitMap* PX_RESTRICT aggregateCollBitmap)
{
	PX_ASSERT(c0);
	PX_ASSERT(c1);

	PxU32 elemId0 = c0->elemHeadID;
	PxU32 elemId1 = c1->elemHeadID;
	const PxU32 nb0 = c0->nbElems;
	const PxU32 nb1 = c1->nbElems;
	PxU32 bitIndex = 0;

	PX_ALLOCA(c0eleIds,PxU32,nb0);
	PX_ALLOCA(c1eleIds,PxU32,nb1);
	
	for (PxU32 i = 0; i < nb0; i++)
	{
		if(mAggregateElems.getGroup(elemId0) == PX_INVALID_BP_HANDLE)
		{
			for(PxU32 j=0;j<nb1;j++)
			{
				resetBitmap(*aggregateCollBitmap, mBitmasks, bitIndex+j);
				PX_ASSERT(!testBitmap(*aggregateCollBitmap, mBitmasks, bitIndex+j));
			}
		}
		c0eleIds[i]= elemId0;
		bitIndex += nb1;
		elemId0 = mAggregateElems.getNextId(elemId0);
	}

	PxU32 nb1Valid = 0;
	for (PxU32 i = 0; i < nb1; i++)
	{
		if(mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE)
		{			
			nb1Valid++;
		}
		c1eleIds[i]= elemId1;
		elemId1 = mAggregateElems.getNextId(elemId1);
	}


	// handle case where not all are valid we have to iterate again 
	if(nb1Valid != nb1)
	{
		bitIndex = 0;
		elemId0 = c0->elemHeadID;
		for(PxU32 i=0;i<nb0;i++)
		{
			if(mAggregateElems.getGroup(elemId0) != PX_INVALID_BP_HANDLE)
			{
				elemId1 = c1->elemHeadID;

				for(PxU32 j=0;j<nb1;j++)
				{
					if(mAggregateElems.getGroup(elemId1) == PX_INVALID_BP_HANDLE)
					{
						resetBitmap(*aggregateCollBitmap,mBitmasks,bitIndex+j);
						PX_ASSERT(!testBitmap(*aggregateCollBitmap,mBitmasks,bitIndex+j));
					}
					elemId1 = mAggregateElems.getNextId(elemId1);
				}
				PX_ASSERT(PX_INVALID_BP_HANDLE==elemId1);
			}
			bitIndex += nb1;
			elemId0 = mAggregateElems.getNextId(elemId0);
		}
	}

	Cm::BitMap::Iterator it(*aggregateCollBitmap);
	PxU32 index = it.getNext();
	while(index != Cm::BitMap::Iterator::DONE)
	{
		const PxU32& index0 = index/nb1;
		const PxU32& index1 = index % nb1;

		resetBitmap(*aggregateCollBitmap, mBitmasks, index);
		addDeletedPair
			(&mScratchAllocator, 
			 mAggregateElems.getUserData(c0eleIds[index0]), mAggregateElems.getUserData(c1eleIds[index1]), 
			 mDeletedPairs, mDeletedPairsSize, mDeletedPairsCapacity);
		index = it.getNext();
	}
}



void PxsAABBManager::removeAggregateSinglePair(const PxcBpHandle s0, Aggregate* PX_RESTRICT c1, Cm::BitMap* PX_RESTRICT aggregateCollBitmap)
{
	PX_ASSERT(c1);

	void* PX_RESTRICT userData0 = mBPElems.getUserData(s0);

	PxU32 elemId1 = c1->elemHeadID;
	const PxU32 nb1 = c1->nbElems;

	PX_ALLOCA(c1eleIds,PxU32,nb1);

	for (PxU32 i = 0; i < nb1; i++)
	{
		if(mAggregateElems.getGroup(elemId1) == PX_INVALID_BP_HANDLE)
		{
			resetBitmap(*aggregateCollBitmap, mBitmasks, i);
			PX_ASSERT(!testBitmap(*aggregateCollBitmap, mBitmasks, i));
		}
		c1eleIds[i]= elemId1;		
		elemId1 = mAggregateElems.getNextId(elemId1);
	}

	Cm::BitMap::Iterator it(*aggregateCollBitmap);
	PxU32 index = it.getNext();
	while(index != Cm::BitMap::Iterator::DONE)
	{
		resetBitmap(*aggregateCollBitmap, mBitmasks, index);
		addDeletedPair
			(&mScratchAllocator, 
			 userData0, mAggregateElems.getUserData(c1eleIds[index]),
			 mDeletedPairs, mDeletedPairsSize, mDeletedPairsCapacity);
		index = it.getNext();
	}
}

struct SortedData
{
	PxcBPValType	minX;		
	PxcBpHandle		rank;

	PX_FORCE_INLINE bool operator < (const SortedData& data) const
	{
		return minX < data.minX;
	}		
};

void PxsAABBManager::sortAggregates
(PxcBpHandle* allRankIds, const PxU32 allRankIdsCapacity, PxcBpHandle* allElemIds, const PxU32 allElemIdsCapacity, AggregateSortedData* aggregateSortedData)
{
#ifdef NDEBUG
	PX_UNUSED(allRankIdsCapacity);
	PX_UNUSED(allElemIdsCapacity);
#endif

	PxU32 numSortedTotal = 0;

	SortedData sortedData[MAX_AGGREGATE_BOUND_SIZE];
	PxcBpHandle elemIds[MAX_AGGREGATE_BOUND_SIZE];
	PxcBpHandle ranks[MAX_AGGREGATE_BOUND_SIZE];

	const IntegerAABB* PX_RESTRICT aggregateElemBounds = mAggregateElems.getBounds();

	for(PxU32 i = 0; i < mNumAggregatesToSort; i++)
	{
		//Sort the next aggregate in the list.
		const PxcBpHandle aggregateId = mAggregatesToSort.mBuffer[i];

		//Iterate over the shapes in the aggregate and record the minX and id of each shape.
		PxU32 numSortedRanks = 0;
		PxU32 numElemIds = 0;
		const Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);
		PxcBpHandle elemId = aggregate->elemHeadID;
		while(PX_INVALID_BP_HANDLE != elemId)
		{
			if(mAggregateElems.getGroup(elemId) != PX_INVALID_BP_HANDLE)
			{
				sortedData[numSortedRanks].minX = aggregateElemBounds[elemId].getMinX();
				sortedData[numSortedRanks].rank = (PxcBpHandle)numElemIds;
				numSortedRanks++;
			}
			elemIds[numElemIds] = elemId;
			numElemIds++;

			elemId = mAggregateElems.getNextId(elemId);
		}
		PX_ASSERT(numElemIds == aggregate->nbElems);
		PX_ASSERT(numSortedRanks == aggregate->nbActive);
		PX_ASSERT(numElemIds >= numSortedRanks);

		//Sort by increasing value of minX
		Ps::sort(sortedData, numSortedRanks);

		//Extract the ranks of the sorted values.
		for(PxU32 j = 0; j < numSortedRanks; j++)
		{
			ranks[j] = sortedData[j].rank;
		}

		//Store a list of the elemIds and the sorted ranks.
		aggregateSortedData[aggregateId].rankIds = &allRankIds[numSortedTotal];
		aggregateSortedData[aggregateId].elemIds = &allElemIds[numSortedTotal];
		PxMemCopy(aggregateSortedData[aggregateId].rankIds, ranks, sizeof(PxcBpHandle) * numSortedRanks);
		PxMemCopy(aggregateSortedData[aggregateId].elemIds, elemIds, sizeof(PxcBpHandle) * numElemIds);

		//Finished with the current aggregate.  Move on to the next.
		numSortedTotal += numElemIds;
		PX_ASSERT(numSortedTotal <= allRankIdsCapacity);
		PX_ASSERT(numSortedTotal <= allElemIdsCapacity);
	}
}


void PxsAABBManager::selfCollideAggregates
(const PxU32 startId, const PxU32 nbWorkItems, const AggregateSortedData* aggregateSortedDatas,
 PxvBroadPhaseOverlap*& createdPairs, PxU32& createdPairsSize, PxU32& createdPairsCapacity,
 PxvBroadPhaseOverlap*& deletedPairs, PxU32& deletedPairsSize, PxU32& deletedPairsCapacity)
{
	PX_ASSERT((startId + nbWorkItems) <= mAggregatesUpdated.getElemsSize());

	const PxcBpHandle* updatedAggregates = mAggregatesUpdated.getElems();
	for(PxU32 i = startId ; i < startId + nbWorkItems ;i++)
	{
		PX_ASSERT(i < mAggregatesUpdated.getElemsSize());
		const PxcBpHandle aggregateId = updatedAggregates[i];
		PX_ASSERT(aggregateId < mAggregateManager.getAggregatesCapacity());
		const AggregateSortedData& aggregateSortedData = aggregateSortedDatas[aggregateId];
		Aggregate* aggregate = mAggregateManager.getAggregate(aggregateId);
		SelfCollideBitmap* selfCollideBitmap = &aggregate->selfCollBitmap;
		if(aggregate->performSelfCollision())
		{
			PX_ASSERT(aggregateSortedData.elemIds && aggregateSortedData.rankIds);
			selfCollideAggregate
				(aggregateSortedData, *aggregate, selfCollideBitmap,
				createdPairs, createdPairsSize, createdPairsCapacity,
				deletedPairs, deletedPairsSize, deletedPairsCapacity);
		}
	}
}

#define BPHANDLE_INTERSECT(bounds0,bounds1) \
	(bounds0.mMinMax[5] >= bounds1.mMinMax[2] && bounds1.mMinMax[5] >= bounds0.mMinMax[2] && bounds0.mMinMax[4] >= bounds1.mMinMax[1] && bounds1.mMinMax[4] >= bounds0.mMinMax[1])

void PxsAABBManager::selfCollideAggregate
(const AggregateSortedData& aggregateSortedData, const Aggregate& aggregate, SelfCollideBitmap* selfCollBitmap,
 PxvBroadPhaseOverlap*& createdPairs, PxU32& createdPairsSize, PxU32& createdPairsCapacity,
 PxvBroadPhaseOverlap*& deletedPairs, PxU32& deletedPairsSize, PxU32& deletedPairsCapacity)
{
	PX_ASSERT(aggregate.performSelfCollision());

	//A new bitmap of overlap pairs.
	SelfCollideBitmap newBitmap;
	newBitmap.resetAll();

	//Compute the new bitmap of overlaps.
	//Add to list of created pairs immediately if we find a pair that wasn't previously recorded in the old bitmap.
	{
		const PxcBpHandle* Sorted = aggregateSortedData.rankIds;
		const PxcBpHandle* elemIds = aggregateSortedData.elemIds;

		const IntegerAABB* bounds = mAggregateElems.getBounds();

		//Prune the list
		const PxcBpHandle* const LastSorted = &Sorted[aggregate.nbActive];
		const PxcBpHandle* RunningAddress = Sorted;
		PxcBpHandle Index0, Index1;
		while(RunningAddress<LastSorted && Sorted<LastSorted)
		{
			Index0 = *Sorted++;

			//while(RunningAddress<LastSorted && data[*RunningAddress++].bounds[0]<data[Index0].bounds[0]);
			while(RunningAddress < LastSorted && bounds[elemIds[*RunningAddress++]].getMinX() < bounds[elemIds[Index0]].getMinX());

			const PxcBpHandle* RunningAddress2 = RunningAddress;

			//while(RunningAddress2<LastSorted && data[Index1 = *RunningAddress2++].bounds[0]<=data[Index0].bounds[3])
			while(RunningAddress2 < LastSorted && bounds[elemIds[Index1 = *RunningAddress2++]].getMinX() <= bounds[elemIds[Index0]].getMaxX())
			{
				if(Index0!=Index1)
				{
					const PxcBpHandle elemId0 = elemIds[Index0];
					const PxcBpHandle elemId1 = elemIds[Index1];
					PX_ASSERT(elemId0 != PX_INVALID_BP_HANDLE);
					PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);
					PX_ASSERT(mAggregateElems.getGroup(elemId0) != PX_INVALID_BP_HANDLE);
					PX_ASSERT(mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE);

					if(mAggregateElems.getGroup(elemId0) != mAggregateElems.getGroup(elemId1))
					{
						PX_ASSERT(mAggregateElems.getAggregateActorId(elemId0) != mAggregateElems.getAggregateActorId(elemId1));

						const IntegerAABB& bounds0 = bounds[elemId0];
						const IntegerAABB& bounds1 = bounds[elemId1];

						if(BPHANDLE_INTERSECT(bounds0, bounds1))
						{
							const PxcBpHandle minElemId = PxMin(Index0, Index1);
							const PxcBpHandle maxElemId = PxMax(Index0, Index1);
							newBitmap.set(minElemId, maxElemId);
							if(!selfCollBitmap->test(minElemId, maxElemId))
							{
								addCreatedPair
									(&mScratchAllocator, 
									 mAggregateElems.getUserData(elemId0), mAggregateElems.getUserData(elemId1), 
									 createdPairs, createdPairsSize, createdPairsCapacity);							
							}
						}
					}
				}
			}
		}	
	}

	//Iterate over all overlap pairs in the previous overlap bitmap.  If a pair in the old bitmap 
	//is not found in the new bitmap then we have a lost pair.  
	//Don't report lost pairs due to shape removal so check groupId before reporting a lost pair.
	SelfCollideBitmapIterator it(*selfCollBitmap);
	const PxcBpHandle* elemIds = aggregateSortedData.elemIds;
	PxU32 i,j;
	PxU32 index = it.getNext(i, j);
	while(index != SelfCollideBitmapIterator::DONE)
	{
		if(!newBitmap.test(i, j))
		{
			const PxcBpHandle elemId0 = elemIds[i];
			const PxcBpHandle elemId1 = elemIds[j];
			PX_ASSERT(elemId0 != PX_INVALID_BP_HANDLE);
			PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);

			if((mAggregateElems.getGroup(elemId0) != PX_INVALID_BP_HANDLE) && (mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE))
			{
				addDeletedPair
					(&mScratchAllocator, 
					 mAggregateElems.getUserData(elemId0), mAggregateElems.getUserData(elemId1), 
					 deletedPairs, deletedPairsSize, deletedPairsCapacity);
			}
		}
		index = it.getNext(i, j);
	}

	//Copy the new bit map. 
	*selfCollBitmap = newBitmap;
}

void PxsAABBManager::initialiseAggregateAggregateBitmaps()
{
	 //Allocate all memory serially.
	 for(PxU32 i = 0; i < mNumAggregatePairsToOverlap; i++)
	 {
		 AggregatePair& aggregatePair = mAggregatePairs[mAggregatePairsToOverlap.mBuffer[i]];

		 if(!aggregatePair.aggregateCollBitmap)
		 {
			 //Get the bp elem ids for each element of the pair (one might be a single).
			 const PxcBpHandle bpElemId0 = aggregatePair.mBPElemId0;
			 const PxcBpHandle bpElemId1 = aggregatePair.mBPElemId1;

			 PxU8 nbElems0 = 1;
			 if(!mBPElems.isSingle(bpElemId0))
			 {
				 const PxcBpHandle aggregateId0 = mBPElems.getAggregateOwnerId(bpElemId0);
				 const Aggregate* aggregate0 = mAggregateManager.getAggregate(aggregateId0);
				 nbElems0 = aggregate0->nbElems;
			 }

			 PxU8 nbElems1 = 1;
			 if(!mBPElems.isSingle(bpElemId1))
			 {
				 const PxcBpHandle aggregateId1 = mBPElems.getAggregateOwnerId(bpElemId1);
				 const Aggregate* aggregate1 = mAggregateManager.getAggregate(aggregateId1);
				 nbElems1 = aggregate1->nbElems;
			 }

			 aggregatePair.aggregateCollBitmap = PX_NEW(Cm::BitMap);
			 const PxU32 bitCount = (PxU32)(nbElems0 * nbElems1);
			 aggregatePair.aggregateCollBitmap->clear(bitCount);
		 }
	 }
 }

void PxsAABBManager::processAggregatePairs
(const PxU32 startId, const PxU32 nbWorkItems, const AggregateSortedData* aggregateSortedData,
 PxvBroadPhaseOverlap*& createdPairs, PxU32& createdPairsSize, PxU32& createdPairsCapacity,
 PxvBroadPhaseOverlap*& deletedPairs, PxU32& deletedPairsSize, PxU32& deletedPairsCapacity)
{
	// Process all active aggregate pairs to complete the number of normal created/deleted pairs
	// An aggregate pair can be (Single, Aggregate) or (Aggregate, Aggregate)

	//Add the created and deleted pairs to createdPairs and deletedPairs.
	for(PxU32 i = startId; i < (startId + nbWorkItems); i++)
	{
		PX_ASSERT(i < mNumAggregatePairsToOverlap);
		AggregatePair& aggregatePair = mAggregatePairs[mAggregatePairsToOverlap.mBuffer[i]];
		PX_ASSERT(aggregatePair.aggregateCollBitmap);

		//Get the bp elem ids for each element of the pair (one might be a single).
		const PxcBpHandle bpElemId0 = aggregatePair.mBPElemId0;
		const PxcBpHandle bpElemId1 = aggregatePair.mBPElemId1;
		PX_ASSERT(true == mBPElems.getAABB(bpElemId0).intersects(mBPElems.getAABB(bpElemId1)));

		//Store the aggregateIds
		PxcBpHandle aggregateId0 = PX_INVALID_BP_HANDLE;
		PxcBpHandle aggregateId1 = PX_INVALID_BP_HANDLE;
		
		//Store the aggregates.
		Aggregate* aggregate0 = NULL;
		Aggregate* aggregate1 = NULL;

		//Get the aggregate ids and aggregates.
		if(!mBPElems.isSingle(bpElemId0))
		{
			aggregateId0 = mBPElems.getAggregateOwnerId(bpElemId0);
			aggregate0 = mAggregateManager.getAggregate(aggregateId0);
		}
		if(!mBPElems.isSingle(bpElemId1))
		{
			aggregateId1 = mBPElems.getAggregateOwnerId(bpElemId1);
			aggregate1 = mAggregateManager.getAggregate(aggregateId1);
		}
		PX_ASSERT(aggregate0 || aggregate1);

		if(aggregate0)
		{
			if(aggregate1)
			{
				processAggregateAggregate
					(aggregateSortedData[aggregateId0], *aggregate0, aggregateSortedData[aggregateId1], *aggregate1, 
					 aggregatePair.aggregateCollBitmap, 
					 createdPairs, createdPairsSize, createdPairsCapacity, 
					 deletedPairs, deletedPairsSize, deletedPairsCapacity);
			}
			else
			{
				processAggregateSingle
					(aggregateSortedData[aggregateId0], *aggregate0, bpElemId1, 
					aggregatePair.aggregateCollBitmap, 
					createdPairs, createdPairsSize, createdPairsCapacity, 
					deletedPairs, deletedPairsSize, deletedPairsCapacity);
			}
		}
		else
		{
			if(aggregate1)
			{
				processAggregateSingle
					(aggregateSortedData[aggregateId1], *aggregate1, bpElemId0, 
					aggregatePair.aggregateCollBitmap, 
					createdPairs, createdPairsSize, createdPairsCapacity, 
					deletedPairs, deletedPairsSize, deletedPairsCapacity);
			}
			else
			{
				// Single-single, shouldn't happen
				PX_ASSERT(0);
			}
		}
	}
}

void PxsAABBManager::processAggregateAggregate
(const AggregateSortedData& aggregateSortedData0, const Aggregate& aggregate0, 
 const AggregateSortedData& aggregateSortedData1, const Aggregate& aggregate1, 
 Cm::BitMap* overlapBitmap,
 PxvBroadPhaseOverlap*&createdPairs, PxU32& createdPairsSize, PxU32& createdPairsCapacity, 
 PxvBroadPhaseOverlap*& deletedPairs, PxU32& deletedPairsSize, PxU32& deletedPairsCapacity)
{
	const PxU32 nb1 = aggregate1.nbElems;

	const PxcBpHandle* elemIds0 = aggregateSortedData0.elemIds;
	const PxcBpHandle* elemIds1 = aggregateSortedData1.elemIds;

	//A new bitmap of overlap pairs
	InlineBitmap<MAX_AGGREGATE_BOUND_SIZE*MAX_AGGREGATE_BOUND_SIZE> newBitmap(MAX_AGGREGATE_BOUND_SIZE*MAX_AGGREGATE_BOUND_SIZE, &mScratchAllocator);
	newBitmap.resetAll();

	//Compute the new bitmap of overlaps.
	{
		const IntegerAABB* bounds = mAggregateElems.getBounds();

		const PxcBpHandle* Sorted0 = aggregateSortedData0.rankIds; 
		const PxcBpHandle* Sorted1 = aggregateSortedData1.rankIds; 

		//Prune the lists
		PxU32 Index0, Index1;

		const PxcBpHandle* const LastSorted0 = &Sorted0[aggregate0.nbActive];
		const PxcBpHandle* const LastSorted1 = &Sorted1[aggregate1.nbActive];
		const PxcBpHandle* RunningAddress0 = Sorted0;
		const PxcBpHandle* RunningAddress1 = Sorted1;

		while(RunningAddress1 < LastSorted1 && Sorted0 < LastSorted0)
		{
			Index0 = *Sorted0++;

			//while(RunningAddress1 < LastSorted1 && data1[(*RunningAddress1)].bounds[0] < data0[Index0].bounds[0])	RunningAddress1++;
			while(RunningAddress1 < LastSorted1 && bounds[elemIds1[(*RunningAddress1)]].getMinX() < bounds[elemIds0[Index0]].getMinX())	RunningAddress1++;

			const PxcBpHandle* RunningAddress2_1 = RunningAddress1;

			//while(RunningAddress2_1<LastSorted1 && data1[Index1 = *RunningAddress2_1++].bounds[0]<=data0[Index0].bounds[3])
			while(RunningAddress2_1 < LastSorted1 && bounds[elemIds1[Index1 = *RunningAddress2_1++]].getMinX() <= bounds[elemIds0[Index0]].getMaxX())
			{
				const PxcBpHandle elemId0 = elemIds0[Index0];
				const PxcBpHandle elemId1 = elemIds1[Index1];
				PX_ASSERT(elemId0 != PX_INVALID_BP_HANDLE);
				PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);
				PX_ASSERT(mAggregateElems.getGroup(elemId0) != PX_INVALID_BP_HANDLE);
				PX_ASSERT(mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE);

				if(mAggregateElems.getGroup(elemId0) != mAggregateElems.getGroup(elemId1))
				{
					PX_ASSERT(mAggregateElems.getAggregateActorId(elemId0) != mAggregateElems.getAggregateActorId(elemId1));

					const IntegerAABB& bounds0 = bounds[elemId0];
					const IntegerAABB& bounds1 = bounds[elemId1];

					if(bounds0.intersects(bounds1))
					{
						setBitmap(newBitmap.mBitmap, mBitmasks, Index0*nb1 + Index1);
						if(!testBitmap(*overlapBitmap, mBitmasks, Index0*nb1 + Index1))
						{
							addCreatedPair
								(&mScratchAllocator, 
								 mAggregateElems.getUserData(elemId0), mAggregateElems.getUserData(elemId1), 
								 createdPairs, createdPairsSize, createdPairsCapacity);
						}
					}
				}
			}
		}

		////

		while(RunningAddress0 < LastSorted0 && Sorted1 < LastSorted1)
		{
			Index0 = *Sorted1++;

			//while(RunningAddress0 < LastSorted0 && data0[*RunningAddress0].bounds[0] <= data1[Index0].bounds[0])	RunningAddress0++;
			while(RunningAddress0<LastSorted0 && bounds[elemIds0[*RunningAddress0]].getMinX() <= bounds[elemIds1[Index0]].getMinX())	RunningAddress0++;

			const PxcBpHandle* RunningAddress2_0 = RunningAddress0;

			//while(RunningAddress2_0<LastSorted0 && data0[Index1 = *RunningAddress2_0++].bounds[0]<=data1[Index0].bounds[3])
			while(RunningAddress2_0<LastSorted0 && bounds[elemIds0[Index1 = *RunningAddress2_0++]].getMinX() <= bounds[elemIds1[Index0]].getMaxX())
			{
				const PxcBpHandle elemId0 = elemIds0[Index1];
				const PxcBpHandle elemId1 = elemIds1[Index0];
				PX_ASSERT(elemId0 != PX_INVALID_BP_HANDLE);
				PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);
				PX_ASSERT(mAggregateElems.getGroup(elemId0) != PX_INVALID_BP_HANDLE);
				PX_ASSERT(mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE);

				if(mAggregateElems.getGroup(elemId0) != mAggregateElems.getGroup(elemId1))
				{
					PX_ASSERT(mAggregateElems.getAggregateActorId(elemId0) != mAggregateElems.getAggregateActorId(elemId1));

					const IntegerAABB& bounds0 = bounds[elemId0];
					const IntegerAABB& bounds1 = bounds[elemId1];

					if(bounds0.intersects(bounds1))
					{
						setBitmap(newBitmap.mBitmap, mBitmasks, Index1*nb1 + Index0);
						if(!testBitmap(*overlapBitmap, mBitmasks, Index1*nb1 + Index0))
						{
							addCreatedPair
								(&mScratchAllocator, 
								 mAggregateElems.getUserData(elemId0), mAggregateElems.getUserData(elemId1), 
								 createdPairs, createdPairsSize, createdPairsCapacity);
						}
					}
				}
			}
		}
	}

	//Iterate over all overlap pairs in the previous overlap bitmap.  If a pair in the old bitmap 
	//is not found in the new bitmap then we have a lost pair.  
	//Don't report lost pairs due to shape removal so check groupId before reporting a lost pair.
	Cm::BitMap::Iterator it(*overlapBitmap);
	PxU32 index = it.getNext();
	while(index != Cm::BitMap::Iterator::DONE)
	{
		if(!testBitmap(newBitmap.mBitmap, mBitmasks, index))
		{
			const PxU32 index0 = index/nb1;
			const PxU32 index1 = index - index0*nb1;

			const PxcBpHandle elemId0 = elemIds0[index0];
			const PxcBpHandle elemId1 = elemIds1[index1];
			PX_ASSERT(elemId0 != PX_INVALID_BP_HANDLE);
			PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);

			if((mAggregateElems.getGroup(elemId0) != PX_INVALID_BP_HANDLE) && (mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE))
			{
				addDeletedPair
					(&mScratchAllocator, 
					 mAggregateElems.getUserData(elemId0), mAggregateElems.getUserData(elemId1), 
					 deletedPairs, deletedPairsSize, deletedPairsCapacity);
			}
		}

		index = it.getNext();
	}

	//Copy the new bitmap into the persistent bitmap.
	PxMemCopy(overlapBitmap->getWords(), newBitmap.mBitmap.getWords(), sizeof(PxU32) * overlapBitmap->getWordCount());
}

void PxsAABBManager::processAggregateSingle
(const AggregateSortedData& aggregateSortedData, const Aggregate& aggregate, const PxU32 singleId, 
 Cm::BitMap* overlapBitmap,
 PxvBroadPhaseOverlap*&createdPairs, PxU32& createdPairsSize, PxU32& createdPairsCapacity, 
 PxvBroadPhaseOverlap*& deletedPairs, PxU32& deletedPairsSize, PxU32& deletedPairsCapacity)
{
	//A new bitmap of overlap pairs
	InlineBitmap<MAX_AGGREGATE_BOUND_SIZE*1> newBitmap(MAX_AGGREGATE_BOUND_SIZE*1, &mScratchAllocator);
	newBitmap.resetAll();

	const PxU32 nb1 = aggregate.nbElems;

	const IntegerAABB* bounds1 = mAggregateElems.getBounds();
	const PxcBpHandle* groups1 = mAggregateElems.getGroups();
	const PxcBpHandle* Sorted1 = aggregateSortedData.rankIds; 
	const PxcBpHandle* elemIds1 = aggregateSortedData.elemIds;

	const IntegerAABB singleBounds[1] = {mBPElems.getBounds()[singleId]};
	const PxcBpHandle singleGroups[1] = {mBPElems.getGroup(singleId)};
	const PxcBpHandle singleRanks[1] = {0};
	const PxcBpHandle singleElemIds[1] = {0};
	const IntegerAABB* bounds0 = singleBounds;
	const PxcBpHandle* groups0 = singleGroups;
	const PxcBpHandle* Sorted0 = singleRanks;
	const PxcBpHandle* elemIds0 = singleElemIds;

	//Compute the new bitmap of overlaps.
	{
		//Prune the lists
		PxcBpHandle Index0, Index1;

		const PxcBpHandle* const LastSorted0 = &Sorted0[1];
		const PxcBpHandle* const LastSorted1 = &Sorted1[aggregate.nbActive];
		const PxcBpHandle* RunningAddress0 = Sorted0;
		const PxcBpHandle* RunningAddress1 = Sorted1;

		while(RunningAddress1 < LastSorted1 && Sorted0 < LastSorted0)
		{
			Index0 = *Sorted0++;

			//while(RunningAddress1 < LastSorted1 && data1[(*RunningAddress1)].bounds[0] < data0[Index0].bounds[0])	RunningAddress1++;
			while(RunningAddress1 < LastSorted1 && bounds1[elemIds1[(*RunningAddress1)]].getMinX() < bounds0[elemIds0[Index0]].getMinX())	RunningAddress1++;

			const PxcBpHandle* RunningAddress2_1 = RunningAddress1;

			//while(RunningAddress2_1<LastSorted1 && data1[Index1 = *RunningAddress2_1++].bounds[0]<=data0[Index0].bounds[3])
			while(RunningAddress2_1 < LastSorted1 && bounds1[elemIds1[Index1 = *RunningAddress2_1++]].getMinX() <= bounds0[elemIds0[Index0]].getMaxX())
			{
				const PxcBpHandle elemId0 = elemIds0[Index0];
				const PxcBpHandle elemId1 = elemIds1[Index1];
				PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);
				PX_ASSERT(mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE);

				if(groups1[elemId1] != groups0[elemId0])
				{
					const IntegerAABB& elemBounds0 = bounds0[elemId0];
					const IntegerAABB& elemBounds1 = bounds1[elemId1];
					if(elemBounds0.intersects(elemBounds1))
					{
						setBitmap(newBitmap.mBitmap, mBitmasks, Index0*nb1 + Index1);
						if(!testBitmap(*overlapBitmap, mBitmasks, Index0*nb1 + Index1))
						{
							addCreatedPair
								(&mScratchAllocator, 
								 mAggregateElems.getUserData(elemId1), mBPElems.getUserData(singleId), 
								 createdPairs, createdPairsSize, createdPairsCapacity);
						}
					}
				}
			}
		}

		////

		while(RunningAddress0 < LastSorted0 && Sorted1 < LastSorted1)
		{
			Index0 = *Sorted1++;

			//while(RunningAddress0 < LastSorted0 && data0[*RunningAddress0].bounds[0] <= data1[Index0].bounds[0])	RunningAddress0++;
			while(RunningAddress0<LastSorted0 && bounds0[elemIds0[*RunningAddress0]].getMinX() <= bounds1[elemIds1[Index0]].getMinX())	RunningAddress0++;

			const PxcBpHandle* RunningAddress2_0 = RunningAddress0;

			//while(RunningAddress2_0<LastSorted0 && data0[Index1 = *RunningAddress2_0++].bounds[0]<=data1[Index0].bounds[3])
			while(RunningAddress2_0<LastSorted0 && bounds0[elemIds0[Index1 = *RunningAddress2_0++]].getMinX() <= bounds1[elemIds1[Index0]].getMaxX())
			{
				const PxcBpHandle elemId0 = elemIds0[Index1];
				const PxcBpHandle elemId1 = elemIds1[Index0];
				PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);
				PX_ASSERT(mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE);

				if(groups1[elemId1] != groups0[elemId0])
				{
					const IntegerAABB& elemBounds0 = bounds0[elemId0];
					const IntegerAABB& elemBounds1 = bounds1[elemId1];
					if(elemBounds0.intersects(elemBounds1))
					{
						setBitmap(newBitmap.mBitmap, mBitmasks, Index1*nb1 + Index0);
						if(!testBitmap(*overlapBitmap, mBitmasks, Index1*nb1 + Index0))
						{
							addCreatedPair
								(&mScratchAllocator, 
								 mAggregateElems.getUserData(elemId1), mBPElems.getUserData(singleId), 
								 createdPairs, createdPairsSize, createdPairsCapacity);
						}
					}
				}
			}
		}
	}

	//Iterate over all overlap pairs in the previous overlap bitmap.  If a pair in the old bitmap 
	//is not found in the new bitmap then we have a lost pair.  
	//Don't report lost pairs due to shape removal so check groupId before reporting a lost pair.
	Cm::BitMap::Iterator it(*overlapBitmap);
	PxU32 index = it.getNext();
	while(index != Cm::BitMap::Iterator::DONE)
	{
		if(!testBitmap(newBitmap.mBitmap, mBitmasks, index))
		{
			const PxU32 index0 = index/nb1;
			const PxU32 index1 = index - index0*nb1;

			const PxcBpHandle elemId1 = elemIds1[index1];
			PX_ASSERT(elemId1 != PX_INVALID_BP_HANDLE);

			if((mAggregateElems.getGroup(elemId1) != PX_INVALID_BP_HANDLE))
			{
				addDeletedPair
					(&mScratchAllocator, 
					 mAggregateElems.getUserData(elemId1), mBPElems.getUserData(singleId), 
					 deletedPairs, deletedPairsSize, deletedPairsCapacity);
			}
		}

		index = it.getNext();
	}

	//Copy the new bitmap into the persistent bitmap.
	PxMemCopy(overlapBitmap->getWords(), newBitmap.mBitmap.getWords(), sizeof(PxU32) * overlapBitmap->getWordCount());
}

///////////////////////////////////////////////////////////////////////////////

Cm::BitMap* PxsAABBManager::promoteBitmap(Cm::BitMap* bitmap, PxU32 nbX, PxU32 nbY, PxU32 newXIndex, PxU32 newYIndex) const
{
	if(!bitmap)
		return NULL;

	Cm::BitMap* newBitmap = PX_NEW(Cm::BitMap);
	const PxU32 newX = newXIndex != PX_INVALID_U32 ? nbX+1 : nbX;
	const PxU32 newY = newYIndex != PX_INVALID_U32 ? nbY+1 : nbY;
	const PxU32 bitSize = newX*newY;
	const PxU32 bitSize128 = (bitSize & 127) ?  ((bitSize + 128) & ~127) : bitSize;
	PX_ASSERT(bitSize128>=bitSize && (0==(bitSize128 & 127)));
	newBitmap->clear(bitSize128);

	PxU32 srcIndex = 0;
	PxU32 dstIndex = 0;
	for(PxU32 j=0;j<nbY;j++)
	{
		if(j==newYIndex)
			dstIndex += newX;
		for(PxU32 i=0;i<nbX;i++)
		{
			if(i==newXIndex)
				dstIndex++;
			if(testBitmap(*bitmap, mBitmasks, srcIndex++))
				setBitmap(*newBitmap, mBitmasks, dstIndex);
			dstIndex++;
		}
	}

	PX_DELETE(bitmap);
	return newBitmap;
}

void PxsAABBManager::promoteBitmaps(Aggregate* aggregate)
{

	// 1) Promote self-CD bitmap
	aggregate->selfCollBitmap.promote(aggregate->nbElems);


	// 2) Promote CD bitmaps
	PxU32 nb = mAggregatePairsSize;
	AggregatePair* PX_RESTRICT aggregatePairs = mAggregatePairs;
	while(nb--)
	{
		AggregatePair& p = *aggregatePairs++;

		// PT: the correct function could be looked up and stored inside "AggregatePair"...
		Aggregate* c0 = mBPElems.isSingle(p.mBPElemId0) ? NULL : mAggregateManager.getAggregate(mBPElems.getAggregateOwnerId(p.mBPElemId0));
		Aggregate* c1 = mBPElems.isSingle(p.mBPElemId1) ? NULL : mAggregateManager.getAggregate(mBPElems.getAggregateOwnerId(p.mBPElemId1));
						
		if(c0)
		{
			if(c1)
			{
				// aggregate-aggregate
				if(c0==aggregate)
				{
					PX_ASSERT(p.aggregateCollBitmap);
					PX_ASSERT(p.aggregateCollBitmap->getWords());
					p.aggregateCollBitmap = promoteBitmap(p.aggregateCollBitmap, aggregate->nbElems, c1->nbElems, 0, PX_INVALID_U32);
				}
				else if(c1==aggregate)
				{
					PX_ASSERT(p.aggregateCollBitmap);
					PX_ASSERT(p.aggregateCollBitmap->getWords());
					p.aggregateCollBitmap = promoteBitmap(p.aggregateCollBitmap, c0->nbElems, aggregate->nbElems, PX_INVALID_U32, 0);
				}
			}
			else
			{
				// aggregate-single
				if(c0==aggregate)
				{
					PX_ASSERT(p.aggregateCollBitmap);
					PX_ASSERT(p.aggregateCollBitmap->getWords());
					p.aggregateCollBitmap = promoteBitmap(p.aggregateCollBitmap, aggregate->nbElems, 1, 0, PX_INVALID_U32);
				}
			}
		}
		else
		{
			if(c1)
			{
				// Single-aggregate
				if(c1==aggregate)
				{
					PX_ASSERT(p.aggregateCollBitmap);
					PX_ASSERT(p.aggregateCollBitmap->getWords());
					p.aggregateCollBitmap = promoteBitmap(p.aggregateCollBitmap, aggregate->nbElems, 1, 0, PX_INVALID_U32);
				}
			}
			else
			{
				// Single-single, shouldn't happen
				PX_ASSERT(0);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void PxsAABBManager::shiftOrigin(const PxVec3& shift)
{
	const PxU32 aggregatesCapacity = mAggregateManager.getAggregatesCapacity();
	for(PxU32 i=0; i < aggregatesCapacity; i++)
	{
		const Aggregate* aggregate = mAggregateManager.getAggregate(i);
		if(aggregate->nbElems)
		{
			IntegerAABB iaabb = mBPElems.getAABB(aggregate->bpElemId);
			iaabb.shift(shift);

			setBPElemVolumeBounds(aggregate->bpElemId, iaabb);

			PxU32 elem = aggregate->elemHeadID;
			while(PX_INVALID_BP_HANDLE != elem)
			{
				IntegerAABB iaabbElem = mAggregateElems.getAABB(elem);
				iaabbElem.shift(shift);

				mAggregateElems.setAABB(elem, iaabbElem);

				elem = mAggregateElems.getNextId(elem);
			}
		}
	}

	const PxU32 singleCapacity = mSingleManager.getCapacity();
	for(PxU32 i=0; i < singleCapacity; i++)
	{
		const Single* single = mSingleManager.getSingle(i);
		PxcBpHandle elem = single->headID;
		while(PX_INVALID_BP_HANDLE != elem)
		{
			IntegerAABB iaabb = mBPElems.getAABB(elem);
			iaabb.shift(shift);

			setBPElemVolumeBounds(elem, iaabb);

			elem = mBPElems.getNextId(elem);
		}
	}

	mBP->shiftOrigin(shift);
}

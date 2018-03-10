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


#include "Px.h"

#include "ScInteraction.h"
#include "PxvDynamics.h"
#include "PsAlloca.h"
#include "PxsContext.h"
#include "ScBodySim.h"
#include "ScScene.h"

#ifdef PX_PS3
#include "CellTimerMarker.h"
#endif

using namespace physx;

Sc::InteractionScene::InteractionScene(Scene& s):
	mActiveBodies				(PX_DEBUG_EXP("interactionSceneActiveBodies")),
	mActiveTwoWayBodyStartIndex	(0),
	mPointerBlock8Pool			(PX_DEBUG_EXP("interactionScene::PointerBlock8Pool")),
	mPointerBlock16Pool			(PX_DEBUG_EXP("interactionScene::PointerBlock16Pool")),
	mPointerBlock32Pool			(PX_DEBUG_EXP("interactionScene::PointerBlock32Pool")),
	mLLContext					(0),
	mTimestamp					(0),
	mOwnerScene					(s)
{
	for (int i=0; i<PX_INTERACTION_TYPE_COUNT; ++i)
		mActiveInteractionCount[i] = 0;
}

Sc::InteractionScene::~InteractionScene()
{
	if (mLLContext)
		PX_DELETE(mLLContext);
}

bool Sc::InteractionScene::init(const PxSceneDesc& desc, PxTaskManager* taskManager, Cm::FlushPool* taskPool, Cm::EventProfiler& profiler/*, PxU32 initialBlocks, PxU32 maxBlocks*/)
{
	for (int i=0; i<PX_INTERACTION_TYPE_COUNT; ++i)
		mActiveInteractionCount[i] = 0;

	if (mLLContext)
	{
		PX_DELETE(mLLContext);
		mLLContext = 0;
	}

	mLLContext = PX_NEW(PxsContext)(desc, taskManager, taskPool, profiler, BodySim::getRigidBodyOffset());

	if (mLLContext == 0)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "Failed to create context; context number limit exceeded?");
		return false;
	}

	return true;
}

void Sc::InteractionScene::addToActiveActorList(Actor& actor)
{
	PX_ASSERT(actor.mSceneArrayIndex >= SC_NOT_IN_ACTIVE_LIST_INDEX);

	Ps::Array<Actor*>* actorListPtr;
	PxU32* actorListStartIndexPtr;
	bool actorComesSecond;

	PX_ASSERT(actor.isDynamicRigid());
	PX_ASSERT(	actor.getIslandNodeType() == IslandNodeInfo::eTWO_WAY ||
				actor.getIslandNodeType() == IslandNodeInfo::eONE_WAY_DOMINATOR);

	// Sort: One-way dominator before two-way
	actorListPtr = &mActiveBodies;
	actorListStartIndexPtr = &mActiveTwoWayBodyStartIndex;
	actorComesSecond = (actor.getIslandNodeType() == IslandNodeInfo::eTWO_WAY);

	PxU32& actorListStartIndex = *actorListStartIndexPtr;
	Ps::Array<Actor*>& actorList = *actorListPtr;
	if (actorComesSecond)
	{
		actor.mSceneArrayIndex = actorList.size();
		actorList.pushBack(&actor);
	}
	else
	{
		if (actorList.size() - actorListStartIndex > 0)
		{
			Actor* swapActor = actorList[actorListStartIndex];

			actor.mSceneArrayIndex = actorListStartIndex;
			actorList[actorListStartIndex] = &actor;

			swapActor->mSceneArrayIndex = actorList.size();
			actorList.pushBack(swapActor);
		}
		else
		{
			actor.mSceneArrayIndex = actorList.size();
			actorList.pushBack(&actor);
		}
		actorListStartIndex++;
	}
}

void Sc::InteractionScene::removeFromActiveActorList(Actor& actor)
{
	const PxU32 sceneArrayIndex = actor.mSceneArrayIndex;
	PX_ASSERT(sceneArrayIndex < SC_NOT_IN_ACTIVE_LIST_INDEX);
	actor.mSceneArrayIndex = SC_NOT_IN_ACTIVE_LIST_INDEX;

	Ps::Array<Actor*>* actorListPtr;
	PxU32* actorListStartIndexPtr;
	bool actorComesSecond;

	PX_ASSERT(actor.isDynamicRigid());
	PX_ASSERT(	actor.getIslandNodeType() == IslandNodeInfo::eTWO_WAY ||
				actor.getIslandNodeType() == IslandNodeInfo::eONE_WAY_DOMINATOR);

	// Sort: One-way dominator before two-way
	actorListPtr = &mActiveBodies;
	actorListStartIndexPtr = &mActiveTwoWayBodyStartIndex;
	actorComesSecond = (actor.getIslandNodeType() == IslandNodeInfo::eTWO_WAY);

	PxU32& actorListStartIndex = *actorListStartIndexPtr;
	Ps::Array<Actor*>& actorList = *actorListPtr;
	if (actorComesSecond)
	{
		PX_ASSERT(actorList.size() - actorListStartIndex > 0);

		actorList.replaceWithLast(sceneArrayIndex);
		if (sceneArrayIndex < actorList.size()) 
			actorList[sceneArrayIndex]->mSceneArrayIndex = sceneArrayIndex;
	}
	else
	{
		PX_ASSERT(actorListStartIndex > 0);

		if (actorList.size() != actorListStartIndex )
		{
			PxU32 swapIndex = actorListStartIndex - 1;

			if (sceneArrayIndex < swapIndex)
			{
				Actor* swapActor = actorList[swapIndex];
				swapActor->mSceneArrayIndex = sceneArrayIndex;
				actorList[sceneArrayIndex] = swapActor;
			}

			actorList.replaceWithLast(swapIndex);
			actorList[swapIndex]->mSceneArrayIndex = swapIndex;
		}
		else
		{
			actorList.replaceWithLast(sceneArrayIndex);
			if (sceneArrayIndex < actorList.size()) 
				actorList[sceneArrayIndex]->mSceneArrayIndex = sceneArrayIndex;
		}
		actorListStartIndex--;
	}
}

void Sc::InteractionScene::addActor(Actor& actor, bool active)
{
	PX_ASSERT(actor.mSceneArrayIndex == SC_NOT_IN_SCENE_INDEX);

	actor.setActive(active, Actor::AS_PART_OF_CREATION);

	if (active)
		addToActiveActorList(actor);
	else
		actor.mSceneArrayIndex = SC_NOT_IN_ACTIVE_LIST_INDEX;
}

void Sc::InteractionScene::removeActor(Actor& actor)
{
	PX_ASSERT(actor.mSceneArrayIndex != SC_NOT_IN_SCENE_INDEX);

	if (actor.isActive())
		removeFromActiveActorList(actor);

	actor.mSceneArrayIndex = SC_NOT_IN_SCENE_INDEX;
}

void Sc::InteractionScene::swapActorArrayIndices(Ps::Array<Actor*>& actorArray, PxU32 id1, PxU32 id2)
{
	Actor* actor1 = actorArray[id1];
	Actor* actor2 = actorArray[id2];
	actorArray[id1] = actor2;
	actorArray[id2] = actor1;
	actor1->mSceneArrayIndex = id2;
	actor2->mSceneArrayIndex = id1;
}

void Sc::InteractionScene::registerInteraction(Interaction* interaction, bool active)
{
	const InteractionType type = interaction->getType();
	interaction->mSceneId = mInteractions[type].size();
	if(mInteractions[type].capacity()==0)
		mInteractions[type].reserve(64);

	mInteractions[type].pushBack(interaction);
	if (active)
	{
		if (interaction->mSceneId > mActiveInteractionCount[type])
			swapInteractionArrayIndices(interaction->mSceneId, mActiveInteractionCount[type], type);
		mActiveInteractionCount[type]++;
	}
}

void Sc::InteractionScene::unregisterInteraction(Interaction* interaction)
{
	const InteractionType type = interaction->getType();
	const PxU32 sceneArrayIndex = interaction->mSceneId;
	mInteractions[type].replaceWithLast(sceneArrayIndex);
	interaction->mSceneId = PX_INVALID_INTERACTION_SCENE_ID;
	if (sceneArrayIndex<mInteractions[type].size()) // The removed interaction was the last one, do not reset its sceneArrayIndex
		mInteractions[type][sceneArrayIndex]->mSceneId = sceneArrayIndex;
	if (sceneArrayIndex<mActiveInteractionCount[type])
	{
		mActiveInteractionCount[type]--;
		if (mActiveInteractionCount[type]<mInteractions[type].size())
			swapInteractionArrayIndices(sceneArrayIndex, mActiveInteractionCount[type], type);
	}
}

void Sc::InteractionScene::swapInteractionArrayIndices(PxU32 id1, PxU32 id2, InteractionType type)
{
	Ps::Array<Interaction*>& interArray = mInteractions[type];
	Interaction* interaction1 = interArray[id1];
	Interaction* interaction2 = interArray[id2];
	interArray[id1] = interaction2;
	interArray[id2] = interaction1;
	interaction1->mSceneId = id2;
	interaction2->mSceneId = id1;
}

bool Sc::InteractionScene::isActiveInteraction(const Interaction* interaction) const
{
	return (interaction->mSceneId < mActiveInteractionCount[interaction->getType()]);
}

void Sc::InteractionScene::notifyInteractionActivated(Interaction* interaction)
{
	PX_ASSERT(interaction->mSceneId != PX_INVALID_INTERACTION_SCENE_ID);

	const InteractionType type = interaction->getType();

	PX_ASSERT(interaction->mSceneId >= mActiveInteractionCount[type]);
	
	if (mActiveInteractionCount[type] < mInteractions[type].size())
		swapInteractionArrayIndices(mActiveInteractionCount[type], interaction->mSceneId, type);
	mActiveInteractionCount[type]++;
}

void Sc::InteractionScene::notifyInteractionDeactivated(Interaction* interaction)
{
	PX_ASSERT(interaction->mSceneId != PX_INVALID_INTERACTION_SCENE_ID);

	const InteractionType type = interaction->getType();
	PX_ASSERT(interaction->mSceneId < mActiveInteractionCount[type]);

	if (mActiveInteractionCount[type] > 1)
		swapInteractionArrayIndices(mActiveInteractionCount[type]-1, interaction->mSceneId, type);
	mActiveInteractionCount[type]--;
}

void Sc::InteractionScene::preAllocate(PxU32 nbBodies, PxU32 nbStaticShapes, PxU32 nbDynamicShapes, PxU32 nbAggregates)
{
	// PT: TODO: this is only used for my addActors benchmark for now. Pre-allocate more arrays here.
//	mInactiveActors.reserve(nbActors);

	mActiveBodies.reserve(PxMax<PxU32>(64,nbBodies));

	if(mLLContext)
		mLLContext->preAllocate(nbBodies, nbStaticShapes, nbDynamicShapes, nbAggregates);
}

void Sc::InteractionScene::retrieveMemory()
{
	mActiveBodies.shrink();
//	mInactiveActors.shrink();

	for(PxU32 i=0; i < PX_INTERACTION_TYPE_COUNT; i++)
	{
		mInteractions[i].shrink();
	}
}

PxsIslandManager& Sc::InteractionScene::getLLIslandManager() const
{
	return mLLContext->getIslandManager();
}

void** Sc::InteractionScene::allocatePointerBlock(PxU32 size)
{
	PX_ASSERT(size>32 || size == 32 || size == 16 || size == 8);
	void* ptr;
	if(size==8)
		ptr = mPointerBlock8Pool.construct();
	else if(size == 16)
		ptr = mPointerBlock16Pool.construct();
	else if(size == 32)
		ptr = mPointerBlock32Pool.construct();
	else
		ptr = PX_ALLOC(size * sizeof(void*), PX_DEBUG_EXP("void*"));

	return reinterpret_cast<void**>(ptr);
}

void Sc::InteractionScene::deallocatePointerBlock(void** block, PxU32 size)
{
	PX_ASSERT(size>32 || size == 32 || size == 16 || size == 8);
	if(size==8)
		mPointerBlock8Pool.destroy(reinterpret_cast<PointerBlock8*>(block));
	else if(size == 16)
		mPointerBlock16Pool.destroy(reinterpret_cast<PointerBlock16*>(block));
	else if(size == 32)
		mPointerBlock32Pool.destroy(reinterpret_cast<PointerBlock32*>(block));
	else
		return PX_FREE(block);
}

void  Sc::InteractionScene::releaseConstraints(bool endOfScene)
{
	PX_ASSERT(mLLContext);
	if(mOwnerScene.getStabilizationEnabled())
	{
		//If stabilization is enabled, we're caching contacts for next frame
		if(!endOfScene)
		{
			//So we only clear memory (flip buffers) when not at the end-of-scene.
			//This means we clear after narrow phase completed so we can 
			//release the previous frame's contact buffers before we enter the solve phase.
			mLLContext->getNpMemBlockPool().releaseContacts();
		}
	}
	else if(endOfScene)
	{
		//We now have a double-buffered pool of mem blocks so we must
		//release both pools (which actually triggers the memory used this 
		//frame to be released 
		mLLContext->getNpMemBlockPool().releaseContacts();
		mLLContext->getNpMemBlockPool().releaseContacts();
	}
	//mLLContext->getNpMemBlockPool().releaseConstraintMemory();
}

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
#include "PsFoundation.h"

#include "ScActor.h"
#include "ScInteraction.h"
#include "ScElement.h"
#include "CmPhysXCommon.h"
#include "PsIntrinsics.h"
#include "PxMath.h"

using namespace physx;

Sc::Actor::Actor(InteractionScene& scene, PxU8 actorType, IslandNodeInfo::Type nodeType) :
	mFirstElement					(NULL),
	mInteractionScene				(scene),
	mSceneArrayIndex				(SC_NOT_IN_SCENE_INDEX),
	mNumTransferringInteractions	(0),
	mNumUniqueInteractions			(0),
	mNumCountedInteractions			(0),
	mActorType						(actorType),
	mIslandNodeInfo					(Ps::to8(nodeType))
{
	//PX_ASSERT(actorType<256);	// PT: type is now stored on a byte

	// PT: what about mTimestamp? mIslandIndex? uninitialized?

	// sizeof(Sc::Actor): 80 => 64 bytes
}


Sc::Actor::~Actor()
{
	PX_ASSERT(mInteractions.size()==0);
	mInteractions.releaseMem(*this);
}


void Sc::Actor::setIslandNodeType(IslandNodeInfo::Type type)
{
	PX_ASSERT(	type == IslandNodeInfo::eONE_WAY_DOMINATOR || 
				type == IslandNodeInfo::eTWO_WAY ||
				type == IslandNodeInfo::eNON_PARTICIPANT);

	PX_ASSERT(getIslandNodeType() != type); // This must only get called if the type does change

	if (isActive())
		mInteractionScene.removeFromActiveActorList(*this);

	mIslandNodeInfo &= ~(	IslandNodeInfo::eONE_WAY_DOMINATOR | 
							IslandNodeInfo::eTWO_WAY |
							IslandNodeInfo::eNON_PARTICIPANT );

	mIslandNodeInfo |= type;

	if (isActive())
		mInteractionScene.addToActiveActorList(*this);
}


void Sc::Actor::activateInteractions(PxU32 infoFlag)
{
	const PxU32 nbInteractions = getActorInteractionCount();

	for(PxU16 i=0; i<mNumTransferringInteractions; ++i)
	{
		Ps::prefetchLine(mInteractions[PxMin(PxU16(i+1),PxU16(mNumTransferringInteractions-1))]);
		Interaction* interaction = mInteractions[i];

		if (!mInteractionScene.isActiveInteraction(interaction))
		{
			bool proceed = interaction->onActivate(infoFlag);
			if (proceed)
				mInteractionScene.notifyInteractionActivated(interaction);
		}
	}

	for(PxU32 i=mNumTransferringInteractions; i<nbInteractions; ++i)
	{
		Ps::prefetchLine(mInteractions[PxMin(i+1,nbInteractions-1)]);
		Interaction* interaction = mInteractions[i];

		if (!mInteractionScene.isActiveInteraction(interaction))
		{
			bool proceed = interaction->onActivate(infoFlag);
			if (proceed)
				mInteractionScene.notifyInteractionActivated(interaction);
		}
	}
}


void Sc::Actor::deactivateInteractions(PxU32 infoFlag)
{
	const PxU32 nbInteractions = getActorInteractionCount();

	for(PxU16 i=0; i<mNumTransferringInteractions; ++i)
	{
		Ps::prefetchLine(mInteractions[PxMin(PxU16(i+1),PxU16(mNumTransferringInteractions-1))]);
		Interaction* interaction = mInteractions[i];
		Actor* otherActor = &interaction->mActor0 == this ? &interaction->mActor1 : &interaction->mActor0;

		if (!otherActor->isActive() && mInteractionScene.isActiveInteraction(interaction))
		{
			bool proceed = interaction->onDeactivate(infoFlag);
			if (proceed)
				mInteractionScene.notifyInteractionDeactivated(interaction);
		}
	}

	for(PxU32 i=mNumTransferringInteractions; i<nbInteractions; ++i)
	{
		Ps::prefetchLine(mInteractions[PxMin(i+1,nbInteractions-1)]);
		Interaction* interaction = mInteractions[i];

		if (mInteractionScene.isActiveInteraction(interaction))
		{
			bool proceed = interaction->onDeactivate(infoFlag);
			if (proceed)
				mInteractionScene.notifyInteractionDeactivated(interaction);
		}
	}
}


void Sc::Actor::setActive(bool active, PxU32 infoFlag)
{
	PX_ASSERT(!active || getIslandNodeType() != IslandNodeInfo::eNON_PARTICIPANT);  // Currently there should be no need to activate an actor that does not take part in island generation

	const PxU32 asPartOfCreation = infoFlag & Actor::AS_PART_OF_CREATION;
	if (asPartOfCreation || isActive() != active)
	{
		PX_ASSERT(!asPartOfCreation || (getActorInteractionCount() == 0)); // On creation or destruction there should be no interactions

		if (active)
		{
			mIslandNodeInfo |= IslandNodeInfo::eACTIVE;
			onActivate();

			activateInteractions(infoFlag);

			if (!asPartOfCreation)
			{
				// Inactive => Active
				PX_ASSERT(isActive());
				mInteractionScene.addToActiveActorList(*this);
			}
		}
		else
		{
			mIslandNodeInfo &= ~IslandNodeInfo::eACTIVE;

			deactivateInteractions(infoFlag);

			onDeactivate();

			if (!asPartOfCreation)
			{
				// Active => Inactive
				PX_ASSERT(!isActive());
				mInteractionScene.removeFromActiveActorList(*this);
			}
		}
	}
}


void Sc::Actor::registerInteraction(Interaction* interaction)
{
	const InteractionType type = interaction->getType();

	// Certain interaction types never take part in sleep island generation
	const bool permanentlyNonTransferring = !interaction->getActor0().isDynamicRigid()
										 || !interaction->getActor1().isDynamicRigid()
										 || type == PX_INTERACTION_TYPE_TRIGGER
										 || type == PX_INTERACTION_TYPE_MARKER;

	const PxU32 id = mInteractions.size();

	if (permanentlyNonTransferring)
	{
		mInteractions.pushBack(interaction, *this);
		interaction->setActorId(this, id);
	}
	else
	{
		if (id == mNumTransferringInteractions)
		{
			mInteractions.pushBack(interaction, *this);
			interaction->setActorId(this, id);
		}
		else
		{
			Interaction* swapInteraction = mInteractions[mNumTransferringInteractions];

			mInteractions.pushBack(swapInteraction, *this);
			swapInteraction->setActorId(this, id);

			mInteractions[mNumTransferringInteractions] = interaction;
			interaction->setActorId(this, mNumTransferringInteractions);
		}

		++mNumTransferringInteractions;
	}

	if(type<sInteractionCountedTypes)
	{
		mNumCountedInteractions++;
		PX_ASSERT(mNumCountedInteractions);	// PT: test for overflow
	}
}

void Sc::Actor::unregisterInteraction(Interaction* interaction)
{
	const PxU32 i = interaction->getActorId(this);
	if (i>=mNumTransferringInteractions)
	{
		mInteractions.replaceWithLast(i); 
		if (i<mInteractions.size())
			mInteractions[i]->setActorId(this, i);
	}
	else
	{
		--mNumTransferringInteractions;
		mInteractions[i] = mInteractions[mNumTransferringInteractions];
		mInteractions[i]->setActorId(this, i);
		mInteractions.replaceWithLast(mNumTransferringInteractions);
		if (mNumTransferringInteractions<mInteractions.size())
			mInteractions[mNumTransferringInteractions]->setActorId(this, mNumTransferringInteractions);
	}

	const InteractionType type = interaction->getType();
	if(type<sInteractionCountedTypes)
		mNumCountedInteractions--;
}

void Sc::Actor::onElementAttach(Element& element)
{
	element.mNextInActor = mFirstElement;
	mFirstElement = &element;
}

void Sc::Actor::onElementDetach(Element& element)
{
	PX_ASSERT(mFirstElement);	// PT: else we shouldn't be called
	Element* currentElem = mFirstElement;
	Element* previousElem = NULL;
	while(currentElem)
	{
		if(currentElem==&element)
		{
			if(previousElem)
				previousElem->mNextInActor = currentElem->mNextInActor;
			else
				mFirstElement = currentElem->mNextInActor;
			return;
		}
		previousElem = currentElem;
		currentElem = currentElem->mNextInActor;
	}
	PX_ASSERT(0);
}

// PT: TODO: refactor with Sc::ParticlePacketShape::reallocInteractions
void Sc::Actor::reallocInteractions(Sc::Interaction**& mem, PxU32& capacity, PxU32 size, PxU32 requiredMinCapacity)
{
	Interaction** newMem;
	PxU32 newCapacity;

	if(requiredMinCapacity==0)
	{
		newCapacity = 0;
		newMem = 0;
	}
	else if(requiredMinCapacity<=INLINE_INTERACTION_CAPACITY)
	{
		newCapacity = INLINE_INTERACTION_CAPACITY;
		newMem = mInlineInteractionMem;
	}
	else
	{
		newCapacity = Ps::nextPowerOfTwo(requiredMinCapacity-1);
		newMem = reinterpret_cast<Interaction**>(mInteractionScene.allocatePointerBlock(newCapacity));
	}

	PX_ASSERT(newCapacity >= requiredMinCapacity && requiredMinCapacity>=size);

	if(mem)
	{
		PxMemCopy(newMem, mem, size*sizeof(Interaction*));

		if(mem!=mInlineInteractionMem)
			mInteractionScene.deallocatePointerBlock(reinterpret_cast<void**>(mem), capacity);
	}
	
	capacity = newCapacity;
	mem = newMem;
}

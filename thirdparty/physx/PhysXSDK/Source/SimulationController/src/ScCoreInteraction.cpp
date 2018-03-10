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


#include "ScCoreInteraction.h"
#include "ScNPhaseCore.h"
#include "ScActorInteraction.h"
#include "ScElementSimInteraction.h"

using namespace physx;

void Sc::CoreInteraction::addToDirtyList()
{
	getNPhaseCore()->addToDirtyInteractionList(this);		
}

void Sc::CoreInteraction::removeFromDirtyList()
{
	getNPhaseCore()->removeFromDirtyInteractionList(this);
}

Sc::CoreInteraction* Sc::CoreInteraction::isCoreInteraction(Interaction* interaction)
{
	switch (interaction->getType())
	{
	case PX_INTERACTION_TYPE_CONSTRAINTSHADER:
		return static_cast<ActorInteraction*>(interaction);
	case PX_INTERACTION_TYPE_OVERLAP:
	case PX_INTERACTION_TYPE_TRIGGER:
	case PX_INTERACTION_TYPE_MARKER:
#if PX_USE_PARTICLE_SYSTEM_API
	case PX_INTERACTION_TYPE_PARTICLE_BODY:
#endif
		return static_cast<ElementSimInteraction*>(interaction);
	case PX_INTERACTION_TYPE_ARTICULATION:
	case PX_INTERACTION_TYPE_COUNT:
	default:
		PX_ASSERT(0);
		return NULL;
	}
}

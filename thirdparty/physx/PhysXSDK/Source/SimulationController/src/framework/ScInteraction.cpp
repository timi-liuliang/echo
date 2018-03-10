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
#include "ScInteractionScene.h"

using namespace physx;

Sc::Interaction::Interaction(Actor& actor0, Actor& actor1, InteractionType type, PxU8 flags) :
	mActor0				(actor0),
	mActor1				(actor1), 
	mSceneId			(PX_INVALID_INTERACTION_SCENE_ID), 
	mActorId0			(PX_INVALID_INTERACTION_ACTOR_ID),
	mActorId1			(PX_INVALID_INTERACTION_ACTOR_ID), 
	mInteractionType	(Ps::to8(type)),
	mInteractionFlags	(flags)
{
	PX_ASSERT_WITH_MESSAGE(&actor0.getInteractionScene() == &actor1.getInteractionScene(),"Cannot create an interaction between actors belonging to different scenes.");
	PX_ASSERT((PxU32)type<256);	// PT: type is now stored on a byte

	// sizeof(Sc::Interaction): 32 => 24 bytes
}

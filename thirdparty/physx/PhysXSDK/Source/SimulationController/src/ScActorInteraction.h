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


#ifndef PX_COLLISION_ACTOR_INTERACTION
#define PX_COLLISION_ACTOR_INTERACTION

#include "ScInteraction.h"
#include "ScCoreInteraction.h"
#include "ScActorSim.h"

namespace physx
{
namespace Sc 
{

	class ActorSim;
	class Scene;

	class ActorInteraction : public Interaction, public CoreInteraction
	{
	public:
		PX_INLINE				ActorInteraction(ActorSim& actor0, ActorSim& actor1, InteractionType interactionType, PxU8 flags);
		virtual					~ActorInteraction();

		PX_INLINE	ActorSim&	getActorSim0() const;
		PX_INLINE	ActorSim&	getActorSim1() const;
		virtual		NPhaseCore*	getNPhaseCore() const;
		PX_INLINE	Scene&		getScene() const;
	private:
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////
PX_INLINE Sc::ActorInteraction::ActorInteraction(ActorSim& actor0, ActorSim& actor1, InteractionType interactionType, PxU8 flags) :
	Interaction		(actor0, actor1, interactionType, flags),
	CoreInteraction	(false)
{
}

PX_INLINE Sc::ActorSim& Sc::ActorInteraction::getActorSim0() const
{
	return static_cast<ActorSim&>(getActor0());
}

PX_INLINE Sc::ActorSim& Sc::ActorInteraction::getActorSim1() const
{
	return static_cast<ActorSim&>(getActor1());
}

PX_INLINE Sc::Scene& Sc::ActorInteraction::getScene() const
{
	return getActorSim0().getScene();
}

}

#endif

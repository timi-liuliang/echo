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


#ifndef PX_PHYSICS_SCP_ELEMENT_ACTOR_INTERACTION
#define PX_PHYSICS_SCP_ELEMENT_ACTOR_INTERACTION

#include "ScElementSimInteraction.h"
#include "ScActorElementPair.h"

namespace physx
{
namespace Sc
{

	class ElementActorInteraction : public ElementSimInteraction
	{
	public:
		ElementActorInteraction(ElementSim& element0, ElementSim& element1, ActorElementPair& actorElementPair, InteractionType type, PxU8 flags);
		virtual ~ElementActorInteraction() {}

		//------- ElementSimInteraction --------
		virtual bool isLastFilterInteraction() const;
		//-----------------------------------

		PX_INLINE ActorElementPair* getActorElementPair() const;

	private:
			ElementActorInteraction &operator=(const ElementActorInteraction &);
			ActorElementPair&	mActorElementPair;
	};

} // namespace Sc

PX_INLINE Sc::ElementActorInteraction::ElementActorInteraction(ElementSim& element0, ElementSim& element1, ActorElementPair& actorElementPair, InteractionType type, PxU8 flags) :
	ElementSimInteraction	(element0, element1, type, flags),
	mActorElementPair		(actorElementPair)
{
}


PX_INLINE Sc::ActorElementPair* Sc::ElementActorInteraction::getActorElementPair() const
{
	return &mActorElementPair;
}

}

#endif

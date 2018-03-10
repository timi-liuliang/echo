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


#include "ScParticleBodyInteraction.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScParticleSystemSim.h"
#include "ScScene.h"
#include "PxsContext.h"
#include "ScParticleSystemCore.h"
#include "ScBodySim.h"

using namespace physx;


Sc::ParticleElementRbElementInteraction::ParticleElementRbElementInteraction(ParticlePacketShape &particleShape, ShapeSim& rbShape, ActorElementPair& actorElementPair) :
	ElementActorInteraction	(particleShape, rbShape, actorElementPair, PX_INTERACTION_TYPE_PARTICLE_BODY, PX_INTERACTION_FLAG_FILTERABLE|PX_INTERACTION_FLAG_ELEMENT_ACTOR),
	mPacketShapeIndex(PX_INVALID_PACKET_SHAPE_INDEX),
	mIsActiveForLowLevel	(false)
{
}


Sc::ParticleElementRbElementInteraction::~ParticleElementRbElementInteraction()
{
}


void Sc::ParticleElementRbElementInteraction::initialize(const PxU32 ccdPass)
{
	ElementSimInteraction::initialize();

	mPacketShapeIndex = getParticleShape().addPacketShapeInteraction(this);

	mIsActiveForLowLevel = false;

	if (!isDisabled())
		activateForLowLevel(ccdPass);	// Collision with rigid body
}


void Sc::ParticleElementRbElementInteraction::destroy(bool isDyingRb, const PxU32 ccdPass)
{
	ParticlePacketShape& ps = getParticleShape();

	if (!isDisabled())
		deactivateForLowLevel(isDyingRb, ccdPass);

	const PxU16 idx = mPacketShapeIndex;
	ps.removePacketShapeInteraction(idx);
	if (idx < ps.getPacketShapeInteractionCount())
		ps.getPacketShapeInteraction(idx)->setPacketShapeIndex(idx);
	mPacketShapeIndex = PX_INVALID_PACKET_SHAPE_INDEX;
	
	ElementSimInteraction::destroy();
}


bool Sc::ParticleElementRbElementInteraction::onActivate(PxU32)
{
	return getParticleShape().getParticleSystem().isActive();
}


bool Sc::ParticleElementRbElementInteraction::onDeactivate(PxU32)
{
	return !getParticleShape().getParticleSystem().isActive();
}


void Sc::ParticleElementRbElementInteraction::activateForLowLevel(const PxU32 ccdPass)
{
	//update active cm count and update transform hash/mirroring
	getParticleShape().getParticleSystem().addInteraction(getParticleShape(), getRbShape(), ccdPass);
	mIsActiveForLowLevel = true;
}


void Sc::ParticleElementRbElementInteraction::deactivateForLowLevel(bool isDyingRb, const PxU32 ccdPass)
{
	//update active cm count and update transform hash/mirroring
	getParticleShape().getParticleSystem().removeInteraction(getParticleShape(), getRbShape(), isDyingRb, ccdPass);
	mIsActiveForLowLevel = false;
}



#endif	// PX_USE_PARTICLE_SYSTEM_API

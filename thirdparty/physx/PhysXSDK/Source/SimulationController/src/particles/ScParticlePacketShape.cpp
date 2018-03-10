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


#include "ScParticlePacketShape.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScParticleBodyInteraction.h"
#include "ScNPhaseCore.h"
#include "ScScene.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ParticlePacketShape::ParticlePacketShape(ParticleSystemSim& particleSystem, PxU32 index, PxvParticleShape* llParticleShape) :
	ElementSim(particleSystem, PX_ELEMENT_TYPE_PARTICLE_PACKET),
	mLLParticleShape(llParticleShape)
{
	// Initialize LL shape.
	PX_ASSERT(mLLParticleShape);
	mLLParticleShape->setUserDataV(this);

	setIndex(index);

	// Add particle actor element to broadphase
	getActorSim().getScene().addBroadPhaseVolume(*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ParticlePacketShape::~ParticlePacketShape()
{
	getParticleSystem().unlinkParticleShape(this);		// Let the particle system remove this shape from the list.

	Scene& scene = getActorSim().getScene();

	// Remove particle actor element from broadphase and cleanup interactions
	scene.removeBroadPhaseVolume(*this);
	PX_ASSERT(!hasAABBMgrHandle());

	// Destroy LowLevel shape
	if (mLLParticleShape)
	{
		mLLParticleShape->destroyV();
		mLLParticleShape = 0;
	}

	PX_ASSERT(mInteractions.size()==0);
	mInteractions.releaseMem(*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::computeWorldBounds(PxBounds3& b) const
{
	 b = getBounds();
	 PX_ASSERT(b.isFinite());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const
{
	filterAttr = 0;
	if (getParticleSystem().getInternalFlags() & PxvInternalParticleSystemFlag::eSPH)
		ElementSim::setFilterObjectAttributeType(filterAttr, PxFilterObjectType::ePARTICLE_FLUID);
	else
		ElementSim::setFilterObjectAttributeType(filterAttr, PxFilterObjectType::ePARTICLE_SYSTEM);

	filterData = getParticleSystem().getSimulationFilterData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ParticleSystemSim& Sc::ParticlePacketShape::getParticleSystem() const
{ 
	return static_cast<ParticleSystemSim&>(getActorSim()); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::setInteractionsDirty(CoreInteraction::DirtyFlag flag)
{
	Cm::Range<ParticleElementRbElementInteraction*const> interactions = getPacketShapeInteractions();
	for (; !interactions.empty(); interactions.popFront())
	{
		ParticleElementRbElementInteraction*const interaction = interactions.front();

		PX_ASSERT(interaction->getInteractionFlags() & PX_INTERACTION_FLAG_FILTERABLE);
		PX_ASSERT(interaction->getInteractionFlags() & PX_INTERACTION_FLAG_ELEMENT_ACTOR);
		PX_ASSERT(CoreInteraction::isCoreInteraction(interaction) != NULL);

		interaction->setDirty(flag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PT: TODO: refactor with Sc::Actor::reallocInteractions
void Sc::ParticlePacketShape::reallocInteractions(Sc::ParticleElementRbElementInteraction**& mem, PxU16& capacity, PxU16 size, PxU16 requiredMinCapacity)
{
	ParticleElementRbElementInteraction** newMem;
	PxU16 newCapacity;

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
		const PxU32 desiredCapacity = Ps::nextPowerOfTwo(PxU32(requiredMinCapacity-1));
		PX_ASSERT(desiredCapacity<=65536);

		const PxU32 limit = 0xffff;
		newCapacity = Ps::to16(PxMin(limit, desiredCapacity));
		newMem = reinterpret_cast<ParticleElementRbElementInteraction**>(getInteractionScene().allocatePointerBlock(newCapacity));
	}

	PX_ASSERT(newCapacity >= requiredMinCapacity && requiredMinCapacity>=size);

	PxMemCopy(newMem, mem, size*sizeof(ParticleElementRbElementInteraction*));

	if(mem && mem!=mInlineInteractionMem)
		getInteractionScene().deallocatePointerBlock(reinterpret_cast<void**>(mem), capacity);
	
	capacity = newCapacity;
	mem = newMem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif	// PX_USE_PARTICLE_SYSTEM_API

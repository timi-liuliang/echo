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


#ifndef PX_PHYSICS_PARTICLESHAPE
#define PX_PHYSICS_PARTICLESHAPE

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScElementSim.h"
#include "ScParticleSystemSim.h"
#include "PxvParticleShape.h"

namespace physx
{
namespace Sc
{
	class ParticleElementRbElementInteraction;


	/**
	A collision detection primitive for particle systems.
	*/
	class ParticlePacketShape : public ElementSim
	{
		public:
												ParticlePacketShape(ParticleSystemSim& particleSystem, PxU32 uid, PxvParticleShape* llParticleShape);
												~ParticlePacketShape();
		
		// Element implementation
		virtual		bool						isActive() const { return false; }
		// ~Element

		// ElementSim implementation
		virtual		void						getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const;
		// ~ElementSim

		public:
					void						setIndex(PxU32 index) { PX_ASSERT(index < ((1 << 16) - 1)); mIndex = static_cast<PxU16>(index); }
		PX_INLINE	PxU16						getIndex()	const { return mIndex; }

		PX_INLINE	PxBounds3					getBounds()	const	{ return mLLParticleShape->getBoundsV(); }

					class ParticleSystemSim&	getParticleSystem()	const;

					void						computeWorldBounds(PxBounds3&)	const;

		PX_FORCE_INLINE	ParticleElementRbElementInteraction**	getInteractions()		const { return mInteractions.begin(); }
		PX_FORCE_INLINE	PxU32									getInteractionsCount()	const { return mInteractions.size(); }

		PX_INLINE	PxvParticleShape*			getLowLevelParticleShape() const { return mLLParticleShape; }

					void						setInteractionsDirty(CoreInteraction::DirtyFlag flag);

		// Get an iterator to the interactions connected to the element
		PX_INLINE	Cm::Range<ParticleElementRbElementInteraction*const> getPacketShapeInteractions() const;
		PX_INLINE	PxU32						getPacketShapeInteractionCount() const;

		PX_INLINE	PxU16						addPacketShapeInteraction(ParticleElementRbElementInteraction* interaction);
		PX_INLINE	void						removePacketShapeInteraction(PxU16 id);
		PX_INLINE	ParticleElementRbElementInteraction*	getPacketShapeInteraction(PxU16 id) const;

		private:
					void						reallocInteractions(ParticleElementRbElementInteraction**& mem, PxU16& capacity, PxU16 size, PxU16 requiredMinCapacity);


		static const PxU32 INLINE_INTERACTION_CAPACITY = 4;
					ParticleElementRbElementInteraction* mInlineInteractionMem[INLINE_INTERACTION_CAPACITY];

		Cm::OwnedArray<ParticleElementRbElementInteraction*, ParticlePacketShape, PxU16, &ParticlePacketShape::reallocInteractions>
												mInteractions;

					PxvParticleShape*			mLLParticleShape;		// Low level handle of particle packet
					PxU16						mIndex;
	};

} // namespace Sc


// Get an iterator to the interactions connected to the packet
PX_INLINE Cm::Range<Sc::ParticleElementRbElementInteraction*const> Sc::ParticlePacketShape::getPacketShapeInteractions() const 
{ 
	return Cm::Range<ParticleElementRbElementInteraction*const>(mInteractions.begin(), mInteractions.end());
}

PX_INLINE PxU32 Sc::ParticlePacketShape::getPacketShapeInteractionCount() const
{
	return mInteractions.size();
}

//These are called from interaction creation/destruction
PX_INLINE PxU16 Sc::ParticlePacketShape::addPacketShapeInteraction(ParticleElementRbElementInteraction* interaction)
{
	mInteractions.pushBack(interaction, *this);
	return PxU16(mInteractions.size()-1);
}

PX_INLINE void Sc::ParticlePacketShape::removePacketShapeInteraction(PxU16 id)
{
	mInteractions.replaceWithLast(id);
}

PX_INLINE Sc::ParticleElementRbElementInteraction* Sc::ParticlePacketShape::getPacketShapeInteraction(PxU16 id) const
{
	PX_ASSERT(id<mInteractions.size());
	return mInteractions[id];
}


}

#endif	// PX_USE_PARTICLE_SYSTEM_API

#endif

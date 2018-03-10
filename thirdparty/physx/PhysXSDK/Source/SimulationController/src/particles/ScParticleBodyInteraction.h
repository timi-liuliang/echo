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


#ifndef PX_PHYSICS_SCP_PARTICLEBODYINTERACTION
#define PX_PHYSICS_SCP_PARTICLEBODYINTERACTION

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScElementActorInteraction.h"
#include "ScParticlePacketShape.h"
#include "ScShapeSim.h"


#define PX_INVALID_PACKET_SHAPE_INDEX 0xffff


namespace physx
{
namespace Sc
{

	class ParticleElementRbElementInteraction : public ElementActorInteraction
	{
	public:

		ParticleElementRbElementInteraction(ParticlePacketShape &particleShape, ShapeSim& rbShape, ActorElementPair& actorElementPair);
		virtual ~ParticleElementRbElementInteraction();
		PX_INLINE void* operator new(size_t s, void* memory);
		
		void initialize(const PxU32 ccdPass);
		void destroy(bool isDyingRb, const PxU32 ccdPass);

		//---------- Interaction ----------
		virtual void initialize() { PX_ASSERT(false); }  // the version above should always be used
		virtual void destroy() { PX_ASSERT(false); }  // the version above should always be used

	protected:
		virtual bool onActivate(PxU32 infoFlag);
		virtual bool onDeactivate(PxU32 infoFlag);
		//-----------------------------------

	public:
		//--------- CoreInteraction ---------
		PX_INLINE virtual void updateState(bool removeFromDirtyList);
		//-----------------------------------

		PX_INLINE ParticlePacketShape& getParticleShape() const;
		PX_INLINE ShapeSim& getRbShape() const;
		
		PX_INLINE void onRbShapeChange();

		PX_FORCE_INLINE bool isDisabled() const { return (getActorElementPair()->isSuppressed() || isRbTrigger()); }

		PX_INLINE void setPacketShapeIndex(PxU16 idx);

		static void operator delete(void*) {}
	
	private:
		ParticleElementRbElementInteraction& operator=(const ParticleElementRbElementInteraction&);
		void activateForLowLevel(const PxU32 ccdPass);
		void deactivateForLowLevel(bool isDyingRb, const PxU32 ccdPass);
		

		PX_FORCE_INLINE PxU32 isRbTrigger() const { return (getRbShape().getFlags() & PxShapeFlag::eTRIGGER_SHAPE); }


				PxU16						mPacketShapeIndex;
				bool						mIsActiveForLowLevel; 
	};

} // namespace Sc

PX_INLINE void* Sc::ParticleElementRbElementInteraction::operator new(size_t, void* memory)
{
	return memory;
}


PX_INLINE Sc::ParticlePacketShape& Sc::ParticleElementRbElementInteraction::getParticleShape() const
{
	PX_ASSERT(getElement0().getElementType() == PX_ELEMENT_TYPE_PARTICLE_PACKET);
	return static_cast<ParticlePacketShape&>(getElement0());
}


PX_INLINE Sc::ShapeSim& Sc::ParticleElementRbElementInteraction::getRbShape() const
{
	PX_ASSERT(getElement1().getElementType() == PX_ELEMENT_TYPE_SHAPE);
	PX_ASSERT(static_cast<ShapeSim&>(getElement1()).getScActor().isDynamicRigid() || (static_cast<ShapeSim&>(getElement1()).getScActor().getActorType() == PxActorType::eRIGID_STATIC));
	return static_cast<ShapeSim&>(getElement1());
}

PX_INLINE void Sc::ParticleElementRbElementInteraction::updateState(bool removeFromDirtyList)
{
	CoreInteraction::updateState(removeFromDirtyList);

	if (!isDisabled() && !mIsActiveForLowLevel)
	{
		// The interaction is now valid --> Create low level contact manager
		activateForLowLevel(false);
	}
	else if (isDisabled() && mIsActiveForLowLevel)
	{
		// The interaction is not valid anymore --> Release low level contact manager
		deactivateForLowLevel(false, false);
	}
}

PX_INLINE void Sc::ParticleElementRbElementInteraction::onRbShapeChange()
{
	getParticleShape().getParticleSystem().onRbShapeChange(getParticleShape(), getRbShape());
}

PX_INLINE void Sc::ParticleElementRbElementInteraction::setPacketShapeIndex(PxU16 idx) 
{ 
	PX_ASSERT(idx != PX_INVALID_PACKET_SHAPE_INDEX); 
	mPacketShapeIndex = idx;
}


}

#endif	// PX_USE_PARTICLE_SYSTEM_API

#endif

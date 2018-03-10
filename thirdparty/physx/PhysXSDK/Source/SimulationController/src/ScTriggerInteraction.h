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


#ifndef PX_COLLISION_TRIGGERINTERACTION
#define PX_COLLISION_TRIGGERINTERACTION

#include "ScRbElementInteraction.h"
#include "GuOverlapTests.h"

namespace physx
{
namespace Sc
{
	class TriggerInteraction : public RbElementInteraction
	{
	public:
		enum TriggerFlag
		{
			PAIR_FLAGS_MASK					= ((PxPairFlag::eNOTIFY_TOUCH_LOST << 1) - 1),	// Bits where the PxPairFlags eNOTIFY_TOUCH_FOUND and eNOTIFY_TOUCH_LOST get stored
			NEXT_FREE						= ((PAIR_FLAGS_MASK << 1) & ~PAIR_FLAGS_MASK),

			PROCESS_THIS_FRAME				= (NEXT_FREE << 0), // the trigger pair is new or the pose of an actor was set -> initial processing required.
																// This is important to cover cases where a static or kinematic
																// (non-moving) trigger is created and overlaps with a sleeping
																// object. Or for the case where a static/kinematic is teleported to a new
																// location. TOUCH_FOUND should still get sent in that case.
			LAST							= (NEXT_FREE << 1)
		};

		PX_INLINE							TriggerInteraction(ShapeSim& triggerShape, ShapeSim& otherShape);
		virtual								~TriggerInteraction();

		PX_FORCE_INLINE	Gu::TriggerCache&	getTriggerCache()									{ return mTriggerCache;					}
		PX_FORCE_INLINE	ShapeSim*			getTriggerShape()							const	{ return &getShape0();					}
		PX_FORCE_INLINE	ShapeSim*			getOtherShape()								const	{ return &getShape1();					}

		PX_FORCE_INLINE bool				lastFrameHadContacts()						const	{ return mLastFrameHadContacts;			}
		PX_FORCE_INLINE void				updateLastFrameHadContacts(bool hasContact)			{ mLastFrameHadContacts = hasContact;	}

		PX_FORCE_INLINE PxPairFlags			getTriggerFlags()							const	{ return PxPairFlags((PxU32)mFlags & PAIR_FLAGS_MASK);		}
		PX_FORCE_INLINE void				setTriggerFlags(PxPairFlags triggerFlags);

		PX_FORCE_INLINE void				raiseFlag(TriggerFlag flag)				{ mFlags |= flag; }
		PX_FORCE_INLINE void				clearFlag(TriggerFlag flag)				{ mFlags &= ~flag; }
		PX_FORCE_INLINE	Ps::IntBool			readIntFlag(TriggerFlag flag)	const	{ return Ps::IntBool(mFlags & flag); }

		PX_FORCE_INLINE void				forceProcessingThisFrame(Sc::InteractionScene& scene);

		//////////////////////// interaction ////////////////////////
		virtual			bool				onActivate(PxU32 infoFlag);
		virtual			bool				onDeactivate(PxU32 infoFlag);

		virtual			void				initialize();
		virtual			void				destroy();

	private:
						bool				isOneActorActive();

	protected:
						Gu::TriggerCache	mTriggerCache;
						PxU16				mFlags;
						bool				mLastFrameHadContacts;
	};

} // namespace Sc


Sc::TriggerInteraction::TriggerInteraction(ShapeSim& tShape, ShapeSim& oShape) :
	RbElementInteraction	(tShape, oShape, PX_INTERACTION_TYPE_TRIGGER, PX_INTERACTION_FLAG_RB_ELEMENT|PX_INTERACTION_FLAG_FILTERABLE),
	mFlags					(PROCESS_THIS_FRAME),
	mLastFrameHadContacts	(false)
{
	// The PxPairFlags eNOTIFY_TOUCH_FOUND and eNOTIFY_TOUCH_LOST get stored and mixed up with internal flags. Make sure any breaking change gets noticed.
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_FOUND < PxPairFlag::eNOTIFY_TOUCH_LOST);
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & PxPairFlag::eNOTIFY_TOUCH_FOUND) == PxPairFlag::eNOTIFY_TOUCH_FOUND);
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & PxPairFlag::eNOTIFY_TOUCH_LOST) == PxPairFlag::eNOTIFY_TOUCH_LOST);
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_FOUND < 0xffff);
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_LOST < 0xffff);
	PX_COMPILE_TIME_ASSERT(LAST < 0xffff);
}


PX_FORCE_INLINE void Sc::TriggerInteraction::setTriggerFlags(PxPairFlags triggerFlags)
{
#ifdef PX_CHECKED
	if (triggerFlags & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
	{
		PX_WARN_ONCE(true, "Trigger pairs do not support PxPairFlag::eNOTIFY_TOUCH_PERSISTS events any longer.");
	}
#endif

	PxU32 newFlags = mFlags;
	PxU32 fl = (PxU32)triggerFlags & (PxU32)(PxPairFlag::eNOTIFY_TOUCH_FOUND|PxPairFlag::eNOTIFY_TOUCH_LOST);
	newFlags &= (~PAIR_FLAGS_MASK);  // clear old flags
	newFlags |= fl;

	mFlags = (PxU16)newFlags;
}


PX_FORCE_INLINE void Sc::TriggerInteraction::forceProcessingThisFrame(Sc::InteractionScene& scene)
{
	raiseFlag(PROCESS_THIS_FRAME);

	if (!scene.isActiveInteraction(this))
		scene.notifyInteractionActivated(this);
}

}

#endif

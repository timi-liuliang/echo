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


#ifndef PX_FRAMEWORK_PXELEMENT
#define PX_FRAMEWORK_PXELEMENT

#include "PxvConfig.h"
#include "ScActor.h"
#include "PsUserAllocated.h"
#include "PxsAABBManagerId.h"

namespace physx
{
namespace Sc
{
	
	class ElementInteraction;
	enum ElementType
	{
		PX_ELEMENT_TYPE_SHAPE = 0,
#if PX_USE_PARTICLE_SYSTEM_API
		PX_ELEMENT_TYPE_PARTICLE_PACKET,
#endif
#if PX_USE_CLOTH_API
        PX_ELEMENT_TYPE_CLOTH,
#endif
		PX_ELEMENT_TYPE_COUNT
	};

	enum ElementFlags
	{
		PX_ELEMENT_FLAG_USER		= (1<<0),
		PX_ELEMENT_FLAG_MAX			= (1<<1)
	};

	PX_COMPILE_TIME_ASSERT((PX_ELEMENT_FLAG_MAX>>2) <= 1);		//2 bits reserved for flags on win32 and win64 (8 bits on other platforms)
	PX_COMPILE_TIME_ASSERT(PX_ELEMENT_TYPE_COUNT <= 4);			//2 bits reserved for type on win32 and win64 (8 bits on other platforms)

	/*
	A Element is a part of a Actor. It contributes to the activation framework by adding its 
	interactions to the actor. */
	class Element : public Ps::UserAllocated
	{
		friend class ElementInteraction;
		friend class InteractionScene;

	public:
		class ElementInteractionIterator
		{
			public:
				PX_FORCE_INLINE			ElementInteractionIterator(const Element& e, Cm::Range<Interaction*const> interactions) : mInteractions(interactions), mElement(&e) {}
				ElementInteraction*		getNext();

			private:
				Cm::Range<Interaction*const>	mInteractions;
				const Element*				mElement;
		};

		class ElementInteractionReverseIterator
		{
			public:
				PX_FORCE_INLINE			ElementInteractionReverseIterator(const Element& e, Cm::Range<Interaction*const> interactions) : mInteractions(interactions), mElement(&e) {}
				ElementInteraction*		getNext();

			private:
				Cm::Range<Interaction*const>	mInteractions;
				const Element*				mElement;
		};

	public:
												PX_FORCE_INLINE Element(Actor& actor, ElementType type)
													: mNextInActor				(NULL)
													, mActor					(actor)
#if PX_USE_16_BIT_HANDLES
													, mAABBMgrShapeHandle		(PX_INVALID_BP_HANDLE)
													, mAABBMgrActorHandle		(PX_INVALID_BP_HANDLE)
													, mType						(Ps::to8(type))
													, mElemFlags				(0)
#else
													, mAABBMgrShapeHandle		(PX_INVALID_BP_HANDLE)
													, mType						(Ps::to8(type))
													, mAABBMgrActorHandle		(PX_INVALID_BP_HANDLE)
													, mElemFlags				(0)
#endif
													{
														PX_ASSERT((type & 0x03) == type);	// we use 2 bits to store
														actor.onElementAttach(*this);
													}

		virtual									~Element();

		// Get an iterator to the interactions connected to the element
		PX_INLINE		ElementInteractionIterator getElemInteractions()	const	{ return ElementInteractionIterator(*this, mActor.getActorInteractions()); }
		PX_INLINE		ElementInteractionReverseIterator getElemInteractionsReverse()	const	{ return ElementInteractionReverseIterator(*this, mActor.getActorInteractions()); }
		PX_INLINE		PxU32					getElemInteractionCount()	const	{ return mActor.getActorInteractionCount(); }

		PX_FORCE_INLINE	Actor&					getScActor()				const	{ return mActor;	}

		PX_FORCE_INLINE	InteractionScene&		getInteractionScene()		const;
		PX_INLINE		bool					isActive()					const;

		PX_FORCE_INLINE	ElementType				getElementType()			const	{ return ElementType(mType);	}

		PX_FORCE_INLINE bool					hasAABBMgrHandle()			const	{ return mAABBMgrShapeHandle != PX_INVALID_BP_HANDLE; }
		PX_FORCE_INLINE PxcBpHandle				getAABBMgrHandle()			const	{ return mAABBMgrShapeHandle; }
		PX_FORCE_INLINE	AABBMgrId				getAABBMgrId()				const	{ return AABBMgrId(mAABBMgrShapeHandle, mAABBMgrActorHandle);	}

						bool					createLowLevelVolume(const PxU32 group, const PxBounds3& bounds, const PxU32 aggregateID=PX_INVALID_U32, const AABBMgrId& aabbMgrId = AABBMgrId());
						bool					destroyLowLevelVolume();

						Element*				mNextInActor;
	private:
						Element&				operator=(const Element&);
						Actor&					mActor;

#if PX_USE_16_BIT_HANDLES
						PxcBpHandle				mAABBMgrShapeHandle;
						PxcBpHandle				mAABBMgrActorHandle;
						PxU8					mType;
	protected:
						PxU8					mElemFlags;
						PxU8					mPad[2];
#else
						PxcBpHandle				mAABBMgrShapeHandle	: 30;
						PxcBpHandle				mType				:  2;
						PxcBpHandle				mAABBMgrActorHandle	: 30;
	protected:
						PxcBpHandle				mElemFlags			:  2;

#endif
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE Sc::InteractionScene& Sc::Element::getInteractionScene() const
{
	return mActor.getInteractionScene();
}

PX_INLINE bool Sc::Element::isActive() const
{
	return mActor.isActive();
}


}

#endif

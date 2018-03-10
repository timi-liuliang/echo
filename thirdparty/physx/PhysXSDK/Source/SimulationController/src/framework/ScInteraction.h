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


#ifndef PX_FRAMEWORK_PXINTERACTION
#define PX_FRAMEWORK_PXINTERACTION

#include "Px.h"
#include "ScInteractionScene.h"
#include "ScActor.h"
#include "PsUserAllocated.h"

namespace physx
{

#define PX_INVALID_INTERACTION_ACTOR_ID 0xffff
#define PX_INVALID_INTERACTION_SCENE_ID 0xffffffff

namespace Sc
{
	PX_FORCE_INLINE bool testInteractionCounts(const Sc::Actor& actor0, const Sc::Actor& actor1)
	{
		if(		actor0.getActorInteractionCount()==PX_INVALID_INTERACTION_ACTOR_ID
			||	actor1.getActorInteractionCount()==PX_INVALID_INTERACTION_ACTOR_ID)
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "An actor has more than 64K interactions. New interaction is dropped.");
			return false;
		}
		return true;
	}

	// Interactions are used for connecting actors into activation
	// groups. An interaction always connects exactly two actors. 
	// An interaction is implicitly active if at least one of the two 
	// actors it connects is active.
	// Todo: we might need an interaction callback mechanism

	enum PxInteractionFlags	// PT: TODO: use PxFlags
	{
		PX_INTERACTION_FLAG_RB_ELEMENT		= (1<<0),
		PX_INTERACTION_FLAG_CONSTRAINT		= (1<<1),
		PX_INTERACTION_FLAG_FILTERABLE		= (1<<2),
		PX_INTERACTION_FLAG_ELEMENT_ACTOR	= (1<<3),
		PX_INTERACTION_FLAG_SIP				= (1<<4)	// PT: this one is really redundant with the type... oh well
	};

	class Interaction : public Ps::UserAllocated
	{
		friend class InteractionScene;
		friend class Actor;
		PX_NOCOPY(Interaction)
	public:
		// Interactions automatically register themselves in the actors here
		PX_INLINE virtual void			initialize();

		// Interactions automatically unregister themselves in the actors here
		PX_INLINE virtual void			destroy();

		PX_FORCE_INLINE	Actor&			getActor0()				const	{ return mActor0;							}
		PX_FORCE_INLINE	Actor&			getActor1()				const	{ return mActor1;							}

		// PT: TODO: why do we have both virtual functions AND a type in there?
		PX_FORCE_INLINE	InteractionType	getType()				const	{ return InteractionType(mInteractionType);	}
		PX_FORCE_INLINE	PxU8			getInteractionFlags()	const	{ return mInteractionFlags;					}

		PX_FORCE_INLINE	bool			isRegistered()			const	{ return mSceneId != PX_INVALID_INTERACTION_SCENE_ID; }

	protected:
										Interaction(Actor& actor0, Actor& actor1, InteractionType interactionType, PxU8 flags);
		virtual							~Interaction() {}

		// Called by the framework when an interaction is activated or created.
		// Return true if activation should proceed else return false (for example: joint interaction between two kinematics should not get activated)
		virtual			bool			onActivate(PxU32 infoFlag)	{ PX_UNUSED(infoFlag); return true; }

		// Called by the framework when an interaction is deactivated.
		// Return true if deactivation should proceed else return false (for example: joint interaction between two kinematics can ignore deactivation because it always is deactivated)
		virtual			bool			onDeactivate(PxU32 infoFlag)	{ PX_UNUSED(infoFlag); return true; }

		PX_INLINE		void			setActorId(Actor* actor, PxU32 id)
		{
			PX_ASSERT(Ps::to16(id) != PX_INVALID_INTERACTION_ACTOR_ID);
			if (&mActor0 == actor)
				mActorId0 = Ps::to16(id);
			else
				mActorId1 = Ps::to16(id);
		}

		PX_INLINE		PxU32			getActorId(const Actor* actor)  const
		{
			if (&mActor0 == actor)
				return mActorId0;
			else
				return mActorId1;
		}

	private:
						Actor&			mActor0;
						Actor&			mActor1;

						PxU32			mSceneId;	// PT: TODO: merge this with mInteractionType

		// PT: TODO: are those IDs even worth caching? Since the number of interactions per actor is (or should be) small,
		// we could just do a linear search and save memory here...
						PxU16			mActorId0;	// PT: id of this interaction within mActor0's mInteractions array
						PxU16			mActorId1;	// PT: id of this interaction within mActor1's mInteractions array
	protected:
						PxU8			mInteractionType;	// PT: stored on a byte to save space, should be InteractionType enum
						PxU8			mInteractionFlags;	// PT: captures the same info as mInteractionType, in convenient bits
						PxU8			mPadding[2];
	};

	class InteractionRange
	{
		typedef Cm::Range<Interaction*const> (InteractionScene::*GetInteractionsFunc)(InteractionType type) const;
	public:
		template <size_t N>
		InteractionRange(const InteractionScene& scene, GetInteractionsFunc func, const InteractionType (&types)[N])
			: mScene(scene), mFunc(func), mTypes(types)
		{
			getInteractions();
		}

		bool empty() const { return mInteractions.empty(); }
		void popFront() { mInteractions.popFront(); getInteractions(); }
		void popBack() { mInteractions.popBack(); getInteractions(); }

		template <typename T> T* front() const { return static_cast<T*>( mInteractions.front()); }
		template <typename T> T* back() const { return static_cast<T*>( mInteractions.back()); }

	private:

		InteractionRange& operator=(const InteractionRange&);

		void getInteractions()
		{
			while(mInteractions.empty())
			{
				if(mTypes.empty())
					return;
				mInteractions = (mScene.*mFunc)(mTypes.front());
				mTypes.popFront();
			}
		}

		const InteractionScene&			mScene;
		GetInteractionsFunc				mFunc;
		Cm::Range<const InteractionType>	mTypes;
		Cm::Range<Interaction*const>		mInteractions;
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////

PX_INLINE void Sc::Interaction::initialize()
{
	bool active = onActivate(0);
	mActor0.getInteractionScene().registerInteraction(this, active);
	mActor0.registerInteraction(this);
	mActor1.registerInteraction(this);
}


PX_INLINE void Sc::Interaction::destroy()
{
	mActor0.unregisterInteraction(this);
	mActor1.unregisterInteraction(this);
	mActor0.getInteractionScene().unregisterInteraction(this);
}

}

#endif // PX_FRAMEWORK_PXINTERACTION

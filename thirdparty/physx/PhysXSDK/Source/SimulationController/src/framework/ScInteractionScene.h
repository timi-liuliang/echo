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


#ifndef PX_FRAMEWORK_PXSCENE
#define PX_FRAMEWORK_PXSCENE

#include "Px.h"
#include "PsArray.h"
#include "PsPool.h"
#include "CmUtils.h"
#include "CmEventProfiler.h"
#include "PsUserAllocated.h"
#include "ScInteractionType.h"
#include "CmRange.h"

namespace physx
{
class PxSceneDesc;
class PxvObjectFactory;
class PxsRigidBody;
class PxsContext;
class PxsIslandManager;
class PxTaskManager;

namespace Cm
{
	class FlushPool;
}

namespace Sc
{
	class Actor;
	class Element;
	class Interaction;

	class Scene;

	// The scene is the most central part of the interaction framework.
	// It keeps lists of active actors and interactions and manages deactivation/
	// island generation.
	class InteractionScene : public Ps::UserAllocated
	{
		friend class Actor;
		friend class Element;
		friend class Interaction;
	
	protected:
		InteractionScene &operator=(const InteractionScene &);
	public:
													InteractionScene(Sc::Scene& ownerScene);
		// You must explicitly delete all actors and interactions before
		// destroying the scene. That's life.
													~InteractionScene();
		// Initialize the scene
						bool						init(const PxSceneDesc& desc, PxTaskManager* taskManager, Cm::FlushPool* taskPool, Cm::EventProfiler& profiler);

		PX_FORCE_INLINE bool						isValid() const { return (mLLContext != 0); }

		// Get the number of actors that are active
		PX_FORCE_INLINE PxU32						getActiveActorCount() const { return mActiveBodies.size(); }

		// Get the number of active one-way dominator actors
		PX_FORCE_INLINE PxU32						getActiveOneWayDominatorCount() const { return mActiveTwoWayBodyStartIndex; }

		// Get an iterator to the active actors
		PX_FORCE_INLINE	Cm::Range<Actor*const>		getActiveBodies() const { return Cm::Range<Actor*const>(mActiveBodies.begin(), mActiveBodies.end()); }

		// Get an array of the active actors.
		PX_FORCE_INLINE	Actor*const*				getActiveBodiesArray() const { return mActiveBodies.begin(); }
		PX_FORCE_INLINE PxU32						getNumActiveBodies() const {return mActiveBodies.size();}

		// Get an iterator to the active one-way dominator actors
		PX_FORCE_INLINE	Actor*const*				getActiveOneWayDominatorBodies() const { return mActiveBodies.begin(); }

		// Check if interaction is registered as active
						bool						isActiveInteraction(const Interaction*) const;

		// Get the total number of interactions of a certain type
		PX_FORCE_INLINE PxU32						getInteractionCount(InteractionType type) const { return mInteractions[type].size(); }

		// Get the number of active interactions of a certain type (an active interaction is an interaction that has at least one active actor)
		PX_FORCE_INLINE PxU32						getActiveInteractionCount(InteractionType type) const  { return mActiveInteractionCount[type]; }

		// Get an iterator to all interactions of a certain type
		PX_FORCE_INLINE	Cm::Range<Interaction*const>	getInteractions(InteractionType type) const 
													{ 
														return Cm::Range<Interaction*const>(mInteractions[type].begin(), mInteractions[type].end()); 
													}

		// Get an iterator to the active interactions of a certain type
		PX_INLINE		Cm::Range<Interaction*const>	getActiveInteractions(InteractionType type) const 
													{ 
														Interaction*const* first = mInteractions[type].begin();
														return Cm::Range<Interaction*const>(first, first + mActiveInteractionCount[type]); 
													}



		PX_FORCE_INLINE PxsContext*					getLowLevelContext()			{ return mLLContext;	}
		PX_FORCE_INLINE const PxsContext*			getLowLevelContext()	const	{ return mLLContext;	}
		PX_FORCE_INLINE Sc::Scene&					getOwnerScene()			const	{ return mOwnerScene;	}

						void						preAllocate(PxU32 nbBodies, PxU32 nbStaticShapes, PxU32 nbDynamicShapes, PxU32 nbAggregates);	// PT: TODO: add a better interface here
						void						retrieveMemory();  // Shrink buffers to the minimum

						void						addActor(Actor& actor, bool active);
						void						removeActor(Actor& actor);

						PxsIslandManager&			getLLIslandManager()						const;

		// for pool management of interaction arrays, a major cause of dynamic allocation
						void**						allocatePointerBlock(PxU32 size);
						void						deallocatePointerBlock(void**, PxU32 size);

						void						releaseConstraints(bool endOfScene);

						void						notifyInteractionActivated(Interaction* interaction);
						void						notifyInteractionDeactivated(Interaction* interaction);

	private:
						void						addToActiveActorList(Actor& actor);
						void						removeFromActiveActorList(Actor& actor);

						void						registerInteraction(Interaction* interaction, bool active);
						void						unregisterInteraction(Interaction* interaction);

		static			void						swapActorArrayIndices(Ps::Array<Actor*>& actorArray, PxU32 id1, PxU32 id2);
						void						swapInteractionArrayIndices(PxU32 id1, PxU32 id2, InteractionType type);

						Ps::Array<Actor*>			mActiveBodies;		// Sorted: one-way dominators before two-way
						PxU32						mActiveTwoWayBodyStartIndex;

						Ps::Array<Interaction*>		mInteractions[PX_INTERACTION_TYPE_COUNT];
						PxU32						mActiveInteractionCount[PX_INTERACTION_TYPE_COUNT]; // Interactions with id < activeInteractionCount are active

						typedef Cm::Block<void*, 8>	PointerBlock8;
						typedef Cm::Block<void*, 16> PointerBlock16;
						typedef Cm::Block<void*, 32> PointerBlock32;

						Ps::Pool<PointerBlock8>		mPointerBlock8Pool;
						Ps::Pool<PointerBlock16>	mPointerBlock16Pool;
						Ps::Pool<PointerBlock32>	mPointerBlock32Pool;

						PxsContext*					mLLContext;

						PxU32						mTimestamp;

						Sc::Scene&					mOwnerScene;	// PT: evil back pointer
	};

} // namespace Sc

}

#endif //PX_FRAMEWORK_PXSCENE

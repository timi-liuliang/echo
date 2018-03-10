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

          
#ifndef PX_COLLISION_SHAPEINSTANCEPAIR_LL
#define PX_COLLISION_SHAPEINSTANCEPAIR_LL

#include "ScRbElementInteraction.h"
#include "ScActorPair.h"
#include "ScScene.h"
#include "ScBodySim.h"
#include "PxsContactManager.h"
#include "PxsContext.h"

#define INVALID_REPORT_PAIR_ID	0xffffffff

namespace physx
{
namespace Sc
{
	/*
	Description: A shape instance pair represents a pair of objects which _may_ have contacts. Created by the broadphase
	and processed by the NPhaseCore.
	*/
	class ShapeInstancePairLL : public RbElementInteraction
	{
		friend class NPhaseCore;
		ShapeInstancePairLL& operator=(const ShapeInstancePairLL&);
	public:
		enum SipFlag
		{
			PAIR_FLAGS_MASK					= (PxPairFlag::eNEXT_FREE - 1),	// Bits where the PxPairFlags get stored
			NEXT_FREE						= ((PAIR_FLAGS_MASK << 1) & ~PAIR_FLAGS_MASK),

			HAS_TOUCH						= (NEXT_FREE << 0),		// Tracks the last know touch state
			HAS_NO_TOUCH					= (NEXT_FREE << 1),		// Tracks the last know touch state
			TOUCH_KNOWN						= (HAS_TOUCH | HAS_NO_TOUCH),  // If none of these flags is set, the touch state is not known (for example, this is true for pairs that never ran narrowphase

			CONTACTS_COLLECT_POINTS			= (NEXT_FREE << 2),		// The user wants to get the contact points (includes debug rendering)
			CONTACTS_RESPONSE_DISABLED		= (NEXT_FREE << 3),		// Collision response disabled

			CONTACT_FORCE_THRESHOLD_PAIRS	= (PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | (PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS | (PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST,
			CONTACT_REPORT_EVENTS			= (PxU32)PxPairFlag::eNOTIFY_TOUCH_FOUND | (PxU32)PxPairFlag::eNOTIFY_TOUCH_PERSISTS | (PxU32)PxPairFlag::eNOTIFY_TOUCH_LOST |
												(PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | (PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS | (PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST,
			CONTACT_REPORT_EXTRA_DATA		= (PxU32)PxPairFlag::ePRE_SOLVER_VELOCITY | (PxU32)PxPairFlag::ePOST_SOLVER_VELOCITY | (PxU32)PxPairFlag::eCONTACT_EVENT_POSE,

			FORCE_THRESHOLD_EXCEEDED_NOW	= (NEXT_FREE << 4),
			FORCE_THRESHOLD_EXCEEDED_BEFORE	= (NEXT_FREE << 5),
			FORCE_THRESHOLD_EXCEEDED_FLAGS	= FORCE_THRESHOLD_EXCEEDED_NOW | FORCE_THRESHOLD_EXCEEDED_BEFORE,

			IS_IN_PERSISTENT_EVENT_LIST		= (NEXT_FREE << 6), // The pair is in the list of persistent contact events
			WAS_IN_PERSISTENT_EVENT_LIST	= (NEXT_FREE << 7), // The pair is inactive but used to be in the list of persistent contact events
			IN_PERSISTENT_EVENT_LIST		= IS_IN_PERSISTENT_EVENT_LIST | WAS_IN_PERSISTENT_EVENT_LIST,
			IS_IN_FORCE_THRESHOLD_EVENT_LIST= (NEXT_FREE << 8), // The pair is in the list of force threshold contact events
			IS_IN_CONTACT_EVENT_LIST		= IS_IN_PERSISTENT_EVENT_LIST | IS_IN_FORCE_THRESHOLD_EVENT_LIST,

			SHAPE0_IS_KINEMATIC				= (NEXT_FREE << 9),	// Cached for performance reasons
			SHAPE1_IS_KINEMATIC				= (NEXT_FREE << 10),	// Cached for performance reasons

			LL_MANAGER_RECREATE_EVENT		= CONTACT_REPORT_EVENTS | CONTACTS_COLLECT_POINTS |
											  CONTACTS_RESPONSE_DISABLED | (PxU32)PxPairFlag::eMODIFY_CONTACTS,

			FACE_INDEX_REPORT_PAIR			= (NEXT_FREE << 11),  // One shape is a mesh/heightfield shape and face indices are reported

			ACTIVE_MANAGER_NOT_ALLOWED		= (NEXT_FREE << 12)  // the active manager has not been allowed
		};
												ShapeInstancePairLL(ShapeSim& s1, ShapeSim& s2, ActorPair& aPair, PxPairFlags pairFlags);
		virtual									~ShapeInstancePairLL()	
		{ 
			PX_ASSERT(!mLLIslandHook.isManaged()); 
		}

		// Submits to contact stream
						void					processUserNotification(PxU32 contactEvent, PxU16 infoFlags, bool touchLost, const PxU32 ccdPass);  // ccdPass is 0 for discrete collision and then 1,2,... for the CCD passes

						void					visualize(Cm::RenderOutput&);

						PxU32					getContactPointData(const void*& contactData, PxU32& contactDataSize, PxU32& contactPointCount, const PxReal*& impulses, PxU32 startOffset);

						bool					managerLostTouch(const PxU32 ccdPass, bool adjustCounters);
						void					managerNewTouch(const PxU32 ccdPass, bool adjustCounters);

		PX_FORCE_INLINE	void					adjustCountersOnLostTouch();
		PX_FORCE_INLINE	void					adjustCountersOnNewTouch();

		PX_FORCE_INLINE	void					sendCCDRetouch(const PxU32 ccdPass);
						void					setContactReportPostSolverVelocity(ContactStreamManager& cs);
		PX_FORCE_INLINE	void					sendLostTouchReport(bool shapeVolumeRemoved, const PxU32 ccdPass);
		PX_FORCE_INLINE	void					resetManagerCachedState()	const	{ if (mManager)	mManager->resetCachedState();		}
		PX_FORCE_INLINE	ActorPair*				getActorPair()				const	{ return &mActorPair;								}
		PX_INLINE		Ps::IntBool				isReportPair()				const	{ return Ps::IntBool(getPairFlags() & CONTACT_REPORT_EVENTS);	}
		PX_INLINE		Ps::IntBool				hasTouch()					const	{ return readIntFlag(HAS_TOUCH); }
		PX_INLINE		Ps::IntBool				hasCCDTouch()				const	{ PX_ASSERT(mManager); return mManager->getHadCCDContact(); }
		PX_INLINE		void					swapAndClearForceThresholdExceeded();

		PX_FORCE_INLINE void					raiseFlag(SipFlag flag)				{ mFlags |= flag; }
		PX_FORCE_INLINE	Ps::IntBool				readIntFlag(SipFlag flag)	const	{ return Ps::IntBool(mFlags & flag); }
		PX_FORCE_INLINE	PxU32					getPairFlags() const;

		PX_FORCE_INLINE	void					removeFromReportPairList();

						void					onPoseChangedWhileSleeping();
						void					postNarrowPhaseSecondPass(PxsIslandManager&, bool connect);
						void					postIslandGenSecondPass();

		PX_FORCE_INLINE	Ps::IntBool				hasKnownTouchState() const;

	private:
						PxU32					mContactReportStamp;
						PxU32					mFlags;
						ActorPair&				mActorPair;
						PxU32					mReportPairIndex;			// Owned by NPhaseCore for its report pair list

						PxsContactManager*		mManager;

						PxsIslandManagerEdgeHook mLLIslandHook;

						PxU16					mReportStreamIndex;  // position of this pair in the contact report stream

		// Internal functions:

						void					createManager();
		PX_INLINE		void					resetManager();
		PX_INLINE		bool					updateManager();
		PX_INLINE		void					destroyManager();
		PX_FORCE_INLINE	bool					activeManagerAllowed();
		PX_FORCE_INLINE	PxU32					getManagerContactState()		const	{ return mFlags & LL_MANAGER_RECREATE_EVENT; }

		PX_FORCE_INLINE void					clearFlag(SipFlag flag)					{ mFlags &= ~flag;				}
		PX_INLINE		void					setFlag(SipFlag flag, bool value)
												{
													if (value)
														raiseFlag(flag);
													else
														clearFlag(flag);
												}
		PX_FORCE_INLINE void					setHasTouch() { clearFlag(HAS_NO_TOUCH); raiseFlag(HAS_TOUCH); }
		PX_FORCE_INLINE void					setHasNoTouch() { clearFlag(HAS_TOUCH); raiseFlag(HAS_NO_TOUCH); }

		PX_FORCE_INLINE void					setSweptProperties();

		PX_FORCE_INLINE	void					setPairFlags(PxPairFlags flags);
		PX_FORCE_INLINE	bool					reportPairFlagsChanged(PxPairFlags flags) const;

		PX_FORCE_INLINE	void					processReportPairOnActivate();
		PX_FORCE_INLINE	void					processReportPairOnDeactivate();

		// Sc::Interaction
		virtual			void					initialize();
		virtual			void					destroy();

		virtual			bool					onActivate(PxU32 infoFlag);
		virtual			bool					onDeactivate(PxU32 infoFlag);
		//~Sc::Interaction

		// CoreInteraction
		virtual			void					updateState(bool removeFromDirtyList);
		//~CoreInteraction

		PX_INLINE		Scene&					getScene()				const	{ return getShape0().getScene();									}
	};

} // namespace Sc


PX_FORCE_INLINE void Sc::ShapeInstancePairLL::sendLostTouchReport(bool shapeVolumeRemoved, const PxU32 ccdPass)
{
	PX_ASSERT(hasTouch());
	PX_ASSERT(isReportPair());

	PxU32 thresholdForceLost = readIntFlag(ShapeInstancePairLL::FORCE_THRESHOLD_EXCEEDED_NOW) ? (PxU32)PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST : 0;  // make sure to only send report if force is still above threshold
	PxU32 triggeredFlags = getPairFlags() & ((PxU32)PxPairFlag::eNOTIFY_TOUCH_LOST | thresholdForceLost);
	if (triggeredFlags)
	{
		PxU16 infoFlag = 0;
		if (mActorPair.getTouchCount() == 1)  // this code assumes that the actor pair touch count does get decremented afterwards
		{
			infoFlag |= PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH;
		}

		processUserNotification(triggeredFlags, infoFlag, true, ccdPass);
	}

	if (mActorPair.hasReportData() && !mActorPair.streamCompareStamp(getScene().getTimeStamp()))
	{
		// If there has been no event recorded yet, there is no need to worry about events with shape pointers which might later reference
		// removed shapes due to buffered removal, i.e., removal while the simulation was running.
		// This is also correct for CCD scenarios where a touch could get lost and then found again etc. If in such a case there ever is a contact event
		// recorded, there will always be another sendLostTouchReport() call at some later point (caused by the simulation or when the shape gets
		// removed at fetchResults).
		PxU16 flagsToRaise = ContactStreamManagerFlag::eHAS_PAIRS_THAT_LOST_TOUCH;

		ContactStreamManager& cs = mActorPair.getContactStreamManager();

		if (shapeVolumeRemoved)
		{
			flagsToRaise |= ContactStreamManagerFlag::eTEST_FOR_REMOVED_SHAPES;

			// if an actor gets deleted while the simulation is running and the actor has a pending contact report with post solver 
			// velocity extra data, then the post solver velocity needs to get written now because it is too late when the reports
			// get fired (the object will have been deleted already)

			if (cs.getFlags() & ContactStreamManagerFlag::eNEEDS_POST_SOLVER_VELOCITY)
				setContactReportPostSolverVelocity(cs);
		}

		cs.raiseFlags(flagsToRaise);
	}
}


PX_FORCE_INLINE void Sc::ShapeInstancePairLL::setPairFlags(PxPairFlags flags)
{
	PX_ASSERT((PxU32)flags < PxPairFlag::eNEXT_FREE);  // to find out if a new PxPairFlag has been added after eLAST instead of in front

	PxU32 newFlags = mFlags;
	PxU32 fl = (PxU32)flags & PAIR_FLAGS_MASK;
	newFlags &= (~PAIR_FLAGS_MASK);  // clear old flags
	newFlags |= fl;

	mFlags = newFlags;
}


// PT: using PxU32 to remove LHS. Please do not undo this.
//PX_FORCE_INLINE PxPairFlags Sc::ShapeInstancePairLL::getReportPairFlags() const
PX_FORCE_INLINE PxU32 Sc::ShapeInstancePairLL::getPairFlags() const
{
	return (mFlags & PAIR_FLAGS_MASK);
}


PX_FORCE_INLINE bool Sc::ShapeInstancePairLL::reportPairFlagsChanged(PxPairFlags flags) const
{
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & 1) == 1);  // the code below assumes that the pair flags are stored first within mFlags
	PxU32 newFlags = flags;
	newFlags = (newFlags & PAIR_FLAGS_MASK);
	PxU32 oldFlags = mFlags & PAIR_FLAGS_MASK;

	return (newFlags != oldFlags);
}


PX_INLINE void Sc::ShapeInstancePairLL::swapAndClearForceThresholdExceeded()
{
	PxU32 flags = mFlags;

	PX_COMPILE_TIME_ASSERT(FORCE_THRESHOLD_EXCEEDED_NOW == (FORCE_THRESHOLD_EXCEEDED_BEFORE >> 1));

	PxU32 nowToBefore = (flags & FORCE_THRESHOLD_EXCEEDED_NOW) << 1;
	flags &= ~(FORCE_THRESHOLD_EXCEEDED_NOW | FORCE_THRESHOLD_EXCEEDED_BEFORE);
	flags |= nowToBefore;

	mFlags = flags;
}

PX_FORCE_INLINE	void Sc::ShapeInstancePairLL::removeFromReportPairList()
{
	// this method should only get called if the pair is in the list for
	// persistent or force based contact reports
	PX_ASSERT(mReportPairIndex != INVALID_REPORT_PAIR_ID);
	PX_ASSERT(readIntFlag(IS_IN_CONTACT_EVENT_LIST));

	Scene& scene = getScene();

	if (readIntFlag(IS_IN_FORCE_THRESHOLD_EVENT_LIST))
		scene.getNPhaseCore()->removeFromForceThresholdContactEventPairs(this);
	else 
	{
		PX_ASSERT(readIntFlag(IS_IN_PERSISTENT_EVENT_LIST));
		scene.getNPhaseCore()->removeFromPersistentContactEventPairs(this);
	}
}

PX_INLINE void Sc::ShapeInstancePairLL::resetManager()
{
	destroyManager();

	if (activeManagerAllowed())
		createManager();
}

PX_INLINE bool Sc::ShapeInstancePairLL::updateManager()
{
	if (activeManagerAllowed())
	{
		if (mManager == 0)
			createManager();

		return (mManager != NULL);  // creation might fail (pool reached limit, mem allocation failed etc.)
	}
	else
		return false;
}

PX_INLINE void Sc::ShapeInstancePairLL::destroyManager()
{
	if (mManager != 0)
	{
		InteractionScene& intScene = getScene().getInteractionScene();
		intScene.getLLIslandManager().clearEdgeRigidCM(mLLIslandHook);
		intScene.getLowLevelContext()->destroyContactManager(mManager);
		mManager = 0;

		PxsTransformCache& cache = getScene().getInteractionScene().getLowLevelContext()->getTransformCache();
		getShape0().destroyTransformCache(cache);
		getShape1().destroyTransformCache(cache);

	}
}


PX_FORCE_INLINE bool Sc::ShapeInstancePairLL::activeManagerAllowed() 
{
	PX_ASSERT(getShape0().getActorSim().isDynamicRigid() || getShape1().getActorSim().isDynamicRigid());
	
	const ActorSim& actorSim0 = getShape0().getActorSim();
	const ActorSim& actorSim1 = getShape1().getActorSim();

	PX_ASSERT(actorSim0.isDynamicRigid() || actorSim1.isDynamicRigid());

	if(actorSim0.isActive() || actorSim1.isActive())
	{
		clearFlag(ACTIVE_MANAGER_NOT_ALLOWED);
		return true;
	}
	else
	{
		//Sleeping kinematic 0 vs sleeping kinematic 1
		raiseFlag(ACTIVE_MANAGER_NOT_ALLOWED);
		return false;
	}
}


PX_FORCE_INLINE void Sc::ShapeInstancePairLL::setSweptProperties()
{
	PX_ASSERT(mManager);

	//we may want to only write these if they have changed, the set code is a bit painful for the integration flags because of bit unpacking + packing.
	mManager->setCCD((getPairFlags() & PxPairFlag::eDETECT_CCD_CONTACT) != 0);
}


PX_FORCE_INLINE void Sc::ShapeInstancePairLL::sendCCDRetouch(const PxU32 ccdPass)
{
	PxU32 pairFlags = getPairFlags();
	if (pairFlags & PxPairFlag::eNOTIFY_TOUCH_CCD)
	{
		processUserNotification(PxPairFlag::eNOTIFY_TOUCH_CCD, 0, false, ccdPass);
	}
}


PX_FORCE_INLINE void Sc::ShapeInstancePairLL::adjustCountersOnLostTouch()
{
	mActorPair.decTouchCount();

	BodySim* body0 = getShape0().getBodySim();
	BodySim* body1 = getShape1().getBodySim();
	if (body0)
		body0->decrementBodyConstraintCounter();
	if (body1)
		body1->decrementBodyConstraintCounter();
}


PX_FORCE_INLINE void Sc::ShapeInstancePairLL::adjustCountersOnNewTouch()
{
	mActorPair.incTouchCount();

	BodySim* body0 = getShape0().getBodySim();
	BodySim* body1 = getShape1().getBodySim();
	if(body0)
		body0->incrementBodyConstraintCounter();
	if(body1)
		body1->incrementBodyConstraintCounter();
}


PX_FORCE_INLINE Ps::IntBool Sc::ShapeInstancePairLL::hasKnownTouchState() const
{
	// For a pair where the bodies were added asleep, the touch state is not known until narrowphase runs on the pair for the first time.
	// If such a pair looses AABB overlap before, the conservative approach is to wake the bodies up. This method provides an indicator that
	// this is such a pair. Note: this might also wake up objects that do not touch but that's the price to pay (unless we want to run
	// overlap tests on such pairs).
	
	if (mManager)
		return mManager->touchStatusKnown();
	else
		return readIntFlag(TOUCH_KNOWN);
}


}

#endif

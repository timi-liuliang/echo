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


#ifndef PX_PHYSICS_SCP_BODYSIM
#define PX_PHYSICS_SCP_BODYSIM

#include "PsUtilities.h"
#include "PsIntrinsics.h"
#include "ScRigidSim.h"
#include "PxvDynamics.h"
#include "ScBodyCore.h"
#include "ScSimStateData.h"
#include "ScConstraintGroupNode.h"
#include "PxRigidDynamic.h"
#include "PxvArticulation.h"
#include "PxsIslandManager.h"
#include "PxsRigidBody.h"

namespace physx
{
	class PxsTransformCache;
namespace Sc
{

	class Scene;
	class ConstraintSim;
	class ArticulationSim;

	static const PxReal ScInternalWakeCounterResetValue = 20.0f*0.02f;

	class BodySim : public RigidSim
	{
	public:
		enum InternalFlags
		{
			BF_DISABLE_GRAVITY		= 1 << 0,	// Don't apply the scene's gravity

			BF_HAS_STATIC_TOUCH		= 1 << 1,	// Set when a body is part of an island with static contacts. Needed to be able to recalculate adaptive force if this changes
			BF_KINEMATIC_MOVED		= 1 << 2,	// Set when the kinematic was moved

			BF_ON_DEATHROW			= 1 << 3,	// Set when the body is destroyed

			BF_IS_IN_SLEEP_LIST		= 1 << 4,	// Set when the body is added to the list of bodies which were put to sleep
			BF_IS_IN_WAKEUP_LIST	= 1 << 5,	// Set when the body is added to the list of bodies which were woken up
			BF_SLEEP_NOTIFY			= 1 << 6,	// A sleep notification should be sent for this body (and not a wakeup event, even if the body is part of the woken list as well)
			BF_WAKEUP_NOTIFY		= 1 << 7,	// A wake up notification should be sent for this body (and not a sleep event, even if the body is part of the sleep list as well)

			BF_HAS_CONSTRAINTS		= 1 << 8,	// Set if the body has one or more constraints
			BF_KINEMATIC_SETTLING	= 1 << 9,	// Set when the body was moved kinematically last frame
			BF_KINEMATIC_MOVE_FLAGS = BF_KINEMATIC_MOVED | BF_KINEMATIC_SETTLING //Used to clear kinematic masks in 1 call

			// PT: WARNING: flags stored on 16-bits now.
		};

	public:
												BodySim(Scene&, BodyCore&);
		virtual									~BodySim();

						void					notifyAddSpatialAcceleration();
						void					notifyClearSpatialAcceleration();
						void					notifyAddSpatialVelocity();
						void					notifyClearSpatialVelocity();
						void					updateCachedTransforms(PxsTransformCache& cache);

		// hooks for actions in body core when it's attached to a sim object. Generally
		// we get called after the attribute changed.
			
		virtual			void					postActorFlagChange(PxU32 oldFlags, PxU32 newFlags);
						void					postBody2WorldChange();
						void					postSetWakeCounter(PxReal t, bool forceWakeUp);
						void					postSetKinematicTarget();
						void					postSwitchToKinematic();
						void					postSwitchToDynamic();

		PX_FORCE_INLINE const PxTransform&		getBody2World()		const	{ return getBodyCore().getCore().body2World;	}
		PX_FORCE_INLINE const PxTransform&		getBody2Actor()		const	{ return getBodyCore().getCore().body2Actor;	}
		PX_FORCE_INLINE const PxsRigidBody&		getLowLevelBody()	const	{ return mLLBody;				}
		PX_FORCE_INLINE	PxsRigidBody&			getLowLevelBody()			{ return mLLBody;				}
		PX_FORCE_INLINE bool					isSleeping()		const	{ return !isActive(); }
						void					wakeUp();  // note: for user API call purposes only, i.e., use from BodyCore. For simulation internal purposes there is internalWakeUp().
						void					putToSleep();

		static			PxU32					getRigidBodyOffset()		{ return  (PxU32)offsetof(BodySim, mLLBody);}

		//---------------------------------------------------------------------------------
		// Actor implementation
		//---------------------------------------------------------------------------------
	protected:
		virtual			void					onActivate();
		virtual			void					onDeactivate();

	private:
		//---------------------------------------------------------------------------------
		// Constraint projection
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE	ConstraintGroupNode*	getConstraintGroup()								{ return mConstraintGroup; }
		PX_FORCE_INLINE	void					setConstraintGroup(ConstraintGroupNode* node)		{ mConstraintGroup = node; }

		// A list of active projection trees in the scene might be better
		PX_FORCE_INLINE void					projectPose() { PX_ASSERT(mConstraintGroup); ConstraintGroupNode::projectPose(*mConstraintGroup); }

		//---------------------------------------------------------------------------------
		// Kinematics
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE Ps::IntBool				isKinematic()								const	{ return getBodyCore().getFlags() & PxRigidBodyFlag::eKINEMATIC ? 1 : 0; }
		PX_FORCE_INLINE bool					isArticulationLink()						const	{ return getActorType() == PxActorType::eARTICULATION_LINK; }
						void					calculateKinematicVelocity(PxReal oneOverDt);
						void					updateKinematicPose();
						bool					deactivateKinematic();

	private:
		PX_FORCE_INLINE void					initKinematicStateBase(BodyCore&);  // called as part of BodySim creation

		//---------------------------------------------------------------------------------
		// Sleeping
		//---------------------------------------------------------------------------------
	public:
						void					internalWakeUp(PxReal wakeCounterValue=ScInternalWakeCounterResetValue);
						void					internalWakeUpArticulationLink(PxReal wakeCounterValue);	// called by ArticulationSim to wake up this link
						void					sleepCheck(PxReal dt, PxReal invDt, bool enableStabilization,bool& readyForSleeping, bool& notReadyForSleeping);
						PxReal					updateWakeCounter(PxReal dt, PxReal energyThreshold, PxReal freezeThreshold, PxReal invDt, bool enableStabilization, bool& notReadyForSleeping);
						void					resetSleepFilter();
						void					notifyReadyForSleeping();			// inform the sleep island generation system that the body is ready for sleeping
						void					notifyNotReadyForSleeping();		// inform the sleep island generation system that the body is not ready for sleeping
		PX_FORCE_INLINE void					notifyWakeUpAndNotReadyForSleeping();	// inform the sleep island generation system that the body is awake and not ready for sleeping
						bool					sleepStateIntegrityCheck();
		PX_FORCE_INLINE bool					checkSleepReadinessBesidesWakeCounter();  // for API triggered changes to test sleep readiness

	private:
		PX_FORCE_INLINE	void					notifyWakeUp();					// inform the sleep island generation system that the object got woken up
		PX_FORCE_INLINE	void					notifyPutToSleep();				// inform the sleep island generation system that the object was put to sleep
		PX_FORCE_INLINE void					internalWakeUpBase(PxReal wakeCounterValue);


		//---------------------------------------------------------------------------------
		// External velocity changes
		//---------------------------------------------------------------------------------
	public:

						void					updateForces(PxReal dt, PxReal oneOverDt, bool updateGravity, const PxVec3& gravity, bool hasStaticTouch, bool simUsesAdaptiveForce);
	private:
		PX_FORCE_INLINE void					raiseVelocityModFlag(VelocityModFlags f)				{ mVelModState |= f;					}
		PX_FORCE_INLINE void					clearVelocityModFlag(VelocityModFlags f)				{ mVelModState &= ~f;					}
		PX_FORCE_INLINE bool					readVelocityModFlag(VelocityModFlags f)					{ return (mVelModState & f) != 0;		}
		PX_FORCE_INLINE void					setForcesToDefaults(bool enableGravity);

		//---------------------------------------------------------------------------------
		// Miscellaneous
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE bool					readInternalFlag(InternalFlags flag)			const	{ return (mInternalFlags & flag) != 0;	}
		PX_FORCE_INLINE void					raiseInternalFlag(InternalFlags flag)					{ mInternalFlags |= flag;				}
		PX_FORCE_INLINE void					clearInternalFlag(InternalFlags flag)					{ mInternalFlags &= ~flag;				}
		PX_FORCE_INLINE PxU32					getFlagsFast()									const	{ return getBodyCore().getFlags();		}

		PX_FORCE_INLINE void					incrementBodyConstraintCounter()						{ mBodyConstraints++;					}
		PX_FORCE_INLINE void					decrementBodyConstraintCounter()						{ PX_ASSERT(mBodyConstraints>0); mBodyConstraints--;	}

		PX_FORCE_INLINE	BodyCore&				getBodyCore()									const	{ return static_cast<BodyCore&>(getRigidCore());		}

		PX_INLINE		ArticulationSim*		getArticulation() const									{ return mArticulation; }
						void 					setArticulation(ArticulationSim* a, PxReal wakeCounter, bool asleep);

		PX_INLINE		PxsIslandManagerNodeHook&		
												getLLIslandManagerNodeHook()							{ return mLLIslandHook; }
		PX_INLINE		const PxsIslandManagerNodeHook& 
												getLLIslandManagerNodeHook() const						{ return mLLIslandHook; }

						bool					isConnectedTo(const RigidSim& other, bool& collisionDisabled) const;  // Check if connected to specified object by a constraint
		PX_FORCE_INLINE void					onConstraintAttach()									{ raiseInternalFlag(BF_HAS_CONSTRAINTS); registerUniqueInteraction();}
						void					onConstraintDetach();

		PX_FORCE_INLINE	void					onOriginShift(const PxVec3& shift)						{ mLLBody.mLastTransform.p -= shift; }

	private:


		//---------------------------------------------------------------------------------
		// Joints & joint groups
		//---------------------------------------------------------------------------------

		// This is a tree data structure that gives us the projection order of joints in which this body is the tree root.
		// note: the link of the root body is not necces. the root link due to the re-rooting of the articulation!
						ConstraintGroupNode*	mConstraintGroup;			

		//---------------------------------------------------------------------------------
		// External velocity changes
		//---------------------------------------------------------------------------------
						PxsRigidBody			mLLBody;

		// VelocityMod data allocated on the fly when the user applies velocity changes
		// which need to be accumulated.
		// VelMod dirty flags stored in BodySim so we can save ourselves the expense of looking at 
		// the separate velmod data if no forces have been set.
						PxU16					mInternalFlags;
						PxU8					mVelModState;

		//---------------------------------------------------------------------------------
		// Used by the USE_ADAPTIVE_FORCE mode only to keep track of how many constraints are on a body
		//---------------------------------------------------------------------------------
						PxU16					mBodyConstraints;			
						//PxU16					mLastBodyConstraints;

		//---------------------------------------------------------------------------------
		// Kinematic/articulation
		//---------------------------------------------------------------------------------
						ArticulationSim*		mArticulation;				// NULL if not in an articulation

		////---------------------------------------------------------------------------------
		//// Sleeping
		////---------------------------------------------------------------------------------
						PxVec3					mSleepLinVelAcc;
						PxReal					mFreezeCount;
						PxVec3					mSleepAngVelAcc;
						PxReal					mAccelScale;

		//---------------------------------------------------------------------------------
		// Island manager
		//---------------------------------------------------------------------------------
						PxsIslandManagerNodeHook mLLIslandHook;			

	};

} // namespace Sc



PX_FORCE_INLINE void Sc::BodySim::setForcesToDefaults(bool enableGravity)
{
	SimStateData* simStateData = getBodyCore().getSimStateData(false);
	if(simStateData) 
	{
		VelocityMod* velmod = simStateData->getVelocityModData();
		velmod->clear();
	}

	if (enableGravity)
		mVelModState = VMF_GRAVITY_DIRTY;	// We want to keep the gravity flag to make sure the acceleration gets changed to gravity-only
											// in the next step (unless the application adds new forces of course)
	else
		mVelModState = 0;
}


PX_FORCE_INLINE bool Sc::BodySim::checkSleepReadinessBesidesWakeCounter()
{
	const BodyCore& bodyCore = getBodyCore();
	const SimStateData* simStateData = bodyCore.getSimStateData(false);
	const VelocityMod* velmod = simStateData ? simStateData->getVelocityModData() : NULL;

	bool readyForSleep = bodyCore.getLinearVelocity().isZero() && bodyCore.getAngularVelocity().isZero();
	if (readVelocityModFlag(VMF_ACC_DIRTY))
	{
		readyForSleep = readyForSleep && (!velmod || velmod->getLinearVelModPerSec().isZero());
		readyForSleep = readyForSleep && (!velmod || velmod->getAngularVelModPerSec().isZero());
	}
	if (readVelocityModFlag(VMF_VEL_DIRTY))
	{
		readyForSleep = readyForSleep && (!velmod || velmod->getLinearVelModPerStep().isZero());
		readyForSleep = readyForSleep && (!velmod || velmod->getAngularVelModPerStep().isZero());
	}

	return readyForSleep;
}


}

#endif

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


#ifndef PX_PHYSICS_SCB_BODY
#define PX_PHYSICS_SCB_BODY

#include "ScBodyCore.h"

#include "ScbRigidObject.h"
#include "CmUtils.h"
#include "PsUtilities.h"
#include "PxRigidDynamic.h"
#include "ScbDefs.h"

namespace physx
{
namespace Scb
{
#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

struct BodyBuffer : public RigidObjectBuffer		//once RigidObject has its own buffered elements, derive from that instead
{
	template <PxU32 I, PxU32 Dummy> struct Fns {};  // TODO: make the base class traits visible
	typedef Sc::BodyCore Core;
	typedef BodyBuffer Buf;

	// regular attributes
	SCB_REGULAR_ATTRIBUTE(0,		PxReal,			InverseMass)
	SCB_REGULAR_ATTRIBUTE(1,		PxVec3,			InverseInertia)
	SCB_REGULAR_ATTRIBUTE(2,		PxReal,			LinearDamping)
	SCB_REGULAR_ATTRIBUTE(3,		PxReal,			AngularDamping)
	SCB_REGULAR_ATTRIBUTE(4,		PxReal,			MaxAngVelSq)
	SCB_REGULAR_ATTRIBUTE(5,		PxReal,			SleepThreshold)
	SCB_REGULAR_ATTRIBUTE(6,		PxReal,			CCDAdvanceCoefficient)
	SCB_REGULAR_ATTRIBUTE(7,		PxReal,			ContactReportThreshold)
	SCB_REGULAR_ATTRIBUTE(8,		PxU16,			SolverIterationCounts)
	SCB_REGULAR_ATTRIBUTE_ALIGNED(9,PxTransform,	Body2Actor, 16)
	SCB_REGULAR_ATTRIBUTE(10,		PxReal,			MaxPenetrationBias)
	SCB_REGULAR_ATTRIBUTE(11,		PxReal,			FreezeThreshold)

	// irregular attributes

	PX_ALIGN(16, PxTransform) mKinematicTarget;
	PxVec3		mLinAcceleration;
	PxVec3		mAngAcceleration;
	PxVec3		mLinDeltaVelocity;
	PxVec3		mAngDeltaVelocity;

	PxRigidBodyFlags mRigidBodyFlags;

	enum
	{
		BF_RigidBodyFlags		= 1<<12,
		BF_KinematicTarget		= 1<<13,
		BF_AccelerationLinear	= 1<<14,
		BF_AccelerationAngular	= 1<<15,
		BF_Acceleration			= BF_AccelerationLinear|BF_AccelerationAngular,
		BF_DeltaVelocityLinear	= 1<<16,
		BF_DeltaVelocityAngular	= 1<<17,
		BF_DeltaVelocity		= BF_DeltaVelocityLinear|BF_DeltaVelocityAngular,
		BF_Body2World			= 1<<18,
		BF_Body2World_CoM		= 1<<19,  // the body pose was adjusted because of a center of mass change only
		BF_LinearVelocity		= 1<<20,
		BF_AngularVelocity		= 1<<21,
		BF_WakeCounter			= 1<<22,
		BF_PutToSleep			= 1<<23,
		BF_WakeUp				= 1<<24
	};

	BodyBuffer(): mLinAcceleration(0), mAngAcceleration(0), mLinDeltaVelocity(0), mAngDeltaVelocity(0) {}
};


#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

class Body : public Scb::RigidObject
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	typedef BodyBuffer Buf;
	typedef Sc::BodyCore Core;

public:
// PX_SERIALIZATION
										Body(const PxEMPTY&) :	Scb::RigidObject(PxEmpty), mBodyCore(PxEmpty), mBufferedIsSleeping(1) { PX_ASSERT(mBodyBufferFlags == 0); }
	static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
	PX_INLINE							Body(PxActorType::Enum type, const PxTransform& bodyPose);
	PX_INLINE							~Body() {}

	//---------------------------------------------------------------------------------
	// Wrapper for Sc::BodyCore interface
	//---------------------------------------------------------------------------------


	PX_FORCE_INLINE const PxTransform&	getBody2World()		const			{ return mBufferedBody2World;	}	// PT: important: keep returning an address here (else update prefetch in SceneQueryManager::addShapes)
	PX_INLINE		void				setBody2World(const PxTransform& p, bool asPartOfBody2ActorChange);

	PX_FORCE_INLINE const PxVec3&		getLinearVelocity()		const		{ return mBufferedLinVelocity;	}
	PX_INLINE		void				setLinearVelocity(const PxVec3& v);
	PX_FORCE_INLINE const PxVec3&		getAngularVelocity()	const		{ return mBufferedAngVelocity;	}
	PX_INLINE		void				setAngularVelocity(const PxVec3& v);

	PX_FORCE_INLINE	void				wakeUp();
	PX_FORCE_INLINE	void				putToSleep();
	PX_FORCE_INLINE PxReal				getWakeCounter()	const			{ return mBufferedWakeCounter;	}
	PX_INLINE		void				setWakeCounter(PxReal w);
	PX_FORCE_INLINE	bool				isSleeping() const					{ return (mBufferedIsSleeping != 0); }

	PX_INLINE		const PxTransform&	getBody2Actor() const				{ return read<Buf::BF_Body2Actor>(); }
	PX_INLINE		void				setBody2Actor(const PxTransform& m)	{ write<Buf::BF_Body2Actor>(m); }

	PX_INLINE		PxReal				getInverseMass() const				{ return read<Buf::BF_InverseMass>(); }
	PX_INLINE		void				setInverseMass(PxReal m)			{ write<Buf::BF_InverseMass>(m); }

	PX_INLINE		PxVec3				getInverseInertia() const			{ return read<Buf::BF_InverseInertia>(); }
	PX_INLINE		void				setInverseInertia(const PxVec3& i)  { write<Buf::BF_InverseInertia>(i); }

	PX_INLINE		PxReal				getLinearDamping() const			{ return read<Buf::BF_LinearDamping>(); }
	PX_INLINE		void				setLinearDamping(PxReal d)			{ write<Buf::BF_LinearDamping>(d); }

	PX_INLINE		PxReal				getAngularDamping() const			{ return read<Buf::BF_AngularDamping>(); }
	PX_INLINE		void				setAngularDamping(PxReal d)			{ write<Buf::BF_AngularDamping>(d); }

	PX_INLINE		PxRigidBodyFlags	getFlags() const					{ return (isBuffered(Buf::BF_RigidBodyFlags)) ? getBodyBuffer()->mRigidBodyFlags : mBodyCore.getFlags(); }
	PX_INLINE		void				setFlags(PxRigidBodyFlags f);

	PX_INLINE		PxReal				getSleepThreshold() const			{ return read<Buf::BF_SleepThreshold>(); }
	PX_INLINE		void				setSleepThreshold(PxReal t)			{ write<Buf::BF_SleepThreshold>(t); }

	PX_INLINE		PxReal				getFreezeThreshold() const			{ return read<Buf::BF_FreezeThreshold>(); }
	PX_INLINE		void				setFreezeThreshold(PxReal t)		{ write<Buf::BF_FreezeThreshold>(t); }

	PX_INLINE		PxReal				getMaxPenetrationBias() const		{ return read<Buf::BF_MaxPenetrationBias>(); }
	PX_INLINE		void				setMaxPenetrationBias(PxReal t)		{ write<Buf::BF_MaxPenetrationBias>(t); }

	PX_INLINE		PxReal				getMaxAngVelSq() const				{ return read<Buf::BF_MaxAngVelSq>(); }
	PX_INLINE		void				setMaxAngVelSq(PxReal v)			{ write<Buf::BF_MaxAngVelSq>(v); }

	PX_INLINE		PxU16				getSolverIterationCounts() const	{ return Ps::to16(read<Buf::BF_SolverIterationCounts>()); }
	PX_INLINE		void				setSolverIterationCounts(PxU16 c)	{ write<Buf::BF_SolverIterationCounts>(c); }

	PX_INLINE		PxReal				getContactReportThreshold() const	{ return read<Buf::BF_ContactReportThreshold>(); }
	PX_INLINE		void				setContactReportThreshold(PxReal t) { write<Buf::BF_ContactReportThreshold>(t); }

	PX_INLINE		void				addSpatialAcceleration(const PxVec3* linAcc, const PxVec3* angAcc);
	PX_INLINE		void				clearSpatialAcceleration(bool force, bool torque);
	PX_INLINE		void				addSpatialVelocity(const PxVec3* linVelDelta, const PxVec3* angVelDelta);
	PX_INLINE		void				clearSpatialVelocity(bool force, bool torque);

	PX_INLINE		bool				getKinematicTarget(PxTransform& p) const;
	PX_INLINE		void				setKinematicTarget(const PxTransform& p);

	PX_INLINE		void				setMinCCDAdvanceCoefficient(PxReal minCCDAdvanceCoefficient){write<Buf::BF_CCDAdvanceCoefficient>(minCCDAdvanceCoefficient);}
	PX_INLINE		PxReal				getMinCCDAdvanceCoefficient() const { return read<Buf::BF_CCDAdvanceCoefficient>();}

	PX_FORCE_INLINE	void				onOriginShift(const PxVec3& shift);

	//---------------------------------------------------------------------------------
	// Data synchronization
	//---------------------------------------------------------------------------------
	PX_INLINE		void				syncState();

	static size_t getScOffset()													
	{ 
		return reinterpret_cast<size_t>(&reinterpret_cast<Body*>(0)->mBodyCore);
	}

	/**
	\brief Shadowed method of #Scb::Base::markUpdated() to store the buffered property flags in a separate location (ran out of flag space)
	*/
	PX_FORCE_INLINE	void				markUpdated(PxU32 flag);

	/**
	\brief Shadowed method of #Scb::Base::isBuffered() to check the buffered property flags (ran out of flag space)
	*/
	PX_FORCE_INLINE	Ps::IntBool			isBuffered(PxU32 flag) const;

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
public:
	PX_FORCE_INLINE	const Sc::BodyCore&		getScBody()	const		{ return mBodyCore; }  // Only use if you know what you're doing!
	PX_FORCE_INLINE	Sc::BodyCore&			getScBody()				{ return mBodyCore; }  // Only use if you know what you're doing!

	PX_FORCE_INLINE static const Body&		fromSc(const Core& a)	{ return static_cast<const Body&>(Actor::fromSc(a)); }
	PX_FORCE_INLINE static Body&			fromSc(Core &a)			{ return static_cast<Body&>(Actor::fromSc(a)); }

	PX_FORCE_INLINE bool					hasKinematicTarget() const;
	PX_FORCE_INLINE void					clearSimStateDataForPendingInsert();
	PX_FORCE_INLINE void					transitionSimStateDataForPendingInsert();

	PX_INLINE		PxMat33					getGlobalInertiaTensorInverse() const;

	PX_FORCE_INLINE bool					checkSleepReadinessBesidesWakeCounter();
	PX_FORCE_INLINE	void					initBufferedState();
	PX_FORCE_INLINE	void					clearBufferedState();
	PX_FORCE_INLINE	void					clearBufferedSleepStateChange();

	PX_INLINE		void					wakeUpInternal(PxReal wakeCounter);
	PX_INLINE		void					putToSleepInternal();

	PX_FORCE_INLINE void					switchBodyToNoSim();

private:
					Sc::BodyCore		mBodyCore;
	//---------------------------------------------------------------------------------
	// Permanently buffered data (simulation written data)
	//---------------------------------------------------------------------------------
					PxTransform			mBufferedBody2World;
					PxVec3				mBufferedLinVelocity;
					PxVec3				mBufferedAngVelocity;
					PxReal				mBufferedWakeCounter;
					PxU32				mBufferedIsSleeping;	// Note: If the object is not in a scene this value must be true, i.e., positive
																// Don't need 4 bytes but otherwise there is padding here anyway.
					PxU32				mBodyBufferFlags;  // Stores the buffered property flags since there is not enough space in usual location in Scb::Base.

	PX_FORCE_INLINE	const Buf*		getBodyBuffer()	const	{ return (const Buf*)getStream();	}
	PX_FORCE_INLINE	Buf*			getBodyBuffer()			{ return (Buf*)getStream();			}

	PX_INLINE void accumulate(PxVec3& linear, PxVec3& angular, PxU32 linearFlag, PxU32 angularFlag, const PxVec3* linIncrement, const PxVec3* angIncrement)
	{
		PxU32 flag = 0;
		if(linIncrement)
		{
			linear += *linIncrement;
			flag |= linearFlag;
		}

		if(angIncrement)
		{
			angular += *angIncrement;
			flag |= angularFlag;
		}

		markUpdated(flag);
	}
		
	PX_INLINE void resetAccumulator(PxVec3& linear, PxVec3& angular, PxU32 linearFlag, PxU32 angularFlag, bool force, bool torque)
	{
		PxU32 flags = mBodyBufferFlags;
		if(force)
		{
			linear = PxVec3(0.0f);
			flags &= ~linearFlag;
		}

		if(torque)
		{
			angular = PxVec3(0.0f);
			flags &= ~angularFlag;
		}
		mBodyBufferFlags = flags;
	}

	PX_FORCE_INLINE void setBufferedParamsForAsleep()  // use this in the non-buffered case to set the buffered properties
	{
		mBufferedIsSleeping = 1;
		mBufferedWakeCounter = 0.0f;
		mBufferedLinVelocity = PxVec3(0.0f);
		mBufferedAngVelocity = PxVec3(0.0f);
		// no need to clear forces since that will be the job of the corresponding core/sim methods
	}

	PX_FORCE_INLINE void setBufferedParamsForAwake(PxReal wakeCounter)  // use this in the non-buffered case to set the buffered properties
	{
		mBufferedIsSleeping = 0;
		mBufferedWakeCounter = wakeCounter;
	}

	//---------------------------------------------------------------------------------
	// Infrastructure for regular attributes
	//---------------------------------------------------------------------------------

	struct Access: public BufferedAccess<Buf, Core, Body, Body> {};

	template<PxU32 f> PX_FORCE_INLINE typename Buf::Fns<f,0>::Arg read() const		{	return Access::read<Buf::Fns<f,0> >(*this, mBodyCore);	}
	template<PxU32 f> PX_FORCE_INLINE void write(typename Buf::Fns<f,0>::Arg v)		{	Access::write<Buf::Fns<f,0> >(*this, mBodyCore, v);		}
	template<PxU32 f> PX_FORCE_INLINE void flush(const Buf& buf)					{	Access::flush<Buf::Fns<f,0> >(*this, mBodyCore, buf);	}

};


PX_INLINE Body::Body(PxActorType::Enum type, const PxTransform& bodyPose) : mBodyCore(type, bodyPose)
{
	setScbType(ScbType::BODY);

	mBufferedBody2World		= mBodyCore.getBody2World();
	mBufferedLinVelocity	= mBodyCore.getLinearVelocity();
	mBufferedAngVelocity	= mBodyCore.getAngularVelocity();
	mBufferedWakeCounter	= mBodyCore.getWakeCounter();
	mBufferedIsSleeping		= 1;  // this is the specified value for free standing objects
	mBodyBufferFlags		= 0;
}

PX_INLINE void Body::setBody2World(const PxTransform& p, bool asPartOfBody2ActorChange)
{
	mBufferedBody2World = p;

	if (!isBuffering())
	{
		mBodyCore.setBody2World(p);
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else
	{
		if (!asPartOfBody2ActorChange)
		{
			// call was triggered by a setGlobalPose(). This means the simulated body pose will get
			// overwritten by the user value, so we do not need to adjust it.

			mBodyBufferFlags &= ~Buf::BF_Body2World_CoM;
		}
		else if (!(mBodyBufferFlags & Buf::BF_Body2World))
		{
			// there has been no setGlobalPose() on the body yet and the center of mass changes.
			// This case needs special treatment because the simulation results for such a body will be based on
			// the old center of mass but need to get translated to the new center of mass.

			mBodyBufferFlags |= Buf::BF_Body2World_CoM;
		}

		markUpdated(Buf::BF_Body2World);
	}
}

PX_INLINE void Body::setLinearVelocity(const PxVec3& v)
{
	mBufferedLinVelocity = v;

	if (!isSimulating())
	{
		mBodyCore.setLinearVelocity(v);
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else
		markUpdated(Buf::BF_LinearVelocity);
}

PX_INLINE void Body::setAngularVelocity(const PxVec3& v)
{
	mBufferedAngVelocity = v;

	if (!isSimulating())
	{
		mBodyCore.setAngularVelocity(v);
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else
		markUpdated(Buf::BF_AngularVelocity);
}


PX_INLINE void Body::wakeUpInternal(PxReal wakeCounter)
{
	PX_ASSERT(getScbScene());

	if (!isSimulating())
	{
		setBufferedParamsForAwake(wakeCounter);
		mBodyCore.wakeUp(wakeCounter);
	}
	else
	{
		mBufferedIsSleeping = 0;
		mBufferedWakeCounter = wakeCounter;
		markUpdated(Buf::BF_WakeUp | Buf::BF_WakeCounter);
		mBodyBufferFlags &= ~Buf::BF_PutToSleep;
	}
}

PX_FORCE_INLINE void Body::wakeUp()
{
	PX_ASSERT(!(getFlags() & PxRigidBodyFlag::eKINEMATIC));
	Scene* scene = getScbScene();
	PX_ASSERT(scene);  // only allowed for an object in a scene

	wakeUpInternal(scene->getWakeCounterResetValue());
}


PX_INLINE void Body::putToSleepInternal()
{
	if (!isSimulating())
	{
		setBufferedParamsForAsleep();
		mBodyCore.putToSleep();
	}
	else
	{
		mBufferedIsSleeping = 1;
		mBufferedWakeCounter = 0.0f;
		// it is necessary to set the velocities as a buffered operation (not just adjust the buffered velocities) because
		// a putToSleep can be followed by a wakeUp in which case only the wakeUp will get processed on sync, however, the velocities
		// still need to get set to 0.
		setLinearVelocity(PxVec3(0.0f));
		setAngularVelocity(PxVec3(0.0f));
		mBodyBufferFlags &= ~(Buf::BF_Acceleration | Buf::BF_DeltaVelocity | Buf::BF_KinematicTarget);

		markUpdated(Buf::BF_PutToSleep | Buf::BF_WakeCounter);
		mBodyBufferFlags &= ~Buf::BF_WakeUp;
	}
}


PX_FORCE_INLINE void Body::putToSleep()
{
	PX_ASSERT(!(getFlags() & PxRigidBodyFlag::eKINEMATIC));

	putToSleepInternal();
}


PX_INLINE void Body::setWakeCounter(PxReal w)
{
	PX_ASSERT(!(getFlags() & PxRigidBodyFlag::eKINEMATIC));

	mBufferedWakeCounter = w;

	if (!isSimulating())
	{
		if (getScbScene() && (w > 0.0f))
			mBufferedIsSleeping = 0;

		mBodyCore.setWakeCounter(w);
		
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else
	{
		if (w > 0.0f)
			wakeUpInternal(w);
		else
			markUpdated(Buf::BF_WakeCounter);
	}
}


PX_INLINE void Body::setFlags(PxRigidBodyFlags f)
{
	PxU32 wasKinematic = getFlags() & PxRigidBodyFlag::eKINEMATIC;
	PxU32 isKinematic = f & PxRigidBodyFlag::eKINEMATIC;
	bool switchToKinematic = ((!wasKinematic) && isKinematic);
	bool switchToDynamic = (wasKinematic && (!isKinematic));

	if (!isBuffering())
	{
		if (switchToKinematic)
			setBufferedParamsForAsleep();

		mBodyCore.setFlags(getScbScene() ? getScbScene()->getScScene().getSimStateDataPool() : NULL, f);
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else if (fetchDataBuffer())
	{
		if (switchToKinematic)
			putToSleepInternal();
		else if (switchToDynamic)
			mBodyBufferFlags &= ~Buf::BF_KinematicTarget;

		getBodyBuffer()->mRigidBodyFlags = f;
		markUpdated(Buf::BF_RigidBodyFlags);
	}
}


PX_INLINE void Body::addSpatialAcceleration(const PxVec3* linAcc, const PxVec3* angAcc)
{
	if (!isSimulating())
	{
		mBodyCore.addSpatialAcceleration(getScbScene()->getScScene().getSimStateDataPool(), linAcc, angAcc);
		//Spatial acceleration isn't sent to PVD.
	}
	else if (fetchDataBuffer())
	{
		Buf* b = getBodyBuffer();
		accumulate(b->mLinAcceleration, b->mAngAcceleration, Buf::BF_AccelerationLinear, Buf::BF_AccelerationAngular, linAcc, angAcc);
	}
}


PX_INLINE void Body::clearSpatialAcceleration(bool force, bool torque)
{
	if (!isSimulating())
	{
		mBodyCore.clearSpatialAcceleration(force, torque);
		//Spatial acceleration isn't sent to PVD.
	}
	else if (fetchDataBuffer())
	{
		Buf* b = getBodyBuffer();
		resetAccumulator(b->mLinAcceleration, b->mAngAcceleration, Buf::BF_AccelerationLinear, Buf::BF_AccelerationAngular, force, torque);
	}
}


PX_INLINE void Body::addSpatialVelocity(const PxVec3* linVelDelta, const PxVec3* angVelDelta)
{
	if (!isSimulating())
	{
		mBodyCore.addSpatialVelocity(getScbScene()->getScScene().getSimStateDataPool(), linVelDelta, angVelDelta);
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else if (fetchDataBuffer())
	{
		Buf* b = getBodyBuffer();
		accumulate(b->mLinDeltaVelocity, b->mAngDeltaVelocity, Buf::BF_DeltaVelocityLinear, Buf::BF_DeltaVelocityAngular, linVelDelta, angVelDelta);
	}
}


PX_INLINE void Body::clearSpatialVelocity(bool force, bool torque)
{
	if (!isSimulating())
	{
		mBodyCore.clearSpatialVelocity(force, torque);
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else if (fetchDataBuffer())
	{
		Buf* b = getBodyBuffer();
		resetAccumulator(b->mLinDeltaVelocity, b->mAngDeltaVelocity, Buf::BF_DeltaVelocityLinear, Buf::BF_DeltaVelocityAngular, force, torque);
	}
}


PX_INLINE bool Body::getKinematicTarget(PxTransform& p) const
{
	if (isBuffered(Buf::BF_KinematicTarget))
	{
		p = getBodyBuffer()->mKinematicTarget;
		return true;
	}
	else if (getControlState() != ControlState::eREMOVE_PENDING)
		return mBodyCore.getKinematicTarget(p);
	else
		return false;
}


PX_INLINE void Body::setKinematicTarget(const PxTransform& p)
{
	Scene* scene = getScbScene();
	PX_ASSERT(scene);  // only allowed for an object in a scene
	PxReal wakeCounterResetValue = scene->getWakeCounterResetValue();

	if (!isSimulating())
	{
		mBodyCore.setKinematicTarget(scene->getScScene().getSimStateDataPool(), p, wakeCounterResetValue);
		setBufferedParamsForAwake(wakeCounterResetValue);

		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else if (fetchDataBuffer())
	{
		PX_ASSERT((mBodyBufferFlags & (Buf::BF_DeltaVelocity|Buf::BF_Acceleration)) == 0);  // switching to kinematic should do that.
		getBodyBuffer()->mKinematicTarget = p;
		markUpdated(Buf::BF_KinematicTarget);

		wakeUpInternal(wakeCounterResetValue);
	}
#if PX_SUPPORT_VISUAL_DEBUGGER
	if(getControlState() == ControlState::eIN_SCENE)
	{
		if(scene->getSceneVisualDebugger().isConnected(true))	
			 scene->getSceneVisualDebugger().updateKinematicTarget(this, p);
	}
#endif
}


PX_FORCE_INLINE	void Body::onOriginShift(const PxVec3& shift)
{
	mBufferedBody2World.p -= shift;
	mBodyCore.onOriginShift(shift);
}



//--------------------------------------------------------------
//
// Miscellaneous
//
//--------------------------------------------------------------

PX_FORCE_INLINE bool Body::hasKinematicTarget() const
{
	return 
	(
		isBuffered(BodyBuffer::BF_KinematicTarget) || mBodyCore.getHasValidKinematicTarget()
	);
}


PX_FORCE_INLINE void Body::clearSimStateDataForPendingInsert()
{
	Sc::BodyCore& core = getScBody();
	if (insertPending())
	{
		// not-so-nice-code to cover the following cases:
		// - user adds a kinematic to the scene, sets a target and removes the kinematic from scene again (all while the sim is running)
		// - same as above but instead of removing the kinematic it gets switched to dynamic
		// - user adds a dynamic to the scene, sets a target and removes the dynamic from scene again (all while the sim is running)

		if(core.getSimStateData(true))
			core.tearDownSimStateData(getScbScene()->getScScene().getSimStateDataPool(), true);
		else if(core.getSimStateData(false))
			core.tearDownSimStateData(getScbScene()->getScScene().getSimStateDataPool(), false);
	}
}


PX_FORCE_INLINE void Body::transitionSimStateDataForPendingInsert()
{
	Sc::BodyCore& core = getScBody();
	if (insertPending())
	{
		// not-so-nice-code to cover the following case:
		// - user adds a dynamic, adds force, then switches to kinematic (all while the sim is running)

		if(core.getSimStateData(false))
		{
			core.setupSimStateData(getScbScene()->getScScene().getSimStateDataPool(), true);  // note: this won't allocate the memory twice
		}
	}
}


PX_INLINE PxMat33 Body::getGlobalInertiaTensorInverse() const
{
	PxMat33 inverseInertiaWorldSpace;
	Cm::transformInertiaTensor(getInverseInertia(), PxMat33(getBody2World().q), inverseInertiaWorldSpace);
	return inverseInertiaWorldSpace;
}


PX_FORCE_INLINE bool Body::checkSleepReadinessBesidesWakeCounter()
{
	return (getLinearVelocity().isZero() && getAngularVelocity().isZero());
	// no need to test for pending force updates yet since currently this is not supported on scene insertion
}


PX_FORCE_INLINE void Body::initBufferedState()
{
	PX_ASSERT(mBufferedIsSleeping);  // this method is only meant to get called when an object is added to the scene
	
	if ((getWakeCounter() == 0.0f) && checkSleepReadinessBesidesWakeCounter())
		mBufferedIsSleeping = 1;
	else
		mBufferedIsSleeping = 0;
}


PX_FORCE_INLINE void Body::clearBufferedState()
{
	if (!(getFlags() & PxRigidBodyFlag::eKINEMATIC))
	{
		mBufferedIsSleeping = 1;  // the expected state when an object gets removed from the scene.
		mBodyBufferFlags &= ~(Buf::BF_Acceleration | Buf::BF_DeltaVelocity);
	}
	else
	{
		// make sure the buffered properties for a kinematic that is not in a scene are set according to the specification

		// necessary to use the putToSleep method because buffered re-insertion case needs to be covered as well. Currently, re-insertion
		// just clears the remove operation. This would prevent the core object parameters to get updated. Thus the operations need
		// to be buffered and putToSleepInternal takes care of that.
		putToSleepInternal();
	}

	RigidObject::clearBufferedState();
}


PX_FORCE_INLINE	void Body::clearBufferedSleepStateChange()
{
	mBodyBufferFlags &= ~(Buf::BF_WakeUp | Buf::BF_PutToSleep);
}


PX_FORCE_INLINE void Body::switchBodyToNoSim()
{
	Scb::Scene* scene = getScbScene();

	switchToNoSim(true);

	if ((!scene) || (!getScbScene()->isPhysicsBuffering()))
	{
		setBufferedParamsForAsleep();
		getScBody().putToSleep();
	}
	else
		putToSleepInternal();

	if (scene)
		clearSimStateDataForPendingInsert();
}


//--------------------------------------------------------------
//
// Data synchronization
//
//--------------------------------------------------------------

PX_FORCE_INLINE	void Body::markUpdated(PxU32 flag)
{
	scheduleForUpdate();
	mBodyBufferFlags |= flag;
}

PX_FORCE_INLINE	Ps::IntBool Body::isBuffered(PxU32 flag) const
{ 
	return Ps::IntBool(mBodyBufferFlags & flag);	
}

PX_INLINE void Body::syncState()
{
	// if the body was removed from the scene, we expect...
	// ...it to be marked as sleeping (that is the specification)
	// ...no pending sleep state change (because wakeUp/putTosleep is illegal for a free standing object and we clear such operations if pending).
	//    Note: a sleep state change might have happened before the removal but the corresponding wake counter change is then covered through the BF_WakeCounter dirty flag.
	PX_ASSERT(	(getControlState() != ControlState::eREMOVE_PENDING) || 
				(mBufferedIsSleeping && (!isBuffered(Buf::BF_WakeUp | Buf::BF_PutToSleep))) );

	//
	// IMPORTANT: Since we ran out of space for buffered property flags, the Scb::Body property related flags are stored in mBodyBufferFlags.
	//            To get the buffer flags from the base classes, use getBufferFlags()
	//
	const PxU32 bufferFlags = mBodyBufferFlags;
	const PxU32 baseBufferFlags = getBufferFlags();

	if ((bufferFlags & Buf::BF_Body2World) == 0)
		mBufferedBody2World = mBodyCore.getBody2World();
	else if ((bufferFlags & Buf::BF_Body2World_CoM) == 0)
		mBodyCore.setBody2World(mBufferedBody2World);
	else
	{
		// IMPORTANT: Do this before adjusting body2Actor
		PX_ASSERT(bufferFlags & Buf::BF_Body2Actor);
		Buf& buffer = *getBodyBuffer();
		const PxTransform newBody2oldBody = mBodyCore.getBody2Actor().transformInv(buffer.mBody2Actor);

		PxTransform b2w = mBodyCore.getBody2World();
		b2w = b2w.transform(newBody2oldBody);  // translate simulation result from old CoM to new CoM

		mBufferedBody2World = b2w;
		mBodyCore.setBody2World(b2w);
	}

	//----

	if ((bufferFlags & Buf::BF_LinearVelocity) == 0)
		mBufferedLinVelocity = mBodyCore.getLinearVelocity();
	else
	{
		PX_ASSERT(	(mBufferedIsSleeping && mBufferedLinVelocity.isZero()) || 
					(!mBufferedIsSleeping) ||
					(getControlState() == ControlState::eREMOVE_PENDING));
		PX_ASSERT(	mBufferedLinVelocity.isZero() || 
					((!mBufferedLinVelocity.isZero()) && (!mBufferedIsSleeping)) ||
					(getControlState() == ControlState::eREMOVE_PENDING));

		mBodyCore.setLinearVelocity(mBufferedLinVelocity);
	}

	//----

	if ((bufferFlags & Buf::BF_AngularVelocity) == 0)
		mBufferedAngVelocity = mBodyCore.getAngularVelocity();
	else
	{
		PX_ASSERT(	(mBufferedIsSleeping && mBufferedAngVelocity.isZero()) || 
					(!mBufferedIsSleeping) ||
					(getControlState() == ControlState::eREMOVE_PENDING));
		PX_ASSERT(	mBufferedAngVelocity.isZero() || 
					((!mBufferedAngVelocity.isZero()) && (!mBufferedIsSleeping)) ||
					(getControlState() == ControlState::eREMOVE_PENDING));

		mBodyCore.setAngularVelocity(mBufferedAngVelocity);
	}
	//----

	if (baseBufferFlags & Buf::BF_ActorFlags)
		syncNoSimSwitch(*getBodyBuffer(), mBodyCore, true);



	//----

	if(bufferFlags & ~(	Buf::BF_WakeCounter|Buf::BF_Body2World|Buf::BF_LinearVelocity|Buf::BF_AngularVelocity
		|Buf::BF_WakeUp|Buf::BF_PutToSleep))  // Optimization to avoid all the if-statements below if possible
	{
		Buf& buffer = *getBodyBuffer();

		flush<Buf::BF_InverseMass>(buffer);
		flush<Buf::BF_InverseInertia>(buffer);
		flush<Buf::BF_LinearDamping>(buffer);
		flush<Buf::BF_AngularDamping>(buffer);
		flush<Buf::BF_MaxAngVelSq>(buffer);
		flush<Buf::BF_SleepThreshold>(buffer);
		flush<Buf::BF_SolverIterationCounts>(buffer);
		flush<Buf::BF_ContactReportThreshold>(buffer);
		flush<Buf::BF_Body2Actor>(buffer);
		flush<Buf::BF_FreezeThreshold>(buffer);
		flush<Buf::BF_MaxPenetrationBias>(buffer);
		Ps::Pool<Sc::SimStateData>* pool = getScbScene()->getScScene().getSimStateDataPool();
		if (bufferFlags & Buf::BF_RigidBodyFlags)
		{
			mBodyCore.setFlags(pool, buffer.mRigidBodyFlags);
		}

		if (bufferFlags & Buf::BF_KinematicTarget)
		{
			PX_ASSERT(mBodyCore.getFlags() & PxRigidBodyFlag::eKINEMATIC);
			PX_ASSERT(mBufferedWakeCounter > 0.0f);  // that is the expected behavior
			mBodyCore.setKinematicTarget(pool, buffer.mKinematicTarget, mBufferedWakeCounter);
		}

		if (bufferFlags & Buf::BF_Acceleration)
		{
			PX_ASSERT(!(mBodyCore.getFlags() & PxRigidBodyFlag::eKINEMATIC));
			PX_ASSERT(!mBufferedIsSleeping || (buffer.mLinAcceleration.isZero() && buffer.mAngAcceleration.isZero()));
			mBodyCore.addSpatialAcceleration(pool, &buffer.mLinAcceleration, &buffer.mAngAcceleration);
		}

		if (bufferFlags & Buf::BF_DeltaVelocity)
		{
			PX_ASSERT(!(mBodyCore.getFlags() & PxRigidBodyFlag::eKINEMATIC));
			PX_ASSERT(!mBufferedIsSleeping || (buffer.mLinDeltaVelocity.isZero() && buffer.mAngDeltaVelocity.isZero()));
			mBodyCore.addSpatialVelocity(pool, &buffer.mLinDeltaVelocity, &buffer.mAngDeltaVelocity);
		}
	}

	//----

	if ((bufferFlags & Buf::BF_WakeCounter) == 0)
		mBufferedWakeCounter = mBodyCore.getWakeCounter();
	else if (!(bufferFlags & (Buf::BF_WakeUp | Buf::BF_PutToSleep)))	// if there has been at least one buffered sleep state transition, then there is no use in adjusting the wake counter separately because it will
																		// get done in the sleep state update.
	{
		PX_ASSERT((getControlState() == ControlState::eREMOVE_PENDING) || (mBufferedWakeCounter == 0.0f));  // a wake counter change is always connected to a sleep state change, except if setWakeCounter(0.0f) was called or an object gets removed from the scene after it was woken up.

		mBodyCore.setWakeCounter(mBufferedWakeCounter);
	}

	//----

	bool isSimObjectSleeping = mBodyCore.isSleeping();
	if ((bufferFlags & (Buf::BF_WakeUp | Buf::BF_PutToSleep)) == 0)
	{
		if (getControlState() != ControlState::eREMOVE_PENDING)  // we do not want to sync the simulation sleep state if the object was removed (free standing objects have buffered state sleeping)
			mBufferedIsSleeping = (PxU32)isSimObjectSleeping;
		else
			PX_ASSERT(mBufferedIsSleeping);  // this must get set immediately at remove
	}
	else
	{
		PX_ASSERT(bufferFlags & Buf::BF_WakeCounter);  // sleep state transitions always mark the wake counter dirty
		PX_ASSERT(getControlState() != ControlState::eREMOVE_PENDING);  // removing an object should clear pending wakeUp/putToSleep operations since the state for a free standing object gets set according to specification.

		// The sleep state ends up with the proper result that reflects the order of the original buffered operations because...
		// - every operation that affects the sleep state makes a buffered call to wakeUp/putToSleep, hence, the buffered sleep transition
		//   will always reflect the latest change
		// - a user triggered sleep state transition (wakeUp/putToSleep) always adjusts the affected properties (velocity, force, wake counter...) through separate
		//   buffered calls, hence, those properties will get adjusted to the correct values in the end
		// - sleep state sync runs after all calls that have side effects on the sleep state.
		//

		if (bufferFlags & Buf::BF_PutToSleep)
		{
			PX_ASSERT(mBufferedIsSleeping);
			PX_ASSERT(!(bufferFlags & Buf::BF_WakeUp));
			PX_ASSERT(mBufferedWakeCounter == 0.0f);
			PX_ASSERT(mBufferedLinVelocity.isZero());
			PX_ASSERT(mBufferedAngVelocity.isZero());
			PX_ASSERT(!(bufferFlags & Buf::BF_Acceleration));
			PX_ASSERT(!(bufferFlags & Buf::BF_DeltaVelocity));

			mBodyCore.putToSleep();
		}
		else
		{
			PX_ASSERT(!mBufferedIsSleeping);
			PX_ASSERT(bufferFlags & Buf::BF_WakeUp);

			// can not assert for any values here since it is possible, for example, to end up waking something up with a 0 wakeCounter here (as a consequence of a buffered wakeUp() followed by a setWakeCounter(0))

			mBodyCore.wakeUp(mBufferedWakeCounter);
		}
	}

	// PT: we must call this even when there's no buffered data
	RigidObject::syncState();

	// --------------
	// postconditions
	//
	PX_ASSERT((getControlState() != ControlState::eREMOVE_PENDING) || mBufferedIsSleeping);  // nothing in this method should change this
#ifdef _DEBUG
	// make sure that for a removed kinematic, the buffered params hold the values as defined in our specification
	if ((mBodyCore.getFlags() & PxRigidBodyFlag::eKINEMATIC) && (getControlState() == ControlState::eREMOVE_PENDING))
	{
		PX_ASSERT(mBufferedLinVelocity.isZero());
		PX_ASSERT(mBufferedAngVelocity.isZero());
		PX_ASSERT(mBufferedWakeCounter == 0.0f);
	}
#endif
	// 
	// postconditions
	// --------------

	postSyncState();
	mBodyBufferFlags = 0;
}

}  // namespace Scb

}

#endif

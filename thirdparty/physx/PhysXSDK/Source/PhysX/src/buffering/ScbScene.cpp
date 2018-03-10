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


#include "ScbScene.h"
#include "ScbRigidStatic.h"
#include "ScbBody.h"
#include "ScbShape.h"
#include "ScbConstraint.h"
#include "ScbParticleSystem.h"
#include "ScbArticulation.h"
#include "ScbArticulationJoint.h"
#include "ScbCloth.h"
#include "ScbNpDeps.h"
#include "ScbAggregate.h"

#include "PsFoundation.h"
#include "PxArticulation.h"

namespace physx
{
	class NpMaterial;
}

using namespace physx;


// constants to make boolean template parameters more readable
static const bool tSimRunning = true;
static const bool tAdd = true;
static const bool tDynamic = true;
static const bool tNonSimObject = true;
static const bool tAddToPVD = true;
static const bool tRemoveFromPVD = true;
static const bool tSyncOnRemove = true;
static const bool tWakeOnLostTouchCheck = true;
static const bool tCreatePVDInstances = true;
static const bool tReleasePVDInstances = true;
static const bool tUpdatePVDInstances = true;

void Scb::ObjectTracker::scheduleForInsert(Scb::Base& element)
{
	ControlState::Enum state = element.getControlState();
	PxU32 flags = element.getControlFlags();
	PX_ASSERT(!(flags & ControlFlag::eIS_RELEASED));
	PX_ASSERT(state == ControlState::eNOT_IN_SCENE || state == ControlState::eREMOVE_PENDING);

	if(state == ControlState::eREMOVE_PENDING)
	{
		element.setControlState(ControlState::eIN_SCENE);
		if(!(flags & ControlFlag::eIS_UPDATED))
			remove(element);
	}
	else
	{
		PX_ASSERT(!(flags & ControlFlag::eIS_UPDATED));
		element.setControlState(ControlState::eINSERT_PENDING);
		insert(element);
	}
}

void Scb::ObjectTracker::scheduleForRemove(Scb::Base& element)
{
	ControlState::Enum state = element.getControlState();
	PxU32 flags = element.getControlFlags();

	PX_ASSERT(!(flags & ControlFlag::eIS_RELEASED));

	if(state == ControlState::eINSERT_PENDING)
	{
		// if it's inserted this frame, just remove it - it can't be dirty
		//ML: this assert wont' work because buffered insert raises this flag. We have a unit test which called TEST_F(ObserverTest, OnRelease) to verify it
		//PX_ASSERT(!(flags & ControlFlag::eIS_UPDATED));
		element.setControlState(ControlState::eNOT_IN_SCENE);
		remove(element);
	}
	else if(state == ControlState::eIN_SCENE)
	{
		element.setControlState(ControlState::eREMOVE_PENDING);
		if(!(flags & ControlFlag::eIS_UPDATED))
			insert(element);
	} 
	else
	{
		PX_ALWAYS_ASSERT_MESSAGE("Trying to remove element not in scene.");
	}
}

void Scb::ObjectTracker::scheduleForUpdate(Scb::Base& element)
{
	ControlState::Enum state = element.getControlState();
	PxU32 flags = element.getControlFlags();

	PX_ASSERT(!(flags & ControlFlag::eIS_RELEASED));
	PX_ASSERT(state == ControlState::eIN_SCENE || state == ControlState::eREMOVE_PENDING || state == ControlState::eINSERT_PENDING);

	if(!(flags & ControlFlag::eIS_UPDATED))
	{
		element.setControlFlag(ControlFlag::eIS_UPDATED);
		if(state == ControlState::eIN_SCENE)
			insert(element);
	}
}
	
void Scb::ObjectTracker::clear()							
{	
	Scb::Base *const * elements = mBuffered.getEntries();
	for(PxU32 i=0;i<mBuffered.size();i++)
	{
		ControlState::Enum state = elements[i]->getControlState();
		PxU32 flags = elements[i]->getControlFlags();

		if(state == ControlState::eIN_SCENE || state == ControlState::eINSERT_PENDING)
			elements[i]->resetControl(ControlState::eIN_SCENE);
		else
		{
			elements[i]->resetControl(ControlState::eNOT_IN_SCENE);
			elements[i]->resetScbScene();
		}

		if(flags & ControlFlag::eIS_RELEASED)
			NpDestroy(*elements[i]);
	}
	mBuffered.clear();
}		

void Scb::ObjectTracker::insert(Scb::Base& element)
{
	PX_ASSERT(!mBuffered.contains(&element));
	mBuffered.insert(&element);
}

void Scb::ObjectTracker::remove(Scb::Base& element)
{
	mBuffered.erase(&element);
}

///////////////////////////////////////////////////////////////////////////////

template <bool TSimRunning, bool TAdd, bool TIsDynamic, bool TIsNonSimObject, class T>
PX_FORCE_INLINE static void addOrRemoveRigidObject(Sc::Scene& s, T& rigidObject, bool wakeOnLostTouch);

template <typename T>struct ScSceneFns {};

template<> struct ScSceneFns<Scb::Articulation>
{
	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::Articulation& v)		
	{ 
		Scb::Body* b = NpArticulationGetRootFromScb(v);
		s.addArticulation(v.getScArticulation(), b->getScBody());
	}
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::Articulation& v, bool wakeOnLostTouch)
	{
		PX_UNUSED(wakeOnLostTouch);

		v.clearBufferedSleepStateChange();  // see comment in remove code of Scb::Body

		s.removeArticulation(v.getScArticulation());
	}
};

template<> struct ScSceneFns<Scb::ArticulationJoint>
{
	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::ArticulationJoint& v)	
	{ 
		Scb::Body* scb0, * scb1;
		NpArticulationJointGetBodiesFromScb(v, scb0, scb1);
		s.addArticulationJoint(v.getScArticulationJoint(), scb0->getScBody(), scb1->getScBody());		
	}
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::ArticulationJoint& v, bool wakeOnLostTouch)	{ PX_UNUSED(wakeOnLostTouch); s.removeArticulationJoint(v.getScArticulationJoint());  }
};

template<> struct ScSceneFns<Scb::Constraint>
{
	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::Constraint& v)	
	{ 
		Scb::RigidObject* scb0, * scb1;
		NpConstraintGetRigidObjectsFromScb(v, scb0, scb1);

		PX_ASSERT((!scb0) || (!(scb0->getActorFlags() & PxActorFlag::eDISABLE_SIMULATION)));
		PX_ASSERT((!scb1) || (!(scb1->getActorFlags() & PxActorFlag::eDISABLE_SIMULATION)));
		
		s.addConstraint(v.getScConstraint(), scb0 ? &scb0->getScRigidCore() : NULL, scb1 ? &scb1->getScRigidCore() : NULL);
	}
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::Constraint& v, bool wakeOnLostTouch)
	{
		PX_UNUSED(wakeOnLostTouch);
		s.removeConstraint(v.getScConstraint());
	}
};

template<> struct ScSceneFns<Scb::RigidStatic>
{
	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::RigidStatic& v)	
	{
		// important to use the buffered flags because for a pending insert those describe the end state the
		// user expects.
		
		if (!(v.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION))
			addOrRemoveRigidObject<!tSimRunning, tAdd, !tDynamic, !tNonSimObject>(s, v, false);
		else
			addOrRemoveRigidObject<!tSimRunning, tAdd, !tDynamic, tNonSimObject>(s, v, false);
	}
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::RigidStatic& v, bool wakeOnLostTouch)
	{
		// important to use the original flags because for a pending removal those describe the original state that needs
		// to get cleaned up.
		
		if (!v.isSimDisabledInternally())
			addOrRemoveRigidObject<!tSimRunning, !tAdd, !tDynamic, !tNonSimObject>(s, v, wakeOnLostTouch);
		else
			addOrRemoveRigidObject<!tSimRunning, !tAdd, !tDynamic, tNonSimObject>(s, v, false);
	}
};

template<> struct ScSceneFns<Scb::Body>
{
	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::Body& v)	
	{
		// see comments in rigid static case
		if (!(v.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION))
			addOrRemoveRigidObject<!tSimRunning, tAdd, tDynamic, !tNonSimObject>(s, v, false);
		else
			addOrRemoveRigidObject<!tSimRunning, tAdd, tDynamic, tNonSimObject>(s, v, false);
	}
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::Body& v, bool wakeOnLostTouch)	
	{
		// strictly speaking, the following is only necessary for pending removes but it does not have a
		// functional side effect if applied all the time.
		// When an object gets removed from the scene, pending wakeUp/putToSleep events should be ignored because
		// the internal sleep state for a free standing object is specified as sleeping. All the other parameter changes
		// that go along with a sleep state change should still get processed though (zero vel, zero wake counter on
		// putToSleep for example). Those are not affected because they are tracked through buffered updates
		// of the velocity and wake counter.
		// The clearing happens here because only here we are sure that the object does get removed for real. At earlier
		// stages someone might remove and then re-insert and object and for such cases it is important to keep the
		// sleep state change buffered.
		v.clearBufferedSleepStateChange();

		// see comments in rigid static case
		if (!v.isSimDisabledInternally())
			addOrRemoveRigidObject<!tSimRunning, !tAdd, tDynamic, !tNonSimObject>(s, v, wakeOnLostTouch);
		else
			addOrRemoveRigidObject<!tSimRunning, !tAdd, tDynamic, tNonSimObject>(s, v, false);
	}
};

#if PX_USE_PARTICLE_SYSTEM_API
template<> struct ScSceneFns<Scb::ParticleSystem>
{
	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::ParticleSystem& v)						{ s.addParticleSystem(v.getScParticleSystem());	 }
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::ParticleSystem& v, bool wakeOnLostTouch)	{ PX_UNUSED(wakeOnLostTouch); s.removeParticleSystem(v.getScParticleSystem(), (v.getControlFlags() & Scb::ControlFlag::eIS_RELEASED) != 0);  }
};
#endif

#if PX_USE_CLOTH_API
template<> struct ScSceneFns<Scb::Cloth>
{
//	static PX_FORCE_INLINE void insert(Sc::Scene& s, Scb::Cloth& v)	{ s.addRigidObject(v.getScCloth());		}
	static PX_FORCE_INLINE void remove(Sc::Scene& s, Scb::Cloth& v, bool wakeOnLostTouch)	{ PX_UNUSED(wakeOnLostTouch); s.removeCloth(v.getScCloth());  }
};
#endif

///////////////////////////////////////////////////////////////////////////////

template<typename T, bool b> struct PvdFns 
{ 
	static void createInstance(Scb::Scene&, Pvd::SceneVisualDebugger*, T*) {} 
	static void updateInstance(Scb::Scene&, Pvd::SceneVisualDebugger*, T*) {}
	static void releaseInstance(Scb::Scene&, Pvd::SceneVisualDebugger*, T*) {}
};

#if PX_SUPPORT_VISUAL_DEBUGGER
template<typename T>struct PvdFns<T, true>
{	
	static void createInstance(Scb::Scene& scene, Pvd::SceneVisualDebugger* d, T* v)
	{
		if(d) 
		{ 
			CM_PROFILE_ZONE_WITH_SUBSYSTEM(scene, PVD, createPVDInstance ); 
			d->createPvdInstance(v);
		}
	}

	static void updateInstance(Scb::Scene& scene, Pvd::SceneVisualDebugger* d, T* v) 
	{ 
		if(d && ((v->getControlFlags() & Scb::ControlFlag::eIS_RELEASED) == 0) && (v->getControlState() != Scb::ControlState::eREMOVE_PENDING))
		{ 
			CM_PROFILE_ZONE_WITH_SUBSYSTEM(scene,PVD, updatePVDProperties ); 
			d->updatePvdProperties(v); 
		} 
	}

	static void releaseInstance(Scb::Scene& scene, Pvd::SceneVisualDebugger* d, T* v) 
	{ 
		if(d) 
		{ 
			CM_PROFILE_ZONE_WITH_SUBSYSTEM(scene, PVD, releasePVDInstance ); 
			d->releasePvdInstance(v); 
		}
	} 

};

#endif

#if PX_SUPPORT_VISUAL_DEBUGGER
	#define CREATE_PVD_INSTANCE(connected, obj) { if(connected) { CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,createPVDInstance ); mSceneVisualDebugger.createPvdInstance(obj); } }
	#define RELEASE_PVD_INSTANCE(connected, obj) { if(connected) { CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,releasePVDInstance ); mSceneVisualDebugger.releasePvdInstance(obj); } }
	#define UPDATE_PVD_PROPERTIES(connected, obj) { if(connected) { CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,updatePVDProperties ); mSceneVisualDebugger.updatePvdProperties(obj); } }
	#define SEND_PVD_ARRAYS(connected, obj) { CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,sendPVDArrays ); if(connected) { mSceneVisualDebugger.sendArrays(obj); } }
	#define PVD_ORIGIN_SHIFT(connected, shift){ CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,originShift ); if(connected) { mSceneVisualDebugger.originShift(shift); } }
#else
	#define CREATE_PVD_INSTANCE(connected, obj) {}
	#define RELEASE_PVD_INSTANCE(connected, obj) {}
	#define UPDATE_PVD_PROPERTIES(connected, obj) {}
	#define SEND_PVD_ARRAYS(connected, obj) {}
	#define PVD_ORIGIN_SHIFT(connected, shift){}
#endif

///////////////////////////////////////////////////////////////////////////////

Scb::Scene::Scene(const PxSceneDesc& desc, Cm::EventProfiler eventBuffer) :
	mScene(desc, eventBuffer),
	mSimulationRunning(false),
	mIsBuffering(desc.simulationOrder == PxSimulationOrder::eSOLVE_COLLIDE),
	mStream(16384),
	mShapeMaterialBuffer(PX_DEBUG_EXP("shapeMaterialBuffer")),
	mShapePtrBuffer(PX_DEBUG_EXP("shapePtrBuffer")),
	mActorPtrBuffer(PX_DEBUG_EXP("actorPtrBuffer")),
#if PX_SUPPORT_VISUAL_DEBUGGER
	mSceneVisualDebugger(*this),
#endif
	mWakeCounterResetValue(desc.wakeCounterResetValue),
	mBufferFlags(0)
{
}

void Scb::Scene::release()
{
#if PX_SUPPORT_VISUAL_DEBUGGER
	mSceneVisualDebugger.detach();
#endif
	mScene.release();
	mShapeMaterialBuffer.clear();
	mShapePtrBuffer.clear();
	mActorPtrBuffer.clear();
	mStream.clear();
}

PxScene* Scb::Scene::getPxScene()
{ 
	return NpGetPxScene(*this); 
}

///////////////////////////////////////////////////////////////////////////////

#ifdef PX_PS3
void Scb::Scene::setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value)
{
	if (!mIsBuffering)
	{
		mScene.setSceneParamInt(param,value);
	}
	else
	{
		mBufferedData.setSceneParamInt(param,value);
		markUpdated(BF_SCENE_PARAMS);
	}
}

PxU32 Scb::Scene::getSceneParamInt(PxPS3ConfigParam::Enum param)
{
	return mScene.getSceneParamInt(param);
}
#endif


///////////////////////////////////////////////////////////////////////////////


template<typename T, bool addToPvd>
void Scb::Scene::add(T& v, ObjectTracker &tracker)
{
	v.setScbScene(this);

	if (!mIsBuffering)
	{
		v.resetControl(ControlState::eIN_SCENE);
		ScSceneFns<T>::insert(mScene, v);
		PvdFns<T, addToPvd>::createInstance(*this, getPvd(), &v);
	}
	else
		tracker.scheduleForInsert(v);
}

template<typename T, bool removeFromPvd>
void Scb::Scene::remove(T& v, ObjectTracker &tracker, bool wakeOnLostTouch)
{
	if (!mIsBuffering)
	{
		ScSceneFns<T>::remove(mScene, v, wakeOnLostTouch);
		PvdFns<T, removeFromPvd>::releaseInstance(*this, getPvd(), &v);
		v.resetControl(ControlState::eNOT_IN_SCENE);
		v.setScbScene(NULL);
	}
	else
	{
		tracker.scheduleForRemove(v);
	}
}


///////////////////////////////////////////////////////////////////////////////


template<typename T, bool TIsDynamic, bool addToPvd>
void Scb::Scene::addRigidNoSim(T& v, ObjectTracker &tracker)
{
	PX_ASSERT(v.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION);
	v.setScbScene(this);

	if (!mIsBuffering)
	{
		v.resetControl(ControlState::eIN_SCENE);
		PvdFns<T, addToPvd>::createInstance(*this, getPvd(), &v);
		addOrRemoveRigidObject<!tSimRunning, tAdd, TIsDynamic, tNonSimObject>(getScScene(), v, false);
	}
	else
	{
		tracker.scheduleForInsert(v);
		addOrRemoveRigidObject<tSimRunning, tAdd, TIsDynamic, tNonSimObject>(getScScene(), v, false);
	}
}


template<typename T, bool TIsDynamic, bool removeFromPvd>
void Scb::Scene::removeRigidNoSim(T& v, ObjectTracker &tracker)
{
	PX_ASSERT(v.isSimDisabledInternally());

	if (!mIsBuffering)
	{
		addOrRemoveRigidObject<!tSimRunning, !tAdd, TIsDynamic, tNonSimObject>(getScScene(), v, false);
		PvdFns<T, removeFromPvd>::releaseInstance(*this, getPvd(), &v);
		v.resetControl(ControlState::eNOT_IN_SCENE);
		v.setScbScene(NULL);
	}
	else
	{
		tracker.scheduleForRemove(v);
		addOrRemoveRigidObject<tSimRunning, !tAdd, TIsDynamic, tNonSimObject>(getScScene(), v, false);
	}
}


void Scb::Scene::switchRigidToNoSim(Scb::RigidObject& r, bool isDynamic)
{
	PX_ASSERT(!mIsBuffering);

	// when a simulation objects has a pending remove and then gets switched to a non-simulation object,
	// we must not process the code below. On sync the object will get removed before this call.
	if (r.getControlState() == ControlState::eIN_SCENE)
	{
		size_t ptrOffset = -Scb::Shape::getScOffset();
		Ps::InlineArray<const Sc::ShapeCore*, 64> scShapes;

		if (isDynamic)
			getScScene().removeBody(static_cast<Sc::BodyCore&>(r.getScRigidCore()), scShapes, true);
		else
			getScScene().removeStatic(static_cast<Sc::StaticCore&>(r.getScRigidCore()), scShapes, true);

		// not in simulation anymore -> decrement shape ref-counts
		void* const* shapes = reinterpret_cast<void*const*>(const_cast<Sc::ShapeCore*const*>(scShapes.begin()));
		for(PxU32 i=0; i < scShapes.size(); i++)
		{
			Scb::Shape& scbShape = *Ps::pointerOffset<Scb::Shape*>(shapes[i], (ptrdiff_t)ptrOffset);
			NpShapeDecRefCount(scbShape);
		}
	}
}


void Scb::Scene::switchRigidFromNoSim(Scb::RigidObject& r, bool isDynamic)
{
	PX_ASSERT(!mIsBuffering);

	// when a non-simulation objects has a pending remove and then gets switched to a simulation object,
	// we must not process the code below. On sync the object will get removed before this call.
	if (r.getControlState() == ControlState::eIN_SCENE)
	{
		void* const* shapes;
		size_t shapePtrOffset = NpShapeGetScPtrOffset();
		size_t ptrOffset = shapePtrOffset - Scb::Shape::getScOffset();

		PxU32 nbShapes;
		if (isDynamic)
		{
			nbShapes = NpRigidDynamicGetShapes(static_cast<Scb::Body&>(r), shapes);
			getScScene().addBody(static_cast<Sc::BodyCore&>(r.getScRigidCore()), shapes, nbShapes, shapePtrOffset);
		}
		else
		{
			nbShapes = NpRigidStaticGetShapes(static_cast<Scb::RigidStatic&>(r), shapes);
			getScScene().addStatic(static_cast<Sc::StaticCore&>(r.getScRigidCore()), shapes, nbShapes, shapePtrOffset);
		}

		// add to simulation -> increment shape ref-counts
		for(PxU32 i=0; i < nbShapes; i++)
		{
			Scb::Shape& scbShape = *Ps::pointerOffset<Scb::Shape*>(shapes[i], (ptrdiff_t)ptrOffset);
			NpShapeIncRefCount(scbShape);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////


void Scb::Scene::addRigidStatic(Scb::RigidStatic& rigidStatic, bool noSim)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addActorToSim);
	if (!noSim)
	{
		add<Scb::RigidStatic, tAddToPVD>(rigidStatic, mRigidStaticManager);

		// copy buffer control state from rigid object to shapes and set scene
		if (mIsBuffering)
			addOrRemoveRigidObject<tSimRunning, tAdd, !tDynamic, !tNonSimObject>(getScScene(), rigidStatic, false);
	}
	else
	{
		addRigidNoSim<Scb::RigidStatic, !tDynamic, tAddToPVD>(rigidStatic, mRigidStaticManager);
	}
}


void Scb::Scene::removeRigidStatic(Scb::RigidStatic& rigidStatic, bool wakeOnLostTouch, bool noSim)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeActorFromSim);
	if (!noSim)
	{
		remove<Scb::RigidStatic, true>(rigidStatic, mRigidStaticManager, wakeOnLostTouch);

		// copy buffer control state from rigid object to shapes and set scene
		if (mIsBuffering)
		{
			if (wakeOnLostTouch)
				rigidStatic.scheduleForWakeTouching();
			addOrRemoveRigidObject<tSimRunning, !tAdd, !tDynamic, !tNonSimObject>(getScScene(), rigidStatic, wakeOnLostTouch);
		}
	}
	else
	{
		removeRigidNoSim<Scb::RigidStatic, !tDynamic, true>(rigidStatic, mRigidStaticManager);
	}

	rigidStatic.clearBufferedState();
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Scene::addRigidBody(Scb::Body& body, bool noSim)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,addActorToSim);
	if (!noSim)
	{
		add<Scb::Body, tAddToPVD>(body, mBodyManager);
		body.initBufferedState();

		// copy buffer control state from rigid object to shapes and set scene
		if (mIsBuffering)
			addOrRemoveRigidObject<tSimRunning, tAdd, tDynamic, !tNonSimObject>(getScScene(), body, false);
	}
	else
	{
		addRigidNoSim<Scb::Body, tDynamic, tAddToPVD>(body, mBodyManager);
		body.initBufferedState();
	}
}


void Scb::Scene::removeRigidBody(Scb::Body& body, bool wakeOnLostTouch, bool noSim)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,API,removeActorFromSim);
	if (!noSim)
	{
		body.clearSimStateDataForPendingInsert();

		remove<Scb::Body, tRemoveFromPVD>(body, mBodyManager, wakeOnLostTouch);
		body.clearBufferedState();

		// copy buffer control state from rigid object to shapes and set scene
		if (mIsBuffering)
		{
			if (wakeOnLostTouch)
				body.scheduleForWakeTouching();
			addOrRemoveRigidObject<tSimRunning, !tAdd, tDynamic, !tNonSimObject>(getScScene(), body, wakeOnLostTouch);
		}
	}
	else
	{
		removeRigidNoSim<Scb::Body, tDynamic, tRemoveFromPVD>(body, mBodyManager);

		// note: "noSim" refers to the internal state here. The following asserts only apply if the bufferd state has not switched to "sim".
		PX_ASSERT(!(body.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION) || body.isSleeping());
		PX_ASSERT(!(body.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION) || !body.isBuffered(BodyBuffer::BF_KinematicTarget | BodyBuffer::BF_Acceleration | BodyBuffer::BF_DeltaVelocity));
		// What about velocity, wakeCounter, ...?
		// Those are not allowed on a no-sim object, however, they might still be necessary due to complex buffering scenarios:
		// Imagine the following operation flow (all buffered):
		// - dynamic sim object awake with velocities
		// - switch to no-sim -> needs to clear velocities, wake counter, put to sleep, ...
		// - switch back to sim -> the velocities, wake counter, ... still need to get cleared and it needs to be asleep (that would be the non-buffered behavior of the operations)

		body.clearBufferedState();	// this also covers the buffered case where a noSim object gets switched to a sim object, followed by a wakeUp() call and then a remove. 
									// If we checked whether the buffered object is still a noSim object then only body.RigidObject::clearBufferedState() would be necessary.
	}
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Scene::addConstraint(Scb::Constraint& constraint)
{
	add<Scb::Constraint, tAddToPVD>(constraint, mConstraintManager);
}

void Scb::Scene::removeConstraint(Scb::Constraint& constraint)
{
	if (!mIsBuffering)
	{
		mScene.removeConstraint(constraint.getScConstraint());
		
		// Release pvd constraint immediately since delayed removal with already released ext::joints does not work, can't call callback.
		RELEASE_PVD_INSTANCE((constraint.getControlState() != ControlState::eINSERT_PENDING) && mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &constraint)
	
		constraint.resetControl(ControlState::eNOT_IN_SCENE);
		constraint.setScbScene(NULL);
	}
	else
	{
		mConstraintManager.scheduleForRemove(constraint);
	}
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Scene::addArticulation(Scb::Articulation& articulation)
{
	add<Scb::Articulation, tAddToPVD>(articulation, mArticulationManager);
	articulation.initBufferedState();
}

void Scb::Scene::removeArticulation(Scb::Articulation& articulation)
{
	remove<Scb::Articulation, tAddToPVD>(articulation, mArticulationManager);
	articulation.clearBufferedState();
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Scene::addArticulationJoint(Scb::ArticulationJoint& joint)
{
	add<Scb::ArticulationJoint, !tAddToPVD>(joint, mArticulationJointManager);
}

void Scb::Scene::removeArticulationJoint(Scb::ArticulationJoint& joint)
{
	remove<Scb::ArticulationJoint, !tAddToPVD>(joint, mArticulationJointManager);
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Scene::addAggregate(Scb::Aggregate& agg)
{
	agg.setScbScene(this);

	if (!mIsBuffering)
	{
		PxU32 aggregateID = mScene.createAggregate(agg.mPxAggregate, agg.getSelfCollide());
		agg.setAggregateID(aggregateID);
		agg.resetControl(ControlState::eIN_SCENE);	
#if PX_SUPPORT_VISUAL_DEBUGGER
		//Sending pvd events after all aggregates's acotrs are inserted into scene
		addAggregateToPvd(agg);
#endif
	}
	else
		mAggregateManager.scheduleForInsert(agg);
}


void Scb::Scene::removeAggregate(Scb::Aggregate& agg)
{
	if (!mIsBuffering)
	{
		mScene.deleteAggregate(agg.getAggregateID());
		agg.resetControl(ControlState::eNOT_IN_SCENE);
		agg.setScbScene(NULL);
#if PX_SUPPORT_VISUAL_DEBUGGER
		removeAggregateFromPvd(agg);
#endif
	}
	else
	{
		mAggregateManager.scheduleForRemove(agg);
	}
}


bool Scb::Scene::addMaterial(Sc::MaterialCore& material)
{
	Ps::Mutex::ScopedLock lock(mSceneMaterialBufferLock);
	MaterialEvent mEvent;
	mEvent.mType = MATERIAL_ADD;
	mEvent.mHandle = material.getMaterialIndex();

	mSceneMaterialBuffer.pushBack(mEvent);
	
	CREATE_PVD_INSTANCE(mSceneVisualDebugger.isConnected(true), &material)

	return true;
}

void Scb::Scene::updateMaterial(Sc::MaterialCore& material)//(PxU32 index, Sc::MaterialCore& material)
{
	Ps::Mutex::ScopedLock lock(mSceneMaterialBufferLock);
	MaterialEvent mEvent;
	mEvent.mType = MATERIAL_UPDATE;
	mEvent.mHandle = material.getMaterialIndex();
	mSceneMaterialBuffer.pushBack(mEvent);
	UPDATE_PVD_PROPERTIES(mSceneVisualDebugger.isConnected(true), &material)
}

void Scb::Scene::removeMaterial(Sc::MaterialCore& material)//(PxU32 index, PxMaterial* nxMat)
{
	if(material.getMaterialIndex() == MATERIAL_INVALID_HANDLE)
		return;

	Ps::Mutex::ScopedLock lock(mSceneMaterialBufferLock);
	MaterialEvent mEvent;
	mEvent.mType = MATERIAL_REMOVE;
	mEvent.mHandle = material.getMaterialIndex();
	mSceneMaterialBuffer.pushBack(mEvent);

	RELEASE_PVD_INSTANCE(mSceneVisualDebugger.isConnected(true), &material);
}

///////////////////////////////////////////////////////////////////////////////

#if PX_USE_PARTICLE_SYSTEM_API
void Scb::Scene::addParticleSystem(Scb::ParticleSystem& ps)
{
	add<Scb::ParticleSystem, tAddToPVD>(ps, mParticleSystemManager);
}

void Scb::Scene::removeParticleSystem(Scb::ParticleSystem& ps, bool isRelease)
{
	if (!mIsBuffering)
	{
		RELEASE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &ps)
		ps.removeFromScene();
		mScene.removeParticleSystem(ps.getScParticleSystem(), isRelease);
		ps.resetControl(ControlState::eNOT_IN_SCENE);
		ps.setScbScene(NULL);
	}
	else
	{
		mParticleSystemManager.scheduleForRemove(ps);
	}
}
#endif


///////////////////////////////////////////////////////////////////////////////

#if PX_USE_CLOTH_API
void Scb::Scene::addCloth(Scb::Cloth& cl)
{
	cl.setScbScene(this);

	if (!mIsBuffering)
	{
		if (mScene.addCloth(cl.getScCloth()))
		{
			cl.resetControl(ControlState::eIN_SCENE);
			CREATE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &cl)
		}
		else
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Adding cloth to the scene failed!");
	}
	else
		mClothManager.scheduleForInsert(cl);
}

void Scb::Scene::removeCloth(Scb::Cloth& cl)
{
	if (!mIsBuffering)
	{
		RELEASE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &cl)
		mScene.removeCloth(cl.getScCloth());
		cl.resetControl(ControlState::eNOT_IN_SCENE);
		cl.setScbScene(NULL);
	}
	else
	{
		mClothManager.scheduleForRemove(cl);
	}
}
#endif // PX_USE_CLOTH_API

///////////////////////////////////////////////////////////////////////////////

void Scb::Scene::updateLowLevelMaterial(NpMaterial** masterMaterial)
{
	Ps::Mutex::ScopedLock lock(mSceneMaterialBufferLock);

//#if PX_SUPPORT_VISUAL_DEBUGGER
//	bool pvdConnected = mSceneVisualDebugger.isConnected();
//#endif
	//sync all the material events
	PxsMaterialManager* manager = mScene.getMaterialManager();
	for(PxU32 i=0; i< mSceneMaterialBuffer.size(); ++i)
	{
		MaterialEvent& mEvent = mSceneMaterialBuffer[i];
		NpMaterial* masMat = masterMaterial[mEvent.mHandle];
		switch(mEvent.mType)
		{
		case MATERIAL_ADD:
			if(masMat)
			{
				Sc::MaterialCore* materialCore = &masterMaterial[mEvent.mHandle]->getScMaterial();
				manager->setMaterial(materialCore);
			}
			break;
		case MATERIAL_UPDATE:
			if(masMat)
			{
				Sc::MaterialCore* materialCore = &masterMaterial[mEvent.mHandle]->getScMaterial();
				manager->updateMaterial(materialCore);
			}
			break;
		case MATERIAL_REMOVE:
			if (mEvent.mHandle < manager->getMaxSize())	// materials might get added and then removed again immediately. However, the add does not get processed (see case MATERIAL_ADD above),
														// so the remove might end up reading out of bounds memory unless checked.
				manager->removeMaterial(manager->getMaterial(mEvent.mHandle));
			break;
		default:
			PX_ASSERT(0);
		};
	}

	mSceneMaterialBuffer.resize(0);
}

///////////////////////////////////////////////////////////////////////////////
void Scb::Scene::preSimulateUpdateAppThread(PxReal timeStep)
{
	PX_UNUSED(timeStep);
#if PX_USE_PARTICLE_SYSTEM_API
	// Submit applied forces to particle systems
	PxU32 nbParticleSystems = mScene.getNbParticleSystems();
	Sc::ParticleSystemCore** particleSystems = mScene.getParticleSystems();
	for(PxU32 i=0; i < nbParticleSystems; i++)
		Scb::ParticleSystem::fromSc(*particleSystems[i]).submitForceUpdates(timeStep);
#endif
}


//--------------------------------------------------------------
//
// Data synchronization
//
//--------------------------------------------------------------

bool Scb::Scene::isUpdatingPvd() 
{
	// apparently this isn't a const method on Pvd::SceneVisualDebugger
#if PX_SUPPORT_VISUAL_DEBUGGER
	return mSceneVisualDebugger.isConnectedAndSendingDebugInformation();
#else
	return false;
#endif
}

Pvd::SceneVisualDebugger* Scb::Scene::getPvd() 
{
#if PX_SUPPORT_VISUAL_DEBUGGER
	return mSceneVisualDebugger.isConnectedAndSendingDebugInformation() ? &mSceneVisualDebugger : NULL;
#else
	return NULL;
#endif
}


void Scb::Scene::syncState()
{

#if PX_SUPPORT_VISUAL_DEBUGGER
	bool pvdConnected = isUpdatingPvd();
#endif

	//process client creation -- must be done before BF_CLIENT_BEHAVIOR_FLAGS processing in the below block:
	while (mBufferedData.numClientsCreated)
	{
		mScene.createClient();
		mBufferedData.numClientsCreated--;
	}

	if (mBufferFlags)
	{
		if (isBuffered(BF_GRAVITY))
			mScene.setGravity(mBufferedData.gravity);

		if(isBuffered(BF_BOUNCETHRESHOLDVELOCITY))
			mScene.setBounceThresholdVelocity(mBufferedData.bounceThresholdVelocity);

		if (isBuffered(BF_FLAGS))
			mScene.setPublicFlags(mBufferedData.flags);

		if (isBuffered(BF_DOMINANCE_PAIRS))
			mBufferedData.syncDominancePairs(mScene);

		if (isBuffered(BF_SOLVER_BATCH_SIZE))
			mScene.setSolverBatchSize(mBufferedData.solverBatchSize);

		if (isBuffered(BF_CLIENT_BEHAVIOR_FLAGS))
		{
			for (PxU32 i = 0; i < mBufferedData.clientBehaviorFlags.size(); i++)
			{
				if (mBufferedData.clientBehaviorFlags[i] != PxClientBehaviorFlag_eNOT_BUFFERED)	//not PxClientBehaviorFlag_eNOT_BUFFERED means it was written.
				{
					mScene.setClientBehaviorFlags(PxClientID(i), mBufferedData.clientBehaviorFlags[i]);
					mBufferedData.clientBehaviorFlags[i] = PxClientBehaviorFlag_eNOT_BUFFERED;
				}

			}
		}

		if (isBuffered(BF_VISUALIZATION))
		{
			for(PxU32 i=0; i < PxVisualizationParameter::eNUM_VALUES; i++)
			{
				if (mBufferedData.visualizationParamChanged[i])
				{
					mScene.setVisualizationParameter(static_cast<PxVisualizationParameter::Enum>(i), mBufferedData.visualizationParam[i]);
				}
			}
		}

#ifdef PX_PS3
		if (isBuffered(BF_SCENE_PARAMS))
		{
			mBufferedData.syncSceneParamInts(mScene);
		}
#endif

#if PX_SUPPORT_VISUAL_DEBUGGER
		if(pvdConnected) 
			mSceneVisualDebugger.updatePvdProperties();
#endif
	}


	mBufferFlags = 0;
	mBufferedData.clearDominanceBuffer();
	mBufferedData.clearVisualizationParams();
#ifdef PX_PS3
	mBufferedData.clearSceneParamBuffer();
#endif

}

template<typename T, bool pvdInserts, bool pvdUpdates>
void Scb::Scene::processUserUpdates(ObjectTracker &tracker, bool /*pvdConnected*/)
{
	Base*const * buffered = tracker.getBuffered();
	for(PxU32 i=0; i < tracker.getBufferedCount(); i++)
	{
		T& v = *static_cast<T*>(buffered[i]);
		if (v.getControlState() == ControlState::eINSERT_PENDING)
		{
			ScSceneFns<T>::insert(mScene, v);
			PvdFns<T, pvdInserts>::createInstance(*this, getPvd(), &v);
		}
		else if(v.getControlFlags() & ControlFlag::eIS_UPDATED)
		{
			v.syncState();
			PvdFns<T, pvdUpdates>::updateInstance(*this, getPvd(), &v);
		}
	}
}

template<typename T, typename S, bool pvdUpdates>
void Scb::Scene::processSimUpdates(S*const * scObjects, PxU32 nbObjects, bool /*pvdConnected*/)
{
	for(PxU32 i=0;i<nbObjects;i++)
	{
		T& v = T::fromSc(*scObjects[i]);
		
		if(!(v.getControlFlags() & ControlFlag::eIS_UPDATED)) // else the data will be synced further below
		{
			v.syncState();
			PvdFns<T, pvdUpdates>::updateInstance(*this, getPvd(), &v);
		}
	}
}


void Scb::Scene::addShapeToPvd(Scb::Shape& shape, PxActor& owner)
{
	PX_UNUSED(shape);
	PX_UNUSED(owner);

#if PX_SUPPORT_VISUAL_DEBUGGER
	 if(mSceneVisualDebugger.isConnectedAndSendingDebugInformation()) 
	 { 
		 CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,createPVDInstance ); 
		 mSceneVisualDebugger.createPvdInstance(&shape, owner); 
	 } 
#endif
}

void Scb::Scene::removeShapeFromPvd(Scb::Shape& shape, PxActor &owner)
{
	PX_UNUSED(shape);
	PX_UNUSED(owner);

#if PX_SUPPORT_VISUAL_DEBUGGER	 
	if(mSceneVisualDebugger.isConnectedAndSendingDebugInformation()) 
	{ 
		CM_PROFILE_ZONE_WITH_SUBSYSTEM( *this,PVD,releasePVDInstance ); 
		mSceneVisualDebugger.releasePvdInstance(&shape, owner); 
	} 
#endif
}


void Scb::Scene::addBodyAndShapesToPvd(Scb::Body& b)
{
	PX_UNUSED(b);

#if PX_SUPPORT_VISUAL_DEBUGGER
	CREATE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &b)

	void* const* shapes;
	size_t shapePtrOffset = NpShapeGetScPtrOffset();
	PxActor& pxActor = *b.getScBody().getPxActor();

	PxU32 nbShapes = NpRigidDynamicGetShapes(b, shapes);
	for(PxU32 i=0;i<nbShapes;i++)
		addShapeToPvd(*reinterpret_cast<Scb::Shape*>(reinterpret_cast<char*>(shapes[i])+shapePtrOffset - Scb::Shape::getScOffset()), pxActor);
#endif
}

void Scb::Scene::addStaticAndShapesToPvd(Scb::RigidStatic& s)
{
	PX_UNUSED(s);

#if PX_SUPPORT_VISUAL_DEBUGGER
	CREATE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &s)
	void* const* shapes;
	size_t shapePtrOffset = NpShapeGetScPtrOffset();
	PxActor& pxActor = *s.getScStatic().getPxActor();

	PxU32 nbShapes = NpRigidStaticGetShapes(s, shapes);
	for(PxU32 i=0;i<nbShapes;i++)
		addShapeToPvd(*reinterpret_cast<Scb::Shape*>(reinterpret_cast<char*>(shapes[i])+shapePtrOffset - Scb::Shape::getScOffset()), pxActor);
#endif
}

void Scb::Scene::addAggregateToPvd(Scb::Aggregate& agg)
{
	PX_UNUSED(agg);

#if PX_SUPPORT_VISUAL_DEBUGGER
	CREATE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &agg)
#endif
}

void Scb::Scene::removeAggregateFromPvd(Scb::Aggregate& agg)
{
	PX_UNUSED(agg);

#if PX_SUPPORT_VISUAL_DEBUGGER	 
	RELEASE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), &agg)
#endif
}
#define ENABLE_PVD_ORIGINSHIFT_EVENT
void Scb::Scene::shiftOrigin(const PxVec3& shift)
{ 
	PX_ASSERT(!isPhysicsBuffering()); 
	mScene.shiftOrigin(shift); 
	
#ifdef ENABLE_PVD_ORIGINSHIFT_EVENT
	PVD_ORIGIN_SHIFT(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), shift);
#endif
}


void Scb::Scene::syncEntireScene(PxU32* error)
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,syncState);
	if (error)
		*error = mScene.getErrorState();

	mStream.lock();

	bool pvdConnected = isUpdatingPvd();

	syncState();
	//
	// Process aggregates (needs to be done before adding actors because the actor's aggregateID needs to get set)
	//

	for(PxU32 i=0; i < mAggregateManager.getBufferedCount(); i++)
	{
		Aggregate* a = static_cast<Aggregate*>(mAggregateManager.getBuffered()[i]);
		if (a->getControlState() == ControlState::eINSERT_PENDING)
		{
			PxU32 aggregateID = mScene.createAggregate(a->mPxAggregate, a->getSelfCollide());
			a->setAggregateID(aggregateID);
			addAggregateToPvd(*a);
			a->syncState(*this);  // Necessary to set the aggregate ID for all actors of the aggregate
		}
		else if(a->getControlFlags() & ControlFlag::eIS_UPDATED)
		{
			a->syncState(*this);
		}
	}
	mAggregateManager.clear();
	mActorPtrBuffer.clear();


	// rigid statics
	processUserUpdates<Scb::RigidStatic, tCreatePVDInstances, tUpdatePVDInstances>(mRigidStaticManager, pvdConnected);
	mRigidStaticManager.clear();


	// rigid dynamics and articulation links
	//
	// 1) Sync simulation changed data
	{
		
		Sc::BodyIterator bodyIterator;
		mScene.initActiveBodiesIterator(bodyIterator);
		for(Sc::BodyCore* body = bodyIterator.getNext(); body; body = bodyIterator.getNext())
		{
			Scb::Body& bufferedBody = Scb::Body::fromSc(*body);
			if (!(bufferedBody.getControlFlags() & ControlFlag::eIS_UPDATED))  // Else the data will be synced further below
				bufferedBody.syncState();
		}
	}

	// 2) Sync data of rigid dynamics which were put to sleep by the simulation

	const Ps::Array<Sc::BodyCore*>& sleepingBodies = mScene.getSleepBodiesArray();
	const PxU32 nbSleepingBodies = sleepingBodies.size();
	processSimUpdates<Scb::Body, Sc::BodyCore, tUpdatePVDInstances>(sleepingBodies.begin(), nbSleepingBodies, pvdConnected);

	// user updates
	processUserUpdates<Scb::Body, tCreatePVDInstances, tUpdatePVDInstances>(mBodyManager, pvdConnected);
	mBodyManager.clear();
	mShapePtrBuffer.clear();


	// rigid body shapes
	//
	// IMPORTANT: This has to run after the material update
	//
	// Sync user changed data. Inserts and removes are handled in actor sync
	for(PxU32 i=0; i < mShapeManager.getBufferedCount(); i++)
	{
		Scb::Shape* s = static_cast<Scb::Shape*>(mShapeManager.getBuffered()[i]);

		if(s->getControlFlags() & ControlFlag::eIS_UPDATED)
		{
			s->syncState();
			UPDATE_PVD_PROPERTIES(pvdConnected, s)
		}
	}

	mShapeManager.clear();
	mShapeMaterialBuffer.clear();

	// constraints (get break force and broken status from sim)

	processSimUpdates<Scb::Constraint, Sc::ConstraintCore, tUpdatePVDInstances>(mScene.getConstraints(), mScene.getNbConstraints(), pvdConnected);
	processUserUpdates<Scb::Constraint, tCreatePVDInstances, tUpdatePVDInstances>(mConstraintManager, pvdConnected);
	mConstraintManager.clear();


	// articulations (get sleep state from sim)

	processSimUpdates<Scb::Articulation, Sc::ArticulationCore, tUpdatePVDInstances>(mScene.getArticulations(), mScene.getNbArticulations(), pvdConnected);
	processUserUpdates<Scb::Articulation, tCreatePVDInstances, tUpdatePVDInstances>(mArticulationManager, pvdConnected);
	mArticulationManager.clear();


	// Process articulation joints

	processUserUpdates<Scb::ArticulationJoint, !tCreatePVDInstances, tUpdatePVDInstances>(mArticulationJointManager, pvdConnected);
	mArticulationJointManager.clear();

#if PX_USE_PARTICLE_SYSTEM_API
	//
	// Process particle systems
	//
	// 1) Sync simulation changed data

	PxU32 nbParticleSystems = mScene.getNbParticleSystems();
	Sc::ParticleSystemCore** particleSystems = mScene.getParticleSystems();
	for(PxU32 i=0; i < nbParticleSystems; i++)
	{
		Scb::ParticleSystem& scbParticleSystem = Scb::ParticleSystem::fromSc(*particleSystems[i]);
		scbParticleSystem.syncState();
		UPDATE_PVD_PROPERTIES(pvdConnected && scbParticleSystem.hasUpdates(), &scbParticleSystem)
	}

	// 2) Sync user changed data
	for(PxU32 i=0; i < mParticleSystemManager.getBufferedCount(); i++)
	{
		ParticleSystem* p = static_cast<ParticleSystem*>(mParticleSystemManager.getBuffered()[i]);
		
		//special handling to release bulk buffer data
		if (p->getControlState() == ControlState::eREMOVE_PENDING)
			p->removeFromScene();

		else if (p->getControlState() == ControlState::eINSERT_PENDING)
		{
			mScene.addParticleSystem(p->getScParticleSystem());
			CREATE_PVD_INSTANCE(pvdConnected, p)
		}
	}
	mParticleSystemManager.clear();
#endif

#if PX_USE_CLOTH_API
	//
	// Process cloth
	//
	// Pending insert & sync user changed data

	for(PxU32 i=0; i < mClothManager.getBufferedCount(); i++)
	{
		Cloth* cl = static_cast<Cloth*>(mClothManager.getBuffered()[i]);
		if (cl->getControlState() == ControlState::eINSERT_PENDING)
		{
			if (mScene.addCloth(cl->getScCloth()))
			{
				CREATE_PVD_INSTANCE(pvdConnected, cl)
			}
			else
				Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Adding cloth to the scene failed!");
		}
		// no need yet
		//else if(cl->getControlFlags() & ControlFlag::eIS_UPDATED)
		//{
		//	cl->syncState();
		//	UPDATE_PVD_PROPERTIES(pvdConnected, cl)
		//}
	}
	mClothManager.clear();
#endif // PX_USE_CLOTH_API

	//Run all pending pvd commands here.
#if PX_SUPPORT_VISUAL_DEBUGGER
	getSceneVisualDebugger().flushPendingCommands();
#endif

	mStream.clearNotThreadSafe();
	mStream.unlock();
}



template<typename T, bool syncOnRemove, bool wakeOnLostTouchCheck, bool releasePvdInstance> 
void Scb::Scene::processRemoves(ObjectTracker& tracker)
{	
	typedef ScSceneFns<T> Fns;
	bool pvdConnected = isUpdatingPvd();

	PX_UNUSED(pvdConnected);

	for(PxU32 i=0; i < tracker.getBufferedCount(); i++)
	{
		T* v = static_cast<T*>(tracker.getBuffered()[i]);
		if(v->getControlState() == ControlState::eREMOVE_PENDING)
		{
			bool wakeOnLostTouch = false;
			if (wakeOnLostTouchCheck)
			{
				PX_ASSERT(	(v->getScbType() == ScbType::BODY) ||
							(v->getScbType() == ScbType::BODY_FROM_ARTICULATION_LINK) ||
							(v->getScbType() == ScbType::RIGID_STATIC) );
				wakeOnLostTouch = (v->Base::isBuffered(RigidObjectBuffer::BF_WakeTouching) != 0);  // important to use Scb::Base::isBuffered() because Scb::Body, for example, has a shadowed implementation of this method
			}

			Fns::remove(mScene, *v, wakeOnLostTouch);

			// if no object param has been updated, the state sync still needs to be processed to write simulation results
			// back to the permanently buffered params.
			if (syncOnRemove && !(v->getControlFlags() & ControlFlag::eIS_UPDATED))
				v->syncState();

			PvdFns<T, releasePvdInstance>::releaseInstance(*this, getPvd(), v);
		}
	}
}

template<typename T> 
void Scb::Scene::processShapeRemoves(ObjectTracker& tracker)
{	
	for(PxU32 i=0; i < tracker.getBufferedCount(); i++)
	{
		T* v = static_cast<T*>(tracker.getBuffered()[i]);
		v->processShapeRemoves();
	}
}

void Scb::Scene::processPendingRemove()
{
	processShapeRemoves<Scb::RigidStatic>(mRigidStaticManager);
	processShapeRemoves<Scb::Body>(mBodyManager);

	processRemoves<Scb::Constraint,			tSyncOnRemove,	!tWakeOnLostTouchCheck,	!tReleasePVDInstances>	(mConstraintManager);
	
	Scb::Base *const * buffered = mConstraintManager.getBuffered();
	for(PxU32 i=0; i < mConstraintManager.getBufferedCount(); i++)
	{
		Scb::Constraint* constraint = static_cast<Scb::Constraint*>(buffered[i]);
		Scb::Scene *scene =  constraint->getScbSceneForAPI();
		
		if(constraint->getControlFlags() & ControlFlag::eIS_UPDATED)
			constraint->prepareForActorRemoval();	// see comments in Scb::Constraint
		else if(constraint->getControlState() != ControlState::eINSERT_PENDING && scene)
			RELEASE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), constraint)

		if(scene== NULL)
			RELEASE_PVD_INSTANCE(mSceneVisualDebugger.isConnectedAndSendingDebugInformation(), constraint)
	}

	processRemoves<Scb::ArticulationJoint,  !tSyncOnRemove,	!tWakeOnLostTouchCheck,	!tReleasePVDInstances>	(mArticulationJointManager);
	processRemoves<Scb::RigidStatic,		!tSyncOnRemove,	tWakeOnLostTouchCheck,	tReleasePVDInstances>	(mRigidStaticManager);
	processRemoves<Scb::Body,				tSyncOnRemove,	tWakeOnLostTouchCheck,	tReleasePVDInstances>	(mBodyManager);
	processRemoves<Scb::Articulation,		tSyncOnRemove,	!tWakeOnLostTouchCheck,	tReleasePVDInstances>	(mArticulationManager);

#if PX_USE_PARTICLE_SYSTEM_API
	processRemoves<Scb::ParticleSystem,		!tSyncOnRemove,	!tWakeOnLostTouchCheck,	tReleasePVDInstances>	(mParticleSystemManager);
#endif

#if PX_USE_CLOTH_API
	processRemoves<Scb::Cloth,				!tSyncOnRemove,	!tWakeOnLostTouchCheck,	tReleasePVDInstances>	(mClothManager);
#endif

	// Do after actors have been removed (coumpound can only be removed after all its elements are gone)
	for(PxU32 i=0; i < mAggregateManager.getBufferedCount(); i++)
	{
		Aggregate* a = static_cast<Aggregate*>(mAggregateManager.getBuffered()[i]);

		if(a->getControlState() == ControlState::eREMOVE_PENDING)
		{
			a->syncState(*this);  // Clears the aggregate ID for all actors of the aggregate
			mScene.deleteAggregate(a->getAggregateID());

#if PX_SUPPORT_VISUAL_DEBUGGER
			removeAggregateFromPvd(*a);
#endif
		}
	}


}

void Scb::Scene::bufferCreationError(const char* object)
{
	Ps::getFoundation().error(PxErrorCode::eOUT_OF_MEMORY, __FILE__, __LINE__, "Fetching %s data buffer failed!", object);
}

void Scb::Scene::scheduleForUpdate(Scb::Base& object)
{
	switch(object.getScbType())
	{
		case ScbType::SHAPE_EXCLUSIVE:
		case ScbType::SHAPE_SHARED:					{ mShapeManager.scheduleForUpdate(object);				}break;
		case ScbType::BODY:							{ mBodyManager.scheduleForUpdate(object);				}break;
		case ScbType::BODY_FROM_ARTICULATION_LINK:	{ mBodyManager.scheduleForUpdate(object);				}break;
		case ScbType::RIGID_STATIC:					{ mRigidStaticManager.scheduleForUpdate(object);		}break;
		case ScbType::CONSTRAINT:					{ mConstraintManager.scheduleForUpdate(object);			}break;
#if PX_USE_PARTICLE_SYSTEM_API
		case ScbType::PARTICLE_SYSTEM:				{ mParticleSystemManager.scheduleForUpdate(object);		}break;
#endif
		case ScbType::ARTICULATION:					{ mArticulationManager.scheduleForUpdate(object);		}break;
		case ScbType::ARTICULATION_JOINT:			{ mArticulationJointManager.scheduleForUpdate(object);	}break;
		case ScbType::AGGREGATE:					{ mAggregateManager.scheduleForUpdate(object);			}break;
#if PX_USE_CLOTH_API
		case ScbType::CLOTH:						{ mClothManager.scheduleForUpdate(object);				}break;
#endif
		case ScbType::UNDEFINED:
		case ScbType::TYPE_COUNT:
		default:
			PX_ALWAYS_ASSERT_MESSAGE( "scheduleForUpdate: missing type!");
			break;
	}
}

PxU8* Scb::Scene::getStream(ScbType::Enum type)
{
	PxU8* memory;
	switch(type)
	{
		case ScbType::SHAPE_EXCLUSIVE:
		case ScbType::SHAPE_SHARED:					{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::ShapeBuffer));				new (memory) Scb::ShapeBuffer;				}break;
		case ScbType::BODY:							{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::BodyBuffer));				new (memory) Scb::BodyBuffer;				}break;
		case ScbType::BODY_FROM_ARTICULATION_LINK:	{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::BodyBuffer));				new (memory) Scb::BodyBuffer;				}break;
		case ScbType::RIGID_STATIC:					{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::RigidStaticBuffer));			new (memory) Scb::RigidStaticBuffer;		}break;
		case ScbType::CONSTRAINT:					{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::ConstraintBuffer));			new (memory) Scb::ConstraintBuffer;			}break;
#if PX_USE_PARTICLE_SYSTEM_API
		case ScbType::PARTICLE_SYSTEM:				{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::ParticleSystemBuffer));		new (memory) Scb::ParticleSystemBuffer;		}break;
#endif
		case ScbType::ARTICULATION:					{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::ArticulationBuffer));		new (memory) Scb::ArticulationBuffer;		}break;
		case ScbType::ARTICULATION_JOINT:			{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::ArticulationJointBuffer));	new (memory) Scb::ArticulationJointBuffer;	}break;
		case ScbType::AGGREGATE:					{ memory = (PxU8*)mStream.allocateNotThreadSafe(sizeof(Scb::AggregateBuffer));			new (memory) Scb::AggregateBuffer;			}break;
		
#if PX_USE_CLOTH_API
		case ScbType::CLOTH:
#endif
		case ScbType::UNDEFINED:
		case ScbType::TYPE_COUNT:
		default:
			PX_ALWAYS_ASSERT_MESSAGE("getStream: missing type!");
			return NULL;
	}
	return memory;
}

///////////////////////////////////////////////////////////////////////////////

PxBroadPhaseType::Enum Scb::Scene::getBroadPhaseType() const
{
	return mScene.getBroadPhaseType();
}

bool Scb::Scene::getBroadPhaseCaps(PxBroadPhaseCaps& caps) const
{
	return mScene.getBroadPhaseCaps(caps);
}

PxU32 Scb::Scene::getNbBroadPhaseRegions() const
{
	return mScene.getNbBroadPhaseRegions();
}

PxU32 Scb::Scene::getBroadPhaseRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return mScene.getBroadPhaseRegions(userBuffer, bufferSize, startIndex);
}

PxU32 Scb::Scene::addBroadPhaseRegion(const PxBroadPhaseRegion& region, bool populateRegion)
{
	if(!isPhysicsBuffering())
		return mScene.addBroadPhaseRegion(region, populateRegion);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::addBroadPhaseRegion() not allowed while simulation is running. Call will be ignored.");
	return 0xffffffff;
}

bool Scb::Scene::removeBroadPhaseRegion(PxU32 handle)
{
	if(!isPhysicsBuffering())
		return mScene.removeBroadPhaseRegion(handle);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::removeBroadPhaseRegion() not allowed while simulation is running. Call will be ignored.");
	return false;
}

//////////////////////////////////////////////////////////////////////////

//
// To avoid duplication of a lot of similar code, the following templated method was introduced. Its main purpose is to
// take care of all the operations related to adding/removing a rigid object to/from the scene. Depending on the type
// of rigid object and the simulation state, there are slight changes to the code flow necessary. Among the operations are:
//
// - Add/remove rigid object to/from scene
// - Add/remove shapes from PVD
// - Adjust buffer control state of shapes
// - Adjust ref-count of shapes
//
template <bool TSimRunning, bool TAdd, bool TIsDynamic, bool TIsNonSimObject, class T>
PX_FORCE_INLINE static void addOrRemoveRigidObjectInternal(Sc::Scene& s, T& rigidObject,Ps::InlineArray<const Sc::ShapeCore*, 64> scShapes, bool wakeOnLostTouch)
{
	PxActor* pxActor = NULL;
	void* const* shapes;
	PxU32 nbShapes;	
	size_t shapePtrOffset = NpShapeGetScPtrOffset();

	Scb::Body& dynamicObject = reinterpret_cast<Scb::Body&>(rigidObject);
	Scb::RigidStatic& staticObject = reinterpret_cast<Scb::RigidStatic&>(rigidObject);

	if (!TSimRunning)
	{
		if (TIsDynamic)
			pxActor = dynamicObject.getScBody().getPxActor();
		else
			pxActor = staticObject.getScStatic().getPxActor();
	}

	size_t ptrOffset;
	if (TAdd || TSimRunning || TIsNonSimObject)
	{
		// Np buffers are still valid when the object gets removed while the sim is running.
		// Furthermore, for non-simulation objects, there exists no shape buffer in the simulation controller
		// and we need to fetch from Np all the time.

		ptrOffset = shapePtrOffset - Scb::Shape::getScOffset();

		if (TIsDynamic)
			nbShapes = NpRigidDynamicGetShapes(dynamicObject, shapes);
		else
			nbShapes = NpRigidStaticGetShapes(staticObject, shapes);
	}

	if ((!TSimRunning) && (!TIsNonSimObject))
	{
		if (TAdd)
		{
			if (TIsDynamic)
				s.addBody(dynamicObject.getScBody(), shapes, nbShapes, shapePtrOffset);
			else
				s.addStatic(staticObject.getScStatic(), shapes, nbShapes, shapePtrOffset);
		}
		else
		{
			ptrOffset = -Scb::Shape::getScOffset();

			if (TIsDynamic)
				s.removeBody(dynamicObject.getScBody(), scShapes, wakeOnLostTouch);
			else
				s.removeStatic(staticObject.getScStatic(), scShapes, wakeOnLostTouch);

			shapes = reinterpret_cast<void*const*>(const_cast<Sc::ShapeCore*const*>(scShapes.begin()));
			nbShapes = scShapes.size();
		}
	}

	Scb::Scene* scbScene = rigidObject.getScbScene();
	Scb::Scene* shapeScenePtr = scbScene;
	Scb::ControlState::Enum controlState = rigidObject.getControlState();

	if (!TSimRunning)
	{
		// hacky: in the non-buffered case the rigid objects might not have been updated properly at this point, so it's done explicitly.

		if (TAdd)
		{
			PX_ASSERT(shapeScenePtr == scbScene);
			controlState = Scb::ControlState::eIN_SCENE;
		}
		else
		{
			shapeScenePtr = NULL;
			controlState = Scb::ControlState::eNOT_IN_SCENE;
		}
	}

	for(PxU32 i=0; i < nbShapes; i++)
	{
		Scb::Shape& scbShape = *Ps::pointerOffset<Scb::Shape*>(shapes[i], (ptrdiff_t)ptrOffset);

		if (!TSimRunning)
		{
			PX_ASSERT(pxActor);
			PX_ASSERT(scbScene);

			if (TAdd)
			{
				scbShape.setControlStateIfExclusive(shapeScenePtr, controlState);

				if (!TIsNonSimObject)
					NpShapeIncRefCount(scbShape);  // simulation increases the refcount to avoid that shapes get destroyed while the sim is running

				scbScene->addShapeToPvd(scbShape, *pxActor);
			}
			else
			{
				scbShape.checkUpdateOnRemove<true>(scbScene);

				scbScene->removeShapeFromPvd(scbShape, *pxActor);

				scbShape.setControlStateIfExclusive(shapeScenePtr, controlState);

				if (!TIsNonSimObject)
					NpShapeDecRefCount(scbShape);  // see comment in the "TAdd" section above
			}
		}
		else
			scbShape.setControlStateIfExclusive(shapeScenePtr, controlState);
	}
}

//////////////////////////////////////////////////////////////////////////

template <bool TSimRunning, bool TAdd, bool TIsDynamic, bool TIsNonSimObject, class T>
PX_FORCE_INLINE static void addOrRemoveRigidObject(Sc::Scene& s, T& rigidObject, bool wakeOnLostTouch)
{
	PX_ASSERT(TIsDynamic || (rigidObject.getScbType() == ScbType::RIGID_STATIC));
	if (TSimRunning && TIsNonSimObject && TAdd)
		PX_ASSERT(rigidObject.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION);
	if (TSimRunning && TIsNonSimObject&& (!TAdd))
		PX_ASSERT(rigidObject.isSimDisabledInternally());
	if (!TSimRunning && TIsNonSimObject)
		PX_ASSERT(rigidObject.isSimDisabledInternally());  // when the simulation flag gets cleared on an object with pending removal, only the core flag knows that internally it is still a non-simulation object.

	if(s.getBatchRemove())
	{
		addOrRemoveRigidObjectInternal<TSimRunning, TAdd, TIsDynamic, TIsNonSimObject>(s, rigidObject, s.getBatchRemove()->removedShapes,wakeOnLostTouch);
	}
	else
	{
		Ps::InlineArray<const Sc::ShapeCore*, 64> scShapes;
		addOrRemoveRigidObjectInternal<TSimRunning, TAdd, TIsDynamic, TIsNonSimObject>(s, rigidObject, scShapes,wakeOnLostTouch);
	}
}

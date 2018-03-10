/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IMPACT_EMITTER_ACTOR_H__
#define __IMPACT_EMITTER_ACTOR_H__

////////////////////////////////////////////////////////////////////////////////

#include "NxImpactEmitterActor.h"
#include "ImpactEmitterAsset.h"
#include "EmitterScene.h"
#include "ApexActor.h"
#include "ApexFIFO.h"
#include "ApexSharedUtils.h"
#include "PsUserAllocated.h"
#include "ApexRand.h"
#include "ApexRWLockable.h"

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterBaseEvent;

namespace NxParameterized
{
class Interface;
};

namespace physx
{
namespace apex
{

class NiInstancedObjectSimulation;
#if NX_SDK_VERSION_MAJOR == 2
class NxExplosionActor;
class NxExplosionAsset;
class NxExplosionActorDesc;
#endif

namespace emitter
{

////////////////////////////////////////////////////////////////////////////////

struct ImpactEventTriggerParams
{
	physx::PxVec3 hitPos;
	physx::PxVec3 hitDir;
	physx::PxVec3 hitNorm;

	ImpactEventTriggerParams() {}
	ImpactEventTriggerParams(const physx::PxVec3& p, const physx::PxVec3& d, const physx::PxVec3& n) : hitPos(p), hitDir(d), hitNorm(n) {}
};

class ImpactEmitterBaseEvent : public physx::UserAllocated
{
public:
	ImpactEmitterBaseEvent(ImpactEmitterActor* emitterActor) : mOwner(emitterActor), mValid(false) {}
	virtual ~ImpactEmitterBaseEvent() {}
	virtual void removeActorReference(ApexActor* a) = 0;

	virtual bool isValid()
	{
		return mValid;
	}
	virtual void trigger(const ImpactEventTriggerParams& params) = 0;
	virtual void setPreferredRenderVolume(NxApexRenderVolume*) = 0;

	virtual void submitTasks(physx::PxTaskManager*) {}
	virtual void setTaskDependencies(physx::PxTask*) {}

protected:
	ImpactEmitterActor* 	mOwner;
	bool mValid;
};

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterParticleEvent : public ImpactEmitterBaseEvent
{
public:
	static const size_t MAX_PARTICLES = 2048;

	enum eAxisType { AXIS_INCIDENT = 0, AXIS_NORMAL, AXIS_REFLECTION };

	ImpactEmitterParticleEvent(NxParameterized::Interface* eventParamPtr, ImpactEmitterAsset& asset, ImpactEmitterActor* emitterActor);

	virtual ~ImpactEmitterParticleEvent();

	virtual void removeActorReference(ApexActor* a)
	{
		if (mParticleInjector == a)
		{
			mParticleInjector = NULL;
		}
	}

	virtual void trigger(const ImpactEventTriggerParams& params);

	virtual void submitTasks(physx::PxTaskManager* tm);
	virtual void setTaskDependencies(physx::PxTask* tickTask);
	virtual void setPreferredRenderVolume(physx::apex::NxApexRenderVolume* vol);

protected:
	void run();

	void trigger(const physx::PxVec3& hitPos, const physx::PxVec3& hitDir, const physx::PxVec3& hitNorm);
	void initParticle(const physx::PxVec3& pos, const physx::PxVec3 basis[], physx::PxVec3& outPos, physx::PxVec3& outVel, physx::PxF32& outLife);

	typedef physx::Array<ImpactEventTriggerParams> TriggerQueue;
	TriggerQueue	mTriggerQueue;
	physx::PxTask* 	mEventTask;
	friend class ParticleEventTask;

	eAxisType		mAxis;
	physx::PxF32	mMinAngle;
	physx::PxF32	mMaxAngle;
	physx::PxF32	mMinSpeed;
	physx::PxF32	mMaxSpeed;
	physx::PxF32	mMinLife;
	physx::PxF32	mMaxLife;
	physx::PxU32	mParticleSpawnCount;
	NiIosInjector* 	mParticleInjector;
};

////////////////////////////////////////////////////////////////////////////////
#if NX_SDK_VERSION_MAJOR == 2
class ImpactEmitterExplosionEvent : public ImpactEmitterBaseEvent
{
public:
	ImpactEmitterExplosionEvent(NxParameterized::Interface* eventParamPtr, ImpactEmitterAsset& asset, ImpactEmitterActor* emitterActor);

	virtual ~ImpactEmitterExplosionEvent();

	virtual void removeActorReference(ApexActor* a)
	{
		PX_UNUSED(a);
	}
	virtual void setPreferredRenderVolume(NxApexRenderVolume*) {}
	virtual void trigger(const ImpactEventTriggerParams& params);

protected:
	physx::PxF32 mDuration;
	NxExplosionAsset* mExplosionAsset;
};
#endif
////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterEventSet : public physx::UserAllocated
{
public:
	struct EventSetEntry
	{
		physx::PxF32 dly;
		ImpactEmitterBaseEvent* evnt;

		EventSetEntry() : dly(0.0f), evnt(NULL) {};
		EventSetEntry(physx::PxF32 d, ImpactEmitterBaseEvent* e) : dly(d), evnt(e) {};
	};

	ImpactEmitterEventSet() {};
	virtual ~ImpactEmitterEventSet();

	bool AddEvent(NxParameterized::Interface* eventParamPtr, ImpactEmitterAsset& asset, ImpactEmitterActor* emitterActor);

	physx::Array<EventSetEntry> entries;

};

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterActor : public NxImpactEmitterActor, public EmitterActorBase, public NxApexResource, public ApexResource, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	class QueuedImpactEvent
	{
	public:
		QueuedImpactEvent() {};
		QueuedImpactEvent(const physx::PxVec3& p, const physx::PxVec3& d, const physx::PxVec3& n, physx::PxF32 t, ImpactEmitterBaseEvent* e)
			: triggerParams(p, d, n), triggerTime(t), eventDef(e) {}

		bool trigger(physx::PxF32 time);

	protected:
		ImpactEventTriggerParams	triggerParams;
		physx::PxF32				triggerTime;
		ImpactEmitterBaseEvent* 	eventDef;
	};
	typedef physx::Array<QueuedImpactEvent> ImpactEventQueue;

#if NX_SDK_VERSION_MAJOR == 2
	class ActiveExplosion
	{
	public:
		ActiveExplosion() : expActor(NULL), duration(1.0f), endTime(0.0f) {};
		ActiveExplosion(NxExplosionActor* actor, physx::PxF32 d, physx::PxF32 t) : expActor(actor), duration(d), endTime(t + d) {};

		bool checkExpire(physx::PxF32 time)
		{
			if (time > endTime)
			{
				releaseActor();
				return true;
			}
			else
			{
				return false;
			}
		}

		void updateScale(physx::PxF32 time);
		void releaseActor();

	protected:

		NxExplosionActor* expActor;
		physx::PxF32 duration;
		physx::PxF32 endTime;

		//lionel: only for the purpose of visualizing the ray
		friend class ImpactEmitterActor;
	};
	typedef physx::Array<ActiveExplosion> ActiveExplosionList;
#endif

	ImpactEmitterActor(const NxImpactEmitterActorDesc&, ImpactEmitterAsset&, NxResourceList&, EmitterScene&);
	ImpactEmitterActor(const NxParameterized::Interface&, ImpactEmitterAsset&, NxResourceList&, EmitterScene&);

	~ImpactEmitterActor();

	NxApexRenderable* 			getRenderable()
	{
		return NULL;
	}
	NxApexActor* 				getNxApexActor()
	{
		return this;
	}

	void						getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const;
	physx::PxF32				getActivePhysicalLod() const;
	void						forcePhysicalLod(physx::PxF32 lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		NX_WRITE_ZONE();
		ApexActor::setEnableDebugVisualization(state);
	}

	/* NxApexResource, ApexResource */
	void				        release();
	physx::PxU32				getListIndex() const
	{
		return m_listIndex;
	}
	void				        setListIndex(class NxResourceList& list, physx::PxU32 index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	/* LODNode */
	physx::PxF32 getBenefit();
	physx::PxF32 setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit);

	/* EmitterActorBase */
	void                        destroy();
	NxApexAsset*		        getOwner() const;
	void						visualize(NiApexRenderDebug&);
#if NX_SDK_VERSION_MAJOR == 2
	void						setPhysXScene(NxScene*)  {}
	NxScene*					getPhysXScene() const
	{
		return NULL;
	}
#elif NX_SDK_VERSION_MAJOR == 3
	void						setPhysXScene(PxScene*)  {}
	PxScene*					getPhysXScene() const
	{
		return NULL;
	}
#endif

	void						submitTasks();
	void						setTaskDependencies();
	void						fetchResults();

	NiApexScene*                getApexScene() const;
	void						removeActorAtIndex(physx::PxU32 index);
	void						setPreferredRenderVolume(physx::apex::NxApexRenderVolume*);

	NxImpactEmitterAsset* 		getEmitterAsset() const;

	/* Override some asset settings at run time */

	/* Actor callable methods */
	void						registerImpact(const physx::PxVec3& hitPos, const physx::PxVec3& hitDir, const physx::PxVec3& surfNorm, physx::PxU32 surfType);

	void							setSeed(PxU32 seed)
	{
		mRand.setSeed(seed);
	}

protected:
	void						tick();

#if NX_SDK_VERSION_MAJOR == 2
	explosion::NxExplosionEnvSettings mExplosionEnv;
	NxParameterized::Interface* mExplEnvParams;
	physx::Array<ActiveExplosion> mActiveExplosions;
	void createExplosion(NxExplosionAsset* asset, NxParameterized::Interface& desc, physx::PxF32 duration);
	/* Actor local data */
#endif
	physx::PxF32  mTime;
	physx::Array<ImpactEmitterEventSet>   mEventSets;
	
	physx::Array<NiIosInjector*> mActiveParticleInjectors;
	ImpactEventQueue mPendingEvents;

	ImpactEmitterAsset* 		mAsset;
	EmitterScene* 				mScene;

	physx::PxTask* 		mTickTask;
	friend class ImpactEmitterTickTask;

	physx::QDSRand				mRand;

	friend class ImpactEmitterParticleEvent;
#if NX_SDK_VERSION_MAJOR == 2
	friend class ImpactEmitterExplosionEvent;
#endif
};

}
}
} // end namespace physx::apex

////////////////////////////////////////////////////////////////////////////////
#endif

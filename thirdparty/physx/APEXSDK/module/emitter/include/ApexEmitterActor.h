/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_ACTOR_H__
#define __EMITTER_ACTOR_H__

#include "NxApex.h"

#include "NxApexEmitterActor.h"
#include "ApexActor.h"
#include "ApexInterface.h"
#include "EmitterScene.h"
#include "NxEmitterLodParamDesc.h"
#include "ApexRWLockable.h"
#include "ApexRand.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{

class NxApexEmitterAsset;

namespace emitter
{
class ApexEmitterAsset;
class EmitterGeom;

class ApexEmitterActor : public NxApexEmitterActor, public EmitterActorBase, public NxApexResource, public ApexResource, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* NxApexEmitterActor methods */
	ApexEmitterActor(const NxApexEmitterActorDesc&, ApexEmitterAsset&, NxResourceList&, EmitterScene&);
	~ApexEmitterActor();

	NxApexEmitterAsset*             getEmitterAsset() const;
	physx::PxMat44					getGlobalPose() const
	{
		NX_READ_ZONE();
		return PxMat44(mPose);
	}
	void							setCurrentPose(const physx::PxMat44& pose)
	{
		NX_WRITE_ZONE();
		mPose = pose;
	}
	void							setCurrentPosition(const physx::PxVec3& pos)
	{
		NX_WRITE_ZONE();
		mPose.t = pos;
	}
	NxApexRenderable* 				getRenderable()
	{
		NX_READ_ZONE();
		return NULL;
	}
	NxApexActor* 					getNxApexActor()
	{
		NX_READ_ZONE();
		return this;
	}
	void							removeActorAtIndex(physx::PxU32 index);

	void							getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const;
	physx::PxF32					getActivePhysicalLod() const;
	void							forcePhysicalLod(physx::PxF32 lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		NX_WRITE_ZONE();
		ApexActor::setEnableDebugVisualization(state);
	}

	NxEmitterExplicitGeom* 			isExplicitGeom();

	const NxEmitterLodParamDesc&	getLodParamDesc() const
	{
		NX_READ_ZONE();
		return mLodParams;
	}
	void							setLodParamDesc(const NxEmitterLodParamDesc& d);

	/* LODNode */
	physx::PxF32					getBenefit();
	physx::PxF32					setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit);

	/* NxApexResource, ApexResource */
	void				            release();
	physx::PxU32				    getListIndex() const
	{
		NX_READ_ZONE();
		return m_listIndex;
	}
	void				            setListIndex(class NxResourceList& list, physx::PxU32 index)
	{
		NX_WRITE_ZONE();
		m_list = &list;
		m_listIndex = index;
	}

	/* EmitterActorBase */
	void                            destroy();
	NxApexAsset*		            getOwner() const;
	void							visualize(NiApexRenderDebug& renderDebug);
#if NX_SDK_VERSION_MAJOR == 2
	void							setPhysXScene(NxScene* s)
	{
		NX_WRITE_ZONE();
		mNxScene = s;
	}
	NxScene*						getPhysXScene() const
	{
		NX_READ_ZONE();
		return mNxScene;
	}
	NxScene*						mNxScene;
#elif NX_SDK_VERSION_MAJOR == 3
	void							setPhysXScene(PxScene* s)
	{
		NX_WRITE_ZONE();
		mNxScene = s;
	}
	PxScene*						getPhysXScene() const
	{
		NX_READ_ZONE();
		return mNxScene;
	}
	PxScene*						mNxScene;
#endif
	void							submitTasks();
	void							setTaskDependencies();
	void							fetchResults();

	void							setDensityRange(const NxRange<physx::PxF32>& r)
	{
		NX_WRITE_ZONE();
		mDensityRange = r;
	}
	void							setRateRange(const NxRange<physx::PxF32>& r)
	{
		NX_WRITE_ZONE();
		mRateRange = r;
	}
	void							setVelocityRange(const NxRange<physx::PxVec3>& r)
	{
		NX_WRITE_ZONE();
		mVelocityRange = r;
	}
	void							setLifetimeRange(const NxRange<physx::PxF32>& r)
	{
		NX_WRITE_ZONE();
		mLifetimeRange = r;
	}

	void							getRateRange(NxRange<physx::PxF32>& r) const
	{
		NX_READ_ZONE();
		r = mRateRange;
	}

	void							setOverlapTestCollisionGroups(physx::PxU32 g)
	{
		NX_WRITE_ZONE();
		mOverlapTestCollisionGroups = g;
	}
#if NX_SDK_VERSION_MAJOR == 2
	void							setOverlapTestCollisionGroupsMask(NxGroupsMask*);
#endif

	physx::PxU32					getOverlapTestCollisionGroups() const
	{
		NX_READ_ZONE();
		return mOverlapTestCollisionGroups;
	}
#if NX_SDK_VERSION_MAJOR == 2
	const NxGroupsMask*				getOverlapTestCollisionGroupsMask() const
	{
		NX_READ_ZONE();
		return mShouldUseGroupsMask ? &mOverlapTestCollisionGroupsMask : NULL;
	}
#endif

	virtual void				setDensityGridPosition(const physx::PxVec3 &pos);

	virtual void setApexEmitterValidateCallback(NxApexEmitterValidateCallback *callback) 
	{
		NX_WRITE_ZONE();
		mEmitterValidateCallback = callback;
	}

	PX_DEPRECATED void setObjectScale(PxF32 scale)
	{
		NX_WRITE_ZONE();
		setCurrentScale(scale);
	}

	PX_DEPRECATED PxF32 getObjectScale(void) const
	{
		NX_READ_ZONE();
		return getObjectScale();
	}

	virtual void				setCurrentScale(PxF32 scale) 
	{
		NX_WRITE_ZONE();
		mObjectScale = scale;
		if (mInjector)
		{
			mInjector->setObjectScale(mObjectScale);
		}
	}

	virtual PxF32				getCurrentScale(void) const
	{
		NX_READ_ZONE();
		return mObjectScale;
	}

	void							startEmit(bool persistent);
	void							stopEmit();
	bool							isEmitting() const
	{
		NX_READ_ZONE();
		return mDoEmit;
	}

	void							emitAssetParticles(bool enable)
	{
		NX_WRITE_ZONE();
		mEmitAssetParticles = enable;
	}
	bool							getEmitAssetParticles() const
	{
		NX_READ_ZONE();
		return mEmitAssetParticles;
	}

#if NX_SDK_VERSION_MAJOR == 2
	void							setAttachActor(NxActor* a)
	{
		NX_WRITE_ZONE();
		mAttachActor = a;
	}
	const NxActor* 					getAttachActor() const
	{
		NX_READ_ZONE();
		return mAttachActor;
	}
	NxActor* 						mAttachActor;
#elif NX_SDK_VERSION_MAJOR == 3
	void							setAttachActor(PxActor* a)
	{
		NX_WRITE_ZONE();
		mAttachActor = a;
	}
	const PxActor* 					getAttachActor() const
	{
		NX_READ_ZONE();
		return mAttachActor;
	}
	PxActor* 						mAttachActor;
#endif
	void							setAttachRelativePose(const physx::PxMat44& p)
	{
		NX_WRITE_ZONE();
		mAttachRelativePose = p;
	}

	const physx::PxMat44			getAttachRelativePose() const
	{
		NX_READ_ZONE();
		return PxMat44(mAttachRelativePose);
	}
	physx::PxF32					getObjectRadius() const
	{
		NX_READ_ZONE();
		return mObjectRadius;
	}
	void							setPreferredRenderVolume(physx::apex::NxApexRenderVolume*);

	static physx::PxVec3			random(const NxRange<physx::PxVec3>& range, QDSRand& rand);
	static physx::PxF32				random(const NxRange<physx::PxF32>& range, QDSRand& rand);

	void							setSeed(PxU32 seed)
	{
		mRand.setSeed(seed);
	}

	virtual PxU32					getSimParticlesCount() const
	{
		NX_READ_ZONE();
		return	mInjector->getSimParticlesCount();
	}

	virtual PxU32					getActiveParticleCount() const;

protected:
	physx::PxU32					computeNbEmittedFromRate(physx::PxF32 dt, physx::PxF32 currate);
	bool							overlapsWithCollision(const physx::PxBounds3& bounds);
	bool							overlapsWithCollision(const physx::PxVec3& pos);
	void							emitObjects(const physx::PxMat34Legacy& pose, physx::PxU32 toEmitNum, bool useFullVolume);
	void							emitObjects(const physx::Array<physx::PxVec3>& positions, physx::PxBounds3& bounds);
	void							tick();

	NxApexEmitterValidateCallback	*mEmitterValidateCallback;
	NiIosInjector* 					mInjector;
	NiInstancedObjectSimulation		*mIOS;
	ApexEmitterAsset* 		    	mAsset;
	EmitterScene* 					mScene;
	physx::PxBounds3				mOverlapAABB;
	physx::PxBounds3				mLastNonEmptyOverlapAABB;

#if NX_SDK_VERSION_MAJOR == 2
	NxGroupsMask						mOverlapTestCollisionGroupsMask;
#endif

	physx::PxF32						mObjectScale;
	/* runtime state */
	physx::PxMat34Legacy				mPose;
	physx::Array<physx::PxMat34Legacy>	mPoses;
	bool								mFirstStartEmitCall;
	bool								mDoEmit;
	bool								mEmitAssetParticles;
	bool                            	mPersist;
	physx::PxF32						mRemainder;
	physx::PxF32						mEmitterVolume;
	bool								mIsOldPoseInitialized;
	physx::PxMat34Legacy				mOldPose;
	NxRange<physx::PxF32>				mDensityRange;
	NxRange<physx::PxF32>				mRateRange;
	NxRange<physx::PxVec3>				mVelocityRange;
	NxRange<physx::PxF32>				mLifetimeRange;
	physx::PxF32						mObjectRadius;
	physx::PxF32						mEmitDuration;
	physx::PxF32						mDescEmitDuration;
	physx::PxU32						mOverlapTestCollisionGroups;
	bool								mShouldUseGroupsMask;
	physx::Array<IosNewObject>			mNewObjectArray;
	physx::Array<physx::PxVec3>			mNewPositions;
	physx::Array<physx::PxVec3>			mNewVelocities;
	physx::Array<physx::PxU32>			mNewUserData;

	EmitterGeom*						mExplicitGeom;
	NxEmitterLodParamDesc				mLodParams;
	physx::PxMat34Legacy				mAttachRelativePose;

	physx::QDSRand						mRand;

	class ApexEmitterTickTask : public physx::PxTask
	{
	private:
		ApexEmitterTickTask& operator=(const ApexEmitterTickTask&);

	public:
		ApexEmitterTickTask(ApexEmitterActor& actor) : mActor(actor) {}

		const char* getName() const
		{
			return "ApexEmitterActor::TickTask";
		}
		void run()
		{
			mActor.tick();
		}

	protected:
		ApexEmitterActor& mActor;
	};
	ApexEmitterTickTask 				mTickTask;

	friend class EmitterScene;
};

}
}
} // end namespace physx::apex

#endif

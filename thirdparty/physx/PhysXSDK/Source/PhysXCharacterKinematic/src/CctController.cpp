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

#include "PxController.h"
#include "CctController.h"
#include "CctBoxController.h"
#include "CctCharacterControllerManager.h"
#include "PxScene.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"
#include "PxExtensionsAPI.h"
#include "PsMathUtils.h"
#include "PsUtilities.h"
#include "PxPhysics.h"

using namespace physx;
using namespace Cct;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Controller::Controller(const PxControllerDesc& desc, PxScene* s) :
	mScene					(s),
	mPreviousSceneTimestamp	(0xffffffff),
	mManager				(NULL),
	mGlobalTime				(0.0f),
	mPreviousGlobalTime		(0.0f),
	mProxyDensity			(0.0f),
	mProxyScaleCoeff		(0.0f),
	mCollisionFlags			(0),
	mCachedStandingOnMoving	(false)
{
	mType								= PxControllerShapeType::eFORCE_DWORD;

	mUserParams.mNonWalkableMode		= desc.nonWalkableMode;
	mUserParams.mSlopeLimit				= desc.slopeLimit;
	mUserParams.mContactOffset			= desc.contactOffset;
	mUserParams.mStepOffset				= desc.stepOffset;
	mUserParams.mInvisibleWallHeight	= desc.invisibleWallHeight;
	mUserParams.mMaxJumpHeight			= desc.maxJumpHeight;
	mUserParams.mHandleSlope			= desc.slopeLimit!=0.0f;

	mReportCallback						= desc.reportCallback;
	mBehaviorCallback					= desc.behaviorCallback;
	mUserData							= desc.userData;

	mKineActor							= NULL;
	mPosition							= desc.position;
	mProxyDensity						= desc.density;
	mProxyScaleCoeff					= desc.scaleCoeff;

	mCctModule.mVolumeGrowth			= desc.volumeGrowth;

	mDeltaXP							= PxVec3(0);
	mOverlapRecover						= PxVec3(0);

	mUserParams.mUpDirection = PxVec3(0.0f);
	setUpDirectionInternal(desc.upDirection);

	// PT: register ourself as a deletion listener, to be called the SDK whenever an object is deleted
	PX_ASSERT(s);
	PxPhysics& physics = s->getPhysics();
	physics.registerDeletionListener(*this, PxDeletionEventFlag::eUSER_RELEASE);
}

Controller::~Controller()
{
	if(mScene)
	{
		PxPhysics& physics = mScene->getPhysics();
		physics.unregisterDeletionListener(*this);

		if(mKineActor)
			mKineActor->release();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::onRelease(const PxBase* observed, void* /*userData*/, PxDeletionEventFlag::Enum deletionEvent)
{
	PX_ASSERT(deletionEvent == PxDeletionEventFlag::eUSER_RELEASE);  // the only type we registered for
	PX_UNUSED(deletionEvent);

	if(mManager && mManager->mLockingEnabled)
	{
		mWriteLock.lock();
		mCctModule.onRelease(*observed);
		mWriteLock.unlock();
	}
	else
	{
		mCctModule.onRelease(*observed);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::onOriginShift(const PxVec3& shift)
{
	mPosition -= shift;
	
	if (mManager && mManager->mLockingEnabled)
	{
		mWriteLock.lock();
		mCctModule.onOriginShift(shift);	
		mWriteLock.unlock();
	}
	else
	{
		mCctModule.onOriginShift(shift);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::setUpDirectionInternal(const PxVec3& up)
{
	PX_CHECK_MSG(up.isNormalized(), "CCT: up direction must be normalized");

	if(mUserParams.mUpDirection==up)
		return;

//	const PxQuat q = Ps::computeQuatFromNormal(up);
	const PxQuat q = Ps::rotationArc(PxVec3(1.0f, 0.0f, 0.0f), up);

	mUserParams.mQuatFromUp		= q;
	mUserParams.mUpDirection	= up;

	// Update kinematic actor
	/*if(mKineActor)
	{
		PxTransform pose = mKineActor->getGlobalPose();
		pose.q = q;
		mKineActor->setGlobalPose(pose);	
	}*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::releaseInternal()
{
	mManager->releaseController(*getPxController());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::getInternalState(PxControllerState& state) const
{
	if(mManager->mLockingEnabled)
	{
		mWriteLock.lock();
	}
	state.deltaXP				= mDeltaXP;
	state.touchedShape			= mCctModule.mTouchedShape;
	state.touchedActor			= const_cast<PxRigidActor*>(mCctModule.mTouchedActor);
	state.touchedObstacleHandle	= mCctModule.mTouchedObstacleHandle;
	state.standOnAnotherCCT		= (mCctModule.mFlags & STF_TOUCH_OTHER_CCT)!=0;
	state.standOnObstacle		= (mCctModule.mFlags & STF_TOUCH_OBSTACLE)!=0;
	state.isMovingUp			= (mCctModule.mFlags & STF_IS_MOVING_UP)!=0;
	state.collisionFlags		= mCollisionFlags;

	if (mManager->mLockingEnabled)
	{
		mWriteLock.unlock();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::getInternalStats(PxControllerStats& stats) const
{
	stats.nbFullUpdates		= mCctModule.mNbFullUpdates;
	stats.nbPartialUpdates	= mCctModule.mNbPartialUpdates;
	stats.nbIterations		= mCctModule.mNbIterations;
	stats.nbTessellation	= mCctModule.mNbTessellation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Controller::setPos(const PxExtendedVec3& pos)
{
	mPosition = pos;

	// Update kinematic actor
	if(mKineActor)
	{
		PxTransform targetPose = mKineActor->getGlobalPose();
		targetPose.p = toVec3(mPosition);  // LOSS OF ACCURACY
		targetPose.q = mUserParams.mQuatFromUp;
		mKineActor->setKinematicTarget(targetPose);	
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Controller::createProxyActor(PxPhysics& sdk, const PxGeometry& geometry, const PxMaterial& material)
{
	// PT: we don't disable raycasting or CD because:
	// - raycasting is needed for visibility queries (the SDK otherwise doesn't know about the CCTS)
	// - collision is needed because the only reason we create actors there is to handle collisions with dynamic shapes
	// So it's actually wrong to disable any of those.

	PxTransform globalPose;
	globalPose.p = toVec3(mPosition);	// LOSS OF ACCURACY
	globalPose.q = mUserParams.mQuatFromUp;

	mKineActor = sdk.createRigidDynamic(globalPose);
	if(!mKineActor)
		return false;

	mKineActor->createShape(geometry, material);
	mKineActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	PxRigidBodyExt::updateMassAndInertia(*mKineActor, mProxyDensity);
	mScene->addActor(*mKineActor);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxShape* Controller::getKineShape() const
{
	// PT: TODO: cache this and avoid the virtual call
	PxShape* shape = NULL;
	PxU32 nb = mKineActor->getShapes(&shape, 1);
	PX_ASSERT(nb==1);
	PX_UNUSED(nb);
	return shape;
}

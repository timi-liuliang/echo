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

#ifndef CCT_CONTROLLER
#define CCT_CONTROLLER

/* Exclude from documentation */
/** \cond */

#include "CctCharacterController.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PxDeletionListener.h"

namespace physx
{

class PxPhysics;
class PxScene;
class PxRigidDynamic;
class PxGeometry;
class PxMaterial;

namespace Cct
{
	class CharacterControllerManager;

	class Controller : public Ps::UserAllocated, public PxDeletionListener
	{
		PX_NOCOPY(Controller)
	public:
														Controller(const PxControllerDesc& desc, PxScene* scene);
		virtual											~Controller();

					void								releaseInternal();
					void								getInternalState(PxControllerState& state)	const;
					void								getInternalStats(PxControllerStats& stats)	const;

		// PxDeletionListener
		virtual		void								onRelease(const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent);
		//~PxDeletionListener

		virtual		PxF32								getHalfHeightInternal()				const	= 0;
		virtual		bool								getWorldBox(PxExtendedBounds3& box)	const	= 0;
		virtual		PxController*						getPxController()							= 0;

					void								onOriginShift(const PxVec3& shift);

					PxControllerShapeType::Enum			mType;
		// User params
					CCTParams							mUserParams;
					PxUserControllerHitReport*			mReportCallback;
					PxControllerBehaviorCallback*		mBehaviorCallback;
					void*								mUserData;
		// Internal data
					SweepTest							mCctModule;			// Internal CCT object. Optim test for Ubi.
					PxRigidDynamic*						mKineActor;			// Associated kinematic actor
					PxExtendedVec3						mPosition;			// Current position
					PxVec3								mDeltaXP;
					PxVec3								mOverlapRecover;
					PxScene*							mScene;				// Handy scene owner
					PxU32								mPreviousSceneTimestamp;
					CharacterControllerManager*			mManager;			// Owner manager
					PxF32								mGlobalTime;
					PxF32								mPreviousGlobalTime;
					PxF32								mProxyDensity;		// Density for proxy actor
					PxF32								mProxyScaleCoeff;	// Scale coeff for proxy actor
					PxControllerCollisionFlags			mCollisionFlags;	// Last known collision flags (PxControllerFlag)
					bool								mCachedStandingOnMoving;
		mutable		Ps::Mutex							mWriteLock;			// Lock used for guarding touched pointers and cache data from overwriting 
																			// during onRelease call.
	protected:
		// Internal methods
					void								setUpDirectionInternal(const PxVec3& up);
					PxShape*							getKineShape()	const;
					bool								createProxyActor(PxPhysics& sdk, const PxGeometry& geometry, const PxMaterial& material);
					bool								setPos(const PxExtendedVec3& pos);
					void								findTouchedObject(const PxControllerFilters& filters, const PxObstacleContext* obstacleContext, const PxVec3& upDirection);
					bool								rideOnTouchedObject(SweptVolume& volume, const PxVec3& upDirection, PxVec3& disp, const PxObstacleContext* obstacleContext);
					PxControllerCollisionFlags			move(SweptVolume& volume, const PxVec3& disp, PxF32 minDist, PxF32 elapsedTime, const PxControllerFilters& filters, const PxObstacleContext* obstacles, bool constrainedClimbingMode);
					bool								filterTouchedShape(const PxControllerFilters& filters);
	};

} // namespace Cct

}

/** \endcond */
#endif

/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MIRROR_SCENE_H

#define MIRROR_SCENE_H

#include "NxPhysXSDKVersion.h"

#if NX_SDK_VERSION_MAJOR == 3

#include "NxMirrorScene.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PxSimulationEventCallback.h"
#include "PxClient.h"
#include "PsHashMap.h"
#include "PxDeletionListener.h"
#include "PxTransform.h"
#include "PsArray.h"

namespace physx
{
	class PxScene;
	class PxRigidDynamic;
	class PxMaterial;
	class PxRigidActor;
	namespace apex
	{
		class MirrorScene;
		class MirrorActor;

		enum MirrorCommandType
		{
			MCT_CREATE_ACTOR,
			MCT_RELEASE_ACTOR,
			MCT_UPDATE_POSE,
			MCT_LAST
		};

		class MirrorCommand
		{
		public:
			MirrorCommand(MirrorCommandType type,MirrorActor *ma)
			{
				mType = type;
				mMirrorActor = ma;
			}
			MirrorCommand(MirrorCommandType type,MirrorActor *ma,const physx::PxTransform &pose)
			{
				mType = type;
				mMirrorActor = ma;
				mPose = pose;
			}
			MirrorCommandType	mType;
			MirrorActor			*mMirrorActor;
			physx::PxTransform	mPose;
		};

		class MirrorActor : public physx::shdfnd::UserAllocated
		{
		public:

			// The constructor is only ever called from the PrimaryScene thread
			MirrorActor(size_t actorHash,physx::PxRigidActor &actor,MirrorScene &parentScene);
			// The destructor is only ever called from the MirrorScene thread
			virtual ~MirrorActor(void);

			// Increments the reference count for the number of shapes on this actor
			// currently inside the trigger volume.
			// Only ever called from the primary scene thread
			void addShape(void)
			{
				mShapeCount++;
			}

			// Decrements the reference count for the number of shapes on this actor
			// which are currently in the trigger volume. 
			// If the reference count goes to zero, then no part of this actor is 
			// any longer inside the trigger volume and it's mirror should in turn be released
			// This is only ever called from the PrimaryScene thread
			bool removeShape(void)
			{
				mShapeCount--;
				return mShapeCount == 0;
			}

			// This method is called when the reference count goes to zero and/or the primary
			// actor is released.
			// This method posts on the MirrorScene thread queue for this object to be deleted
			// the next time it does an update.
			// At this point the primary scene should remove this actor from the hash table.
			void release(void);


			// Required by the PxObserver class we inherited
			virtual	PxU32 getObjectSize()const
			{
				return sizeof(MirrorActor);
			}

			// This method is only called by the PrimaryScene thread
			// If the pose of the actor we are mirroring has 
			void synchronizePose(void); // called from the primary scene thread; see if the pose of the mirrored actor has changed.

			void createActor(PxScene &scene);
			void updatePose(const PxTransform &pose);

			MirrorScene		&mMirrorScene;
			PxU32			mShapeCount;
			PxU32			mMirrorShapeCount;
			PxRigidActor	*mPrimaryActor;
			PxRigidActor	*mMirrorActor;
			bool			mReleasePosted;
			PxTransform		mPrimaryGlobalPose;
			size_t	mActorHash;	// hash in primary scene.

		private:
			MirrorActor& operator=(const MirrorActor&);
		};

		typedef physx::shdfnd::Array< MirrorCommand > MirrorCommandArray;

		class MirrorScene : public NxMirrorScene, public physx::shdfnd::UserAllocated,
			public physx::PxSimulationEventCallback, physx::PxDeletionListener
		{
		public:

			typedef physx::shdfnd::HashMap< size_t, MirrorActor * > ActorHash;
			typedef physx::shdfnd::HashMap< size_t, MirrorActor * > ShapeHash;

			enum ActorChange
			{
				AC_DELETED,
				AC_FOUND,
				AC_LOST
			};

			MirrorScene(physx::PxScene &primaryScene,
				physx::PxScene &mirrorScene,
				NxMirrorScene::MirrorFilter &mirrorFilter,
				physx::PxF32 mirrorStaticDistance,
				physx::PxF32 mirrorDynamicDistance,
				physx::PxF32 mirrorDistanceThreshold);

			NxMirrorScene::MirrorFilter & getMirrorFilter(void) const
			{
				return mMirrorFilter;
			}

			virtual void synchronizePrimaryScene(const physx::PxVec3 &cameraPos);
			virtual void synchronizeMirrorScene(void);

			virtual void release(void);

			void postCommand(const MirrorCommand &mc);


	/**
	\brief This is called when a breakable constraint breaks.
	
	\note The user should not release the constraint shader inside this call!

	\param[in] constraints - The constraints which have been broken.
	\param[in] count       - The number of constraints

	@see PxConstraint PxConstraintDesc.linearBreakForce PxConstraintDesc.angularBreakForce
	*/
	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count);

	/**
	\brief This is called during PxScene::fetchResults with the actors which have just been woken up.

	\note Only supported by rigid bodies yet.
	\note Only called on actors for which the PxActorFlag eSEND_SLEEP_NOTIFIES has been set.

	\param[in] actors - The actors which just woke up.
	\param[in] count  - The number of actors

	@see PxScene.setSimulationEventCallback() PxSceneDesc.simulationEventCallback PxActorFlag PxActor.setActorFlag()
	*/
	virtual void onWake(PxActor** actors, PxU32 count);

	/**
	\brief This is called during PxScene::fetchResults with the actors which have just been put to sleep.

	\note Only supported by rigid bodies yet.
	\note Only called on actors for which the PxActorFlag eSEND_SLEEP_NOTIFIES has been set.  

	\param[in] actors - The actors which have just been put to sleep.
	\param[in] count  - The number of actors

	@see PxScene.setSimulationEventCallback() PxSceneDesc.simulationEventCallback PxActorFlag PxActor.setActorFlag()
	*/
	virtual void onSleep(PxActor** actors, PxU32 count);

	/**
	\brief The user needs to implement this interface class in order to be notified when
	certain contact events occur.

	The method will be called for a pair of actors if one of the colliding shape pairs requested contact notification.
	You request which events are reported using the filter shader/callback mechanism (see #PxSimulationFilterShader,
	#PxSimulationFilterCallback, #PxPairFlag).
	
	Do not keep references to the passed objects, as they will be 
	invalid after this function returns.

	\param[in] pairHeader Information on the two actors whose shapes triggered a contact report.
	\param[in] pairs The contact pairs of two actors for which contact reports have been requested. See #PxContactPair.
	\param[in] nbPairs The number of provided contact pairs.

	@see PxScene.setSimulationEventCallback() PxSceneDesc.simulationEventCallback PxContactPair PxPairFlag PxSimulationFilterShader PxSimulationFilterCallback
	*/
	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);

	/*
	\brief This is called during PxScene::fetchResults with the current trigger pair events.

	Shapes which have been marked as triggers using PxShapeFlag::eTRIGGER_SHAPE will send events
	according to the pair flag specification in the filter shader (see #PxPairFlag, #PxSimulationFilterShader).

	\param[in] pairs - The trigger pairs which caused events.
	\param[in] count - The number of trigger pairs.

	@see PxScene.setSimulationEventCallback() PxSceneDesc.simulationEventCallback PxPairFlag PxSimulationFilterShader PxShapeFlag PxShape.setFlag()
	*/
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count);

	// This is a notification even that the PrimayScene actor we are mirroring has been
	// deleted.  If this is the case, we need to zero out PrimaryActor pointer so we
	// no longer attempt to access it.
	// However...we do not call release, because we should get trigger events which cause
	// the reference count to go to zero.
	virtual void onRelease(const PxBase* observed,
		void* userData,
		PxDeletionEventFlag::Enum deletionEvent);



		protected:
			virtual ~MirrorScene(void);
		private:

			void mirrorShape(const PxTriggerPair &tp);

			void createTriggerActor(const physx::PxVec3 &cameraPosition);

			physx::PxScene				&mPrimaryScene;
			physx::PxScene				&mMirrorScene;
			NxMirrorScene::MirrorFilter &mMirrorFilter;
			physx::PxF32				mMirrorStaticDistance;
			physx::PxF32				mMirrorDynamicDistance;
			physx::PxF32				mMirrorDistanceThreshold;
			physx::PxVec3				mLastCameraLocation;
			physx::PxRigidDynamic		*mTriggerActor;
			physx::PxMaterial			*mTriggerMaterial;
			physx::PxShape				*mTriggerShapeStatic;
			physx::PxShape				*mTriggerShapeDynamic;
			ActorHash					mActors;
			ShapeHash					mShapes;
			physx::shdfnd::Mutex		mMirrorCommandMutex;
			MirrorCommandArray			mMirrorCommands;
			physx::PxSimulationEventCallback	*mSimulationEventCallback;
			physx::shdfnd::Array< physx::PxTriggerPair > mTriggerPairs;
		};

	}; // end of apex namespace
}; // end of physx namespace

#endif // NX_SDK_VERSION_MAJOR

#endif

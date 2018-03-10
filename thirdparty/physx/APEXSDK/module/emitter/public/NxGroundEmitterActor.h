/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_GROUND_EMITTER_ACTOR_H
#define NX_GROUND_EMITTER_ACTOR_H

#include "NxApex.h"
#include "NxApexUtils.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class NxGroundEmitterAsset;
class NxEmitterLodParamDesc;
class NxApexRenderVolume;

/**
 \brief a user calback interface used to map raycast hitpoints to material ID
 * If an instance of this class is registered with a ground emitter actor, the actor will call
 * requestMaterialLookups() in lieu of doing raycasts.  The call will occur from within the scope
 * of the ApexScene thread, so the callback must be thread safe.
 */
class NxMaterialLookupCallback
{
public:
	/// Material request structure.
	struct MaterialRequest
	{
		physx::PxVec3	samplePosition; ///< test point position. This is filled by the emitter
		physx::PxU32	outMaterialID; ///< ID of the material at the hitpoint. This must be filled by the user
	};

	/**
	\brief Submit the meterial lookup request. This method is called by the emitter and implemented by the user
	\param requestCount [IN] number of requests
	\param reqList [IN/OUT] the pointer to the requests. samplePosition is read and materialID is written to by the user.
	*/
	virtual void requestMaterialLookups(physx::PxU32 requestCount, MaterialRequest* reqList) = 0;

	virtual ~NxMaterialLookupCallback() {}
};

///Ground Emitter actor. Uses raycasts against ground to spawn particles
class NxGroundEmitterActor : public NxApexActor
{
protected:
	virtual ~NxGroundEmitterActor() {}

public:
	/**
	\brief Returns the asset the instance has been created from.
	*/
	virtual NxGroundEmitterAsset*   getEmitterAsset() const = 0;

	/**
	\brief Returns the pose of the emitter
	*/
	virtual const physx::PxMat44	getPose() const = 0;
	/**
	\brief Sets the pose of the emitter
	*/
	virtual void				    setPose(const physx::PxMat44& pos) = 0;

	/** \brief Set the material lookup callback method that replaces raycasts */
	virtual void					setMaterialLookupCallback(NxMaterialLookupCallback*) = 0;
	/** \brief Get the material lookup callback method that replaces raycasts */
	virtual NxMaterialLookupCallback* getMaterialLookupCallback() const = 0;

	/**
	\brief Attaches the emitter to an actor
	NxActor pointer can be NULL to detach existing actor
	*/
#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/**
	\brief PhysX SDK 2.8.X.  Attaches the emitter to an actor
	NxActor pointer can be NULL to detach existing actor
	*/
	virtual void					setAttachActor(NxActor*) = 0;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/**
	\brief PhysX SDK 3.X.  Attaches the emitter to an actor
	NxActor pointer can be NULL to detach existing actor
	*/
	virtual void					setAttachActor(PxActor*) = 0;
#endif
	/**
	\brief sets the relative position of the emitter in the space of the actor to which it is attached
	*/
	virtual void					setAttachRelativePosition(const physx::PxVec3& pos) = 0;

#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/**\brief PhysX SDK 2.8.X.  Retrieves the actor, to which the emitter is attached. NULL is returned for an unattached emitter. */
	virtual const NxActor* 			getAttachActor() const = 0;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/**\brief PhysX SDK 3.X.  Retrieves the actor, to which the emitter is attached. NULL is returned for an unattached emitter. */
	virtual const PxActor* 			getAttachActor() const = 0;
#endif
	/** \brief Retrieves the relative position of the emitter in the space of the actor to which it is attached. */
	virtual const physx::PxVec3& 	getAttachRelativePosition() const = 0;

	/* Override some asset settings at run time */

	///Sets the range from which the density of particles within the volume is randomly chosen
	virtual void					setDensityRange(const NxRange<physx::PxF32>&) = 0;
	///Sets the radius. The ground emitter actor will create objects within a circle of size 'radius'.
	virtual void                    setRadius(physx::PxF32) = 0;
	///Sets The maximum raycasts number per frame.
	virtual void                    setMaxRaycastsPerFrame(physx::PxU32) = 0;
	///Sets the height from which the ground emitter will cast rays at terrain/objects opposite of the 'upDirection'.
	virtual void                    setRaycastHeight(physx::PxF32) = 0;
	/**
	\brief Sets the height above the ground to emit particles.
	 If greater than 0, the ground emitter will refresh a disc above the player's position rather than
	 refreshing a circle around the player's position.

	*/
	virtual void                    setSpawnHeight(physx::PxF32) = 0;

	///Gets the range from which the density of particles within the volume is randomly chosen
	virtual const NxRange<physx::PxF32> & getDensityRange() const = 0;
	///Gets the radius. The ground emitter actor will create objects within a circle of size 'radius'.
	virtual physx::PxF32			getRadius() const = 0;
	///Gets The maximum raycasts number per frame.
	virtual physx::PxU32			getMaxRaycastsPerFrame() const = 0;
	///Gets the height from which the ground emitter will cast rays at terrain/objects opposite of the 'upDirection'.
	virtual physx::PxF32			getRaycastHeight() const = 0;
	/**
	\brief Gets the height above the ground to emit particles.
	 If greater than 0, the ground emitter will refresh a disc above the player's position rather than
	 refreshing a circle around the player's position.
	*/
	virtual physx::PxF32			getSpawnHeight() const = 0;

	/// Sets collision groups used to cast rays
	virtual void					setRaycastCollisionGroups(physx::PxU32) = 0;
#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/// PHYSX SDK 2.8.X.  Sets collision groups mask. \sa NxGroupsMask
	virtual void					setRaycastCollisionGroupsMask(NxGroupsMask*) = 0;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/// PHYSX SDK 3.X.  Sets collision groups mask. \sa NxGroupsMask
	virtual void					setRaycastCollisionGroupsMask(physx::PxFilterData*) = 0;
#endif

	/// Gets collision groups used to cast rays
	virtual physx::PxU32			getRaycastCollisionGroups() const = 0;
#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/// PHYSX SDK 2.8.X only.  Gets collision groups mask. \sa NxGroupsMask
	virtual const NxGroupsMask* 	getRaycastCollisionGroupsMask() const = 0;
#endif

	///Emitted particles are injected to specified render volume on initial frame.
	///Set to NULL to clear the preferred volume.
	virtual void					setPreferredRenderVolume(NxApexRenderVolume* volume) = 0;
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_GROUND_EMITTER_ACTOR_H

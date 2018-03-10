/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_EMITTER_ACTOR_H
#define NX_APEX_EMITTER_ACTOR_H

#include "NxApex.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxApexEmitterAsset;
class NxEmitterExplicitGeom;
class NxEmitterLodParamDesc;
class NxApexRenderVolume;

/// Apex emitter actor class. Emits particles within a given shape.
class NxApexEmitterActor : public NxApexActor
{
protected:
	virtual ~NxApexEmitterActor() {}

public:

	/// This is an optional user validation callback interface.
	/// If the application wants to confirm/verify all emitted particles they can provide
	/// this callback interface by using the setApexEmitterValidateCallback method.
	class NxApexEmitterValidateCallback
	{
	public:
		/**
		\brief This application callback is used to verify an emitted particle position.
		If the user returns false, then the particle will not be emitted.  If the user returns true
		if will be emitted but using the position value which is passed by reference.  The application
		can choose to leave the emitter position alone, or modify it to a new location.  All locations
		are in world space.  For convenience to the application the world space emitter position is provided.
		*/
		virtual bool validateEmitterPosition(const physx::PxVec3 &emitterOrigin,physx::PxVec3 &position) = 0;
	};

	/**
	\brief Returns the asset the instance has been created from.
	*/
	virtual NxApexEmitterAsset* getEmitterAsset() const = 0;

	/**
	\brief Returns the explicit geometry for THIS ACTOR only
	*/
	virtual NxEmitterExplicitGeom* isExplicitGeom() = 0;

	/**
	\brief Gets the global pose
	*/
	virtual physx::PxMat44	     getGlobalPose() const = 0;
	/**
	\brief Sets the curent pose of the emitter
	*/
	virtual void				 setCurrentPose(const physx::PxMat44& pose) = 0;
	/**
	\brief Sets the curent position of the emitter
	*/
	virtual void				 setCurrentPosition(const physx::PxVec3& pos) = 0;

#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/**
	\brief PhysX SDK 2.8.X.  Attaches the emitter to an actor
	NxActor pointer can be NULL to detach existing actor
	*/
	virtual void				 setAttachActor(NxActor*) = 0;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/**
	\brief PhysX SDK 3.X.  Attaches the emitter to an actor
	PxActor pointer can be NULL to detach existing actor
	*/
	virtual void				 setAttachActor(PxActor*) = 0;
#endif
	/**
	\brief sets the relative pose of the emitter in the space of the actor to which it is attached
	*/
	virtual void				 setAttachRelativePose(const physx::PxMat44& pose) = 0;

#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/// PhysX SDK 2.8.X.  Retrieves the actor, to which the emitter is attached. NULL is returned for an unattached emitter.
	virtual const NxActor* 		 getAttachActor() const = 0;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/// PhysX SDK 3.X. Retrieves the actor, to which the emitter is attached. NULL is returned for an unattached emitter.
	virtual const PxActor* 		 getAttachActor() const = 0;
#endif
	/// Retrieves the relative pose of the emitter in the space of the actor to which it is attached
	virtual const physx::PxMat44 getAttachRelativePose() const = 0;
	///	Retrieves the particle radius
	virtual physx::PxF32         getObjectRadius() const = 0;

	/* Collision filtering settings for overlap tests */
	/// Sets collision groups used to reject particles that overlap the geometry
	virtual void				setOverlapTestCollisionGroups(physx::PxU32) = 0;
#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/// PhysX SDK 2.8.X only.  Sets collision groups mask. \sa NxGroupsMask
	virtual void				setOverlapTestCollisionGroupsMask(NxGroupsMask*) = 0;
#endif

	/// Gets collision groups used to reject particles that overlap the geometry
	virtual physx::PxU32		getOverlapTestCollisionGroups() const = 0;
#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/// PhysX SDK 2.8.X only.  Gets collision groups mask. \sa NxGroupsMask
	virtual const NxGroupsMask*	getOverlapTestCollisionGroupsMask() const = 0;
#endif

	/*
	\brief start emitting particles
	 * If persistent is true, the emitter will emit every frame until stopEmit() is
	 * called.
	 */
	virtual void                 startEmit(bool persistent = true) = 0;
	///stop emitting particles
	virtual void                 stopEmit() = 0;
	///true if the emitter is emitting particles
	virtual bool                 isEmitting() const = 0;

	///	Gets LOD settings
	virtual const NxEmitterLodParamDesc& getLodParamDesc() const = 0;
	///	Sets LOD settings
	virtual void				 setLodParamDesc(const NxEmitterLodParamDesc&) = 0;


	/* Override authored scalable parameters, if necessary */
	///Sets the range from which the density of particles within the volume is randomly chosen
	virtual void                 setDensityRange(const NxRange<physx::PxF32>&) = 0;
	///Sets the range from which the emission rate is randomly chosen
	virtual void                 setRateRange(const NxRange<physx::PxF32>&) = 0;

	///Sets the range from which the velocity of a particle is randomly chosen
	virtual void                 setVelocityRange(const NxRange<physx::PxVec3>&) = 0;
	///Sets the range from which the lifetime of a particle is randomly chosen
	virtual void                 setLifetimeRange(const NxRange<physx::PxF32>&) = 0;

	///Sets whether or not authored asset particles are emitted
	virtual void				 emitAssetParticles(bool enable) = 0;
	///Gets whether or not authored asset particles are emitted
	virtual bool				 getEmitAssetParticles() const = 0;

	///Emitted particles are injected to specified render volume on initial frame.
	///This will work only if you have one renderVolume for each emitter.
	///Set to NULL to clear the preferred volume.
	virtual void                 setPreferredRenderVolume(NxApexRenderVolume* volume) = 0;

	///Gets the range from which the emission rate is randomly chosen
	virtual void                 getRateRange(NxRange<physx::PxF32>&) const = 0;

	virtual PxU32				 getSimParticlesCount() const = 0;

	///Returns the number of particles still alive
	virtual PxU32				 getActiveParticleCount() const = 0;

	///Sets the origin of the density grid used by this emitter.  Important, this density grid may be shared with lots of other emitters as well, it is based on the underlying IOS
	virtual void				setDensityGridPosition(const physx::PxVec3 &pos) = 0;

	/// Sets the ApexEmitterPosition validation callback interface
	virtual void setApexEmitterValidateCallback(NxApexEmitterValidateCallback *callback) = 0;

	///Sets the uniform overall object scale
	PX_DEPRECATED virtual void				setObjectScale(PxF32 scale) = 0;

	//Retrieves the uniform overall object scale
	PX_DEPRECATED virtual PxF32				getObjectScale(void) const = 0;

	///Sets the uniform overall object scale
	virtual void				setCurrentScale(PxF32 scale) = 0;

	//Retrieves the uniform overall object scale
	virtual PxF32				getCurrentScale(void) const = 0;


};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_EMITTER_ACTOR_H

/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_ACTOR_H
#define NX_APEX_ACTOR_H

/*!
\file
\brief classes NxApexActor, NxApexActorSource
*/

#include "NxApexInterface.h"

// PhysX SDK class declarations
#if NX_SDK_VERSION_MAJOR == 2
class NxActorDesc;
class NxActorDescBase;
class NxShapeDesc;
class NxBodyDesc;
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxShape.h"
#include "PxFiltering.h"
namespace physx { namespace apex
{

/** Corresponds to 20 frames for a time step of 0.02, PhysX 3.3 took PX_SLEEP_INTERVAL away */
#define APEX_DEFAULT_WAKE_UP_COUNTER 0.4f

/** Used to define generic get/set methods for 1-parameter values in NxPhysX3DescTemplate. */
#define NX_APEX_ACTOR_TEMPLATE_PARAM(_type, _name, _valid, _default) \
bool is##_name##Valid(_type x) const { PX_UNUSED(x); return _valid; } \
_type getDefault##_name() const { return _default; } \
virtual _type get##_name() const = 0; \
virtual bool set##_name(_type) = 0

/**
	Template for PhysX3.x actor, body and shape.  Used by the Destruction module when creating PhysX objects.

	See NxApexActorSource::setPhysX3Template and NxApexActorSource::getPhysX3Template.
*/
class NxPhysX3DescTemplate
{
public:
	/*
		For each Name below, there are functions: getName(), setName(), isNameValid(), and getDefaultName().  For example:

		PxDominanceGroup	getDominanceGroup() const;
		bool				setDominanceGroup(PxDominanceGroup);	// Returns true iff the passed-in value is valid.  (Note, will set the internal values even if they are not valid.)
		bool				isDominanceGroupValid() const;
		PxDominanceGroup	getDefaultDominanceGroup() const;
	*/

	/*							 Type						Name					Validity Condition		Default Value */
	// Actor
	/** getX(), setX(), getDefaultX(), and isXValid() for X=DominanceGroup (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxDominanceGroup,	DominanceGroup,			(1),					0);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ActorFlags (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU16,				ActorFlags,				(1),					physx::PxActorFlag::eSEND_SLEEP_NOTIFIES);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=OwnerClient (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxClientID,			OwnerClient,			(1),					0);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ClientBehaviorBits (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU32,				ClientBehaviorBits,		(1),					0);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ContactReportFlags (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU16,				ContactReportFlags,		(1),					0);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=UserData (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(void*,						UserData,				(1),					NULL);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=Name (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(const char*,				Name,					(1),					NULL);

	// Body
	/** getX(), setX(), getDefaultX(), and isXValid() for X=Density (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				Density,				(x >= 0.0f),			1.0f);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=BodyFlags (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU8,				BodyFlags,				(1),					0);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=WakeUpCounter (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				WakeUpCounter,			(x >= 0.0f),			APEX_DEFAULT_WAKE_UP_COUNTER);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=LinearDamping (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				LinearDamping,			(x >= 0.0f),			0.0f);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=AngularDamping (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				AngularDamping,			(x >= 0.0f),			0.05f);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=MaxAngularVelocity (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				MaxAngularVelocity,		(1),					7.0f);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=SleepLinearVelocity (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				SleepLinearVelocity,	(1),					0.0f);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=SolverIterationCount (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU32,				SolverIterationCount,	(x >= 1 && x <= 255),	4);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=VelocityIterationCount (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU32,				VelocityIterationCount,	(x >= 1 && x <= 255),	1);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ContactReportThreshold (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				ContactReportThreshold,	(1),					PX_MAX_F32);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=SleepThreshold (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				SleepThreshold,			(1),					0.005f);

	// Shape
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ShapeFlags (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxU8,				ShapeFlags,				(1),					physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ShapeUserData (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(void*,						ShapeUserData,			(1),					NULL);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ShapeName (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(const char*,				ShapeName,				(1),					NULL);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=SimulationFilterData (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxFilterData,		SimulationFilterData,	(1),					physx::PxFilterData(0, 0, 0, 0));
	/** getX(), setX(), getDefaultX(), and isXValid() for X=QueryFilterData (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxFilterData,		QueryFilterData,		(1),					physx::PxFilterData(0, 0, 0, 0));
	/** getX(), setX(), getDefaultX(), and isXValid() for X=ContactOffset (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				ContactOffset,			(1),					-1.0f);
	/** getX(), setX(), getDefaultX(), and isXValid() for X=RestOffset (see NX_APEX_ACTOR_TEMPLATE_PARAM) */
	NX_APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,				RestOffset,				(1),					PX_MAX_F32);
	// Shape materials get explicitly defined API:
	/** getMaterials function is non-generic, as it returns two parameters (the materials array and the array size in materialCount) */
	virtual physx::PxMaterial**	getMaterials(physx::PxU32& materialCount) const = 0;
	/** setMaterials function is non-generic, as it sets two parameters (the materials array in materialArray and the array size in materialCount) */
	virtual bool				setMaterials(physx::PxMaterial** materialArray, physx::PxU32 materialCount) = 0;	// Must have non-zero sized array of materials to be valid.


	/** Use this method to release this object */
	virtual void release() = 0;

protected:
	virtual ~NxPhysX3DescTemplate() {}
};	// NxPhysX3DescTemplate

}}
#endif

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxApexAsset;

/**
\brief Base class for APEX module objects.
*/
class NxApexActor : public NxApexInterface
{
public:
	/**
	\brief Returns the owning asset
	*/
	virtual NxApexAsset* getOwner() const = 0;

	/**
	\brief Returns the range of possible values for physical Lod overwrite

	\param [out] min		The minimum lod value
	\param [out] max		The maximum lod value
	\param [out] intOnly	Only integers are allowed if this is true, gets rounded to nearest

	\note The max value can change with different graphical Lods
	\see NxApexActor::forcePhysicalLod()
	*/
	virtual void getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const = 0;

	/**
	\brief Get current physical lod.
	*/
	virtual physx::PxF32 getActivePhysicalLod() const = 0;

	/**
	\brief Force an APEX Actor to use a certian physical Lod

	\param [in] lod	Overwrite the Lod system to use this Lod.

	\note Setting the lod value to a negative number will turn off the overwrite and proceed with regular Lod computations
	\see NxApexActor::getPhysicalLodRange()
	*/
	virtual void forcePhysicalLod(physx::PxF32 lod) = 0;

	/**
	\brief Ensure that all module-cached data is cached.
	*/
	virtual void cacheModuleData() const {}

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state) = 0;
	

protected:
	virtual ~NxApexActor() {} // use release() method instead!
};

/**
\brief Base class for APEX classes that spawn PhysX SDK Actors
*/
class NxApexActorSource
{
public:
	/**
	\brief Set the current actor template

	User can specify a descriptor template for actors that this object may create.  APEX may customize these suggested settings.
	Does not include shape array, as we only want base properties.  Specify base shape properties using setShapeBase().

	Already created / existing actors will not be changed if the actor template is changed!  The actor template will only be used for new
	actors created after this is called!

	members that are ignored:
	globalPose
	body
	type

	These fields should be left at their default values as set by the desc constructor.

	Because it is not possible to instance the type NxActorDescBase directly, it is simplest to pass a pointer to a NxActorDesc.

	You can pass NULL to not have a template.
	*/

#if NX_SDK_VERSION_MAJOR == 2
	virtual void setActorTemplate(const NxActorDescBase*) = 0;

	/**
	\brief Retrieve the current actor template

	If the template is NULL this will return false; otherwise it will fill in dest and return true.
	*/
	virtual bool getActorTemplate(NxActorDescBase& dest) const = 0;

	/**
	\brief Sets the current shape template

	User can specify a descriptor template for shapes that this object may create.  APEX may customize these suggested settings.
	Already created / existing shapes will not be changed if the shape template is changed!  The shape template will only be used for new
	shapes created after this is called!

	members that are ignored:
	type
	localPose
	ccdSkeleton

	These fields should be left at their default values as set by the desc constructor.

	Because it is not possible to instance the type NxShapeDesc directly, it is simplest to pass a pointer to a NxSphereShapeDesc.
	*/
	virtual void setShapeTemplate(const NxShapeDesc*) = 0;

	/**
	\brief Retrieve the current shape template

	If the template is NULL this will return false; otherwise it will fill in dest and return true.
	*/
	virtual bool getShapeTemplate(NxShapeDesc& dest) const = 0;

	/**
	\brief Sets the current body template

	User can specify a descriptor template for bodies that this object may create.  APEX may customize these suggested settings.
	Already created / existing bodies will not be changed if the body template is changed!  The body template will only be used for
	new bodies created after this is called!

	members that are ignored:
	massLocalPose
	massSpaceInertia
	mass
	linearVelocity
	angularVelocity

	These fields should be left at their default values as set by the desc constructor.

	Because it is not possible to instance the type NxShapeDesc directly, it is simplest to pass a pointer to a NxSphereShapeDesc.
	*/
	virtual void setBodyTemplate(const NxBodyDesc*) = 0;

	/**
	\brief Retrieve the current body template

	If the template is NULL this will return false; otherwise it will fill in dest and return true.
	*/
	virtual bool getBodyTemplate(NxBodyDesc& dest) const = 0;
#elif NX_SDK_VERSION_MAJOR == 3
	/**
	\brief Sets the current body template

	User can specify a descriptor template for bodies that this object may create.  APEX may customize these suggested settings.
	Already created / existing bodies will not be changed if the body template is changed!  The body template will only be used for
	new bodies created after this is called!

	members that are ignored:
	massLocalPose
	massSpaceInertia
	mass
	linearVelocity
	angularVelocity

	These fields should be left at their default values as set by the desc constructor.

	Because it is not possible to instance the type NxShapeDesc directly, it is simplest to pass a pointer to a NxSphereShapeDesc.
	*/
	virtual void setPhysX3Template(const NxPhysX3DescTemplate*) = 0;

	/**
	\brief Retrieve the current body template
	*/
	virtual bool getPhysX3Template(NxPhysX3DescTemplate& dest) const = 0;

	/**
	\brief Create an NxPhysX3DescTemplate object to pass into the get/set methods.
	*/
	virtual NxPhysX3DescTemplate* createPhysX3DescTemplate() const = 0;
#endif
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_ACTOR_H

/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_GROUND_EMITTER_ASSET_H
#define NX_GROUND_EMITTER_ASSET_H

#include "NxApex.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

#define NX_GROUND_EMITTER_AUTHORING_TYPE_NAME "GroundEmitterAsset"

///Ground emitter asset. Used to create Ground emitter actors with specific properties.
class NxGroundEmitterAsset : public NxApexAsset
{
protected:
	PX_INLINE NxGroundEmitterAsset() {}
	virtual ~NxGroundEmitterAsset() {}

public:
	///Gets the range from which the density of particles within the volume is randomly chosen
	virtual const NxRange<physx::PxF32> & getDensityRange() const = 0;  /* Scalable parameter */
	///Gets the velocity range.	The ground emitter actor will create objects with a random velocity within the velocity range.
	virtual const NxRange<physx::PxVec3> & getVelocityRange() const = 0; /* noise parameters */
	///Gets the lifetime range. The ground emitter actor will create objects with a random lifetime (in seconds) within the lifetime range.
	virtual const NxRange<physx::PxF32> & getLifetimeRange() const = 0;

	///Gets the radius.  The ground emitter actor will create objects within a circle of size 'radius'.
	virtual physx::PxF32                  getRadius() const = 0;
	///Gets The maximum raycasts number per frame.
	virtual physx::PxU32					getMaxRaycastsPerFrame() const = 0;
	///Gets the height from which the ground emitter will cast rays at terrain/objects opposite of the 'upDirection'.
	virtual physx::PxF32					getRaycastHeight() const = 0;
	/**
	\brief Gets the height above the ground to emit particles.
	 If greater than 0, the ground emitter will refresh a disc above the player's position rather than
	 refreshing a circle around the player's position.
	*/
	virtual physx::PxF32					getSpawnHeight() const = 0;
	/// Gets collision groups name used to cast rays
	virtual const char* 			getRaycastCollisionGroupMaskName() const = 0;
};

///Ground emitter authoring class. Used to create Ground emitter assets.
class NxGroundEmitterAssetAuthoring : public NxApexAssetAuthoring
{
protected:
	virtual ~NxGroundEmitterAssetAuthoring() {}
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_GROUND_EMITTER_ASSET_H

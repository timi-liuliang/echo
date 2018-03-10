/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_CLOTHING_VELOCITY_CALLBACK_H
#define NX_CLOTHING_VELOCITY_CALLBACK_H

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

/**
\brief container class for the velocity shader callback.
*/
class NxClothingVelocityCallback
{
public:
	/**
	\brief This callback will be fired in Apex threads. It must not address any user data, just operate on the data.
	\param [in,out] velocities  The velocities of the cloth. These can be modified if necessary, but then the method needs to return true!
	\param [in] positions       The positions of the cloth. Must not be modified, only read.
	\param [in] numParticles    Size of the velocities and positions array.

	\return return true if the velocities have been altered, false if they just have been read
	*/
	virtual bool velocityShader(physx::PxVec3* velocities, const physx::PxVec3* positions, physx::PxU32 numParticles) = 0;
};

} // namespace apex
} // namespace physx

#endif // NX_CLOTHING_VELOCITY_CALLBACK_H

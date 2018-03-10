/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "EmitterGeom.h"

namespace physx
{
namespace apex
{
namespace emitter
{

/* Return percentage of new volume not covered by old volume */
physx::PxF32 EmitterGeom::computeNewlyCoveredVolume(
    const physx::PxMat34Legacy& oldPose,
    const physx::PxMat34Legacy& newPose,
    QDSRand& rand) const
{
	// estimate by sampling
	physx::PxU32 numSamples = 100;
	physx::PxU32 numOutsideOldVolume = 0;
	for (physx::PxU32 i = 0; i < numSamples; i++)
	{
		if (!isInEmitter(randomPosInFullVolume(newPose, rand), oldPose))
		{
			numOutsideOldVolume++;
		}
	}

	return (physx::PxF32) numOutsideOldVolume / numSamples;
}


// TODO make better, this is very slow when emitter moves slowly
// SJB: I'd go one further, this seems mildly retarted
physx::PxVec3 EmitterGeom::randomPosInNewlyCoveredVolume(const physx::PxMat34Legacy& pose, const physx::PxMat34Legacy& oldPose, QDSRand& rand) const
{
	physx::PxVec3 pos;
	do
	{
		pos = randomPosInFullVolume(pose, rand);
	}
	while (isInEmitter(pos, oldPose));
	return pos;
}

}
}
} // namespace physx::apex

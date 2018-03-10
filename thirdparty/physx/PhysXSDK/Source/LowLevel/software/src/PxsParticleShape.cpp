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


#include "PxsParticleShape.h"
#include "PxsContext.h"
#include "PxsParticleSystemSim.h"
#include "PxsFluidSpatialHash.h"

using namespace physx;

PxsParticleShape::PxsParticleShape(PxsContext* /*context*/, PxU32 index) :
	mIndex(index),
	mParticleSystem(NULL),
	mPacket(NULL),
	mUserData(NULL)
{
}

PxsParticleShape::~PxsParticleShape()
{
}

void PxsParticleShape::init(PxsParticleSystemSim* particleSystem, const PxsParticleCell* packet)
{
	PX_ASSERT(mParticleSystem == NULL);
	PX_ASSERT(mPacket == NULL);
	PX_ASSERT(mUserData == NULL);

	PX_ASSERT(particleSystem);
	PX_ASSERT(packet);

	mParticleSystem = particleSystem;
	mPacket = packet;
	mPacketCoordinates = packet->coords;	// this is needed for the remapping process.

	// Compute and store AABB of the assigned packet
	mParticleSystem->getPacketBounds(mPacketCoordinates, mBounds);
}

void PxsParticleShape::destroyV()
{
	PX_ASSERT(mParticleSystem);
	mParticleSystem->getContext().releaseFluidShape(this);

	mParticleSystem = NULL;
	mPacket = NULL;
	mUserData = NULL;

}


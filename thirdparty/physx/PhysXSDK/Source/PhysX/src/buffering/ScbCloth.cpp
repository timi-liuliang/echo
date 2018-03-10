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

#include "PxPhysXConfig.h"

#if PX_USE_CLOTH_API

#include "ScbCloth.h"

using namespace physx;

Scb::Cloth::Cloth(const PxTransform& globalPose, Sc::ClothFabricCore& fabric, const PxClothParticle* particles, PxClothFlags flags) : 
	mCloth(globalPose, fabric, particles, flags)
{
	setScbType(ScbType::CLOTH);
}


Scb::Cloth::~Cloth()
{
}


void Scb::Cloth::syncState()
{
	if (getBufferFlags())  // Optimization to avoid all the if-statements below if possible
	{
		Actor::syncState();
	}

	postSyncState();
}

#endif // PX_USE_CLOTH_API

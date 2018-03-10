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


#include "ScStaticCore.h"
#include "ScStaticSim.h"
#include "PxRigidStatic.h"

using namespace physx;

Sc::StaticSim* Sc::StaticCore::getSim() const
{
	return static_cast<StaticSim*>(Sc::ActorCore::getSim());
}

void Sc::StaticCore::setActor2World(const PxTransform& actor2World)
{
	mCore.body2World = actor2World;

	StaticSim* sim = getSim();
	if(sim)
		sim->postActor2WorldChange();
}

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


#include "ScStaticSim.h"
#include "ScScene.h"

using namespace physx;

Sc::StaticSim::StaticSim(Scene& scene, StaticCore& core) :
	RigidSim(scene, core, IslandNodeInfo::eNON_PARTICIPANT)
{
	scene.getInteractionScene().addActor(*this, false);
}

Sc::StaticSim::~StaticSim()
{
	getInteractionScene().removeActor(*this);

	getStaticCore().setSim(NULL);
}

void Sc::StaticSim::postActor2WorldChange()
{
	notifyShapeTranformChange();
}

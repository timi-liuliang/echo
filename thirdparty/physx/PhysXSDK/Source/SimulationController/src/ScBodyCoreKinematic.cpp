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

#include "ScBodyCore.h"
#include "ScSimStateData.h"

using namespace physx;

bool Sc::BodyCore::getKinematicTarget(PxTransform& p) const
{
	PX_ASSERT(mCore.mFlags & PxRigidBodyFlag::eKINEMATIC);

	if (mSimStateData && mSimStateData->isKine() && mSimStateData->getKinematicData()->targetValid)
	{
		p = mSimStateData->getKinematicData()->targetPose;
		return true;
	}
	else
		return false;
}

bool Sc::BodyCore::getHasValidKinematicTarget() const
{
 //The use pattern for this is that we should only look for kinematic data if we know it is kinematic.
 //We might look for velmod data even if it is kinematic.
 PX_ASSERT(!mSimStateData || mSimStateData->isKine()); 
 return 
 (
  mSimStateData && 
  mSimStateData->isKine() &&
  mSimStateData->getKinematicData()->targetValid
 );
}

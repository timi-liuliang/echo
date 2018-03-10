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

#include "CctSweptBox.h"
#include "CctCharacterController.h"
#include "CctUtils.h"

using namespace physx;
using namespace Cct;

SweptBox::SweptBox()
{
	mType = SweptVolumeType::eBOX;
}

SweptBox::~SweptBox()
{
}

void SweptBox::computeTemporalBox(const SweepTest& test, PxExtendedBounds3& box, const PxExtendedVec3& center, const PxVec3& direction) const
{
	const float radius = PxMax(mExtents.y, mExtents.z);
	const float height = 2.0f * mExtents.x;
	Cct::computeTemporalBox(box, radius, height, test.mUserParams.mContactOffset, test.mUserParams.mMaxJumpHeight, test.mUserParams.mUpDirection, center, direction);
}

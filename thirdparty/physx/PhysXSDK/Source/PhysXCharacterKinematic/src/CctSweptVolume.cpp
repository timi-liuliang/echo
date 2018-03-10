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

#include "CctSweptVolume.h"

using namespace physx;
using namespace Cct;

SweptVolume::SweptVolume()
{
	mType = SweptVolumeType::eLAST;
}

SweptVolume::~SweptVolume()
{
}

void Cct::computeTemporalBox(PxExtendedBounds3& _box, float radius, float height, float contactOffset, float maxJumpHeight, const PxVec3& upDirection, const PxExtendedVec3& center, const PxVec3& direction)
{
	const float r = radius + contactOffset;
	PxVec3 extents(r);
	const float halfHeight = height*0.5f;
	extents.x += fabsf(upDirection.x)*halfHeight;
	extents.y += fabsf(upDirection.y)*halfHeight;
	extents.z += fabsf(upDirection.z)*halfHeight;

	PxExtendedBounds3 box;
	setCenterExtents(box, center, extents);

	{
		PxExtendedBounds3 destBox;
		PxExtendedVec3 tmp = center;
		tmp += direction;
		setCenterExtents(destBox, tmp, extents);
		add(box, destBox);
	}

	if(maxJumpHeight!=0.0f)
	{
		PxExtendedBounds3 destBox;
		PxExtendedVec3 tmp = center;
		tmp -= upDirection * maxJumpHeight;
		setCenterExtents(destBox, tmp, extents);
		add(box, destBox);
	}

	_box = box;
}

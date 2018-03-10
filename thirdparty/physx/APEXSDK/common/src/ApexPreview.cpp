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
#include "ApexPreview.h"

namespace physx
{
namespace apex
{

ApexPreview::ApexPreview() :
	mInRelease(false)
{
	mPose.id();
}

ApexPreview::~ApexPreview()
{
	destroy();
}

void ApexPreview::setPose(const physx::PxMat44& pose)
{
	mPose = pose;
}

const physx::PxMat44 ApexPreview::getPose() const
{
	return mPose;
}

void ApexPreview::destroy()
{
	mInRelease = true;

	renderDataLock();
}

}
} // end namespace physx::apex

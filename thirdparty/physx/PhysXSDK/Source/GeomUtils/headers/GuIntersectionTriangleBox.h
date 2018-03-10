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

#ifndef GU_INTERSECTION_TRIANGLE_BOX_H
#define GU_INTERSECTION_TRIANGLE_BOX_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "GuBox.h"

namespace physx
{
namespace Gu
{
	class Box;
	struct OBBTriangleTest
	{
		PxMat33								mRModelToBox;		//!< Rotation from model space to obb space
		PxMat33								mRBoxToModel;		//!< Rotation from obb space to model space
		PxVec3								mTModelToBox;		//!< Translation from model space to obb space
		PxVec3								mTBoxToModel;		//!< Translation from obb space to model space
		PxVec3								mBoxExtents;

		OBBTriangleTest(const Gu::Box& box);
		Ps::IntBool obbTriTest(const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2) const;

	};
	PX_PHYSX_COMMON_API Ps::IntBool intersectTriangleBox(const PxVec3& boxCenter, const PxVec3& extents, const PxVec3& tp0, const PxVec3& tp1, const PxVec3& tp2);

} // namespace Gu
}

#endif

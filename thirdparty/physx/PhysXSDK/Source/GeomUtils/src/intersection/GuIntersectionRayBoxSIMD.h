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

#ifndef GU_INTERSECTION_RAY_BOX_SIMD_H
#define GU_INTERSECTION_RAY_BOX_SIMD_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "PxIntrinsics.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{

	int PX_PHYSX_COMMON_API intersectRayAABB(	const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum,
												const Ps::aos::Vec3VArg rayOrigin, const Ps::aos::Vec3VArg rayDirection,
												Ps::aos::FloatV& tnear, Ps::aos::FloatV& tfar);

	int PX_PHYSX_COMMON_API intersectRayAABB(	const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum,
												const Ps::aos::Vec3VArg rayOrigin, const Ps::aos::Vec3VArg rayDirection, const Ps::aos::Vec3VArg invDirection,
												Ps::aos::FloatV& tnear, Ps::aos::FloatV& tfar);


	bool PX_PHYSX_COMMON_API intersectRayAABB2(	const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum, 
												const Ps::aos::Vec3VArg ro, const Ps::aos::Vec3VArg rd, const Ps::aos::FloatVArg maxDist, 
												Ps::aos::FloatV& tnear, Ps::aos::FloatV& tfar);

} // namespace Gu

}

#endif

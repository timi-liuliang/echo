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

#ifndef GU_DISTANCE_SEGMENT_TRIANGLE_SIMD_H
#define GU_DISTANCE_SEGMENT_TRIANGLE_SIMD_H

#include "PxPhysXCommonConfig.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{

	/*
		closest0 is the closest point on segment pq
		closest1 is the closest point on triangle abc
	*/
	PX_PHYSX_COMMON_API Ps::aos::FloatV distanceSegmentTriangleSquared(
		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q,
		const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg c,
		Ps::aos::Vec3V& closest0, Ps::aos::Vec3V& closest1);

} // namespace Gu

}

#endif

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

#ifndef GU_GJK_FALLBACK_H
#define GU_GJK_FALLBACK_H

//#include "CmPhysXCommon.h"
//#include "PsAllocator.h"
#include "GuConvexSupportTable.h"
#include "GuGJKUtil.h"


namespace physx
{

namespace Gu
{
	PxGJKStatus gjkRelativeFallbackWithStack(const ConvexV& a, const ConvexV& b, SupportMap* map1, SupportMap* map2,  const Ps::aos::Vec3VArg initialDir, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist);

	PxGJKStatus gjkRelativeFallback(const ConvexV& a, const ConvexV& b, SupportMap* map1, SupportMap* map2,  const Ps::aos::Vec3VArg initialDir, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist);
}
}

#endif

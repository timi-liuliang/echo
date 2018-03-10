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

#ifndef GU_RAYCAST_TESTS_H
#define GU_RAYCAST_TESTS_H

#include "CmPhysXCommon.h"
#include "PxPhysXCommonConfig.h"
#include "PxSimpleTypes.h"
#include "PxQueryReport.h"

namespace physx
{

class PxGeometry;

#define GU_RAY_FUNC_PARAMS	const PxGeometry& geom, const PxTransform& pose,\
							const PxVec3& rayOrigin, const PxVec3& rayDir, PxReal maxDist,\
							PxHitFlags hintFlags, PxU32 maxHits, PxRaycastHit* PX_RESTRICT hits, bool anyHit,\
							RaycastHitFunc hitCB, void* userData

namespace Gu
{
	typedef bool	(*RaycastHitFunc)	(PxRaycastHit&, void*);
	typedef PxU32	(*RaycastFunc)		(GU_RAY_FUNC_PARAMS);

	typedef RaycastFunc GeomRaycastTableEntry7[7];
	PX_PHYSX_COMMON_API GeomRaycastTableEntry7& GetRaycastFuncTable();
}  // namespace Gu

}

#endif
